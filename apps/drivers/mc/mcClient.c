#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>

static struct nl_sock *sk = NULL;

/**
* Attributes and commands have to be the same as in kernelspace, so you might
* want to move these enums to a .h and just #include that from both files.
*/
enum attributes 
{
	ATTR_DUMMY,
	ATTR_HELLO,
	ATTR_FOO,

	/* This must be last! */
	__ATTR_MAX,
};

enum commands {
	COMMAND_HELLO,

	/* This must be last! */
	__COMMAND_MAX,
};

static int fail(int error, char *func_name)
{
	printf("%s() failed.\n", func_name);
	return error;
}

static int nl_fail(int error, char *func_name)
{
	printf("%s (%d)\n", nl_geterror(error), error);
	return fail(error, func_name);
}

/*
* This function will be called for each valid netlink message received
* in nl_recvmsgs_default()
*/
static int cb(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nl_hdr;
	struct genlmsghdr *genl_hdr;
	struct nlattr *attrs[__ATTR_MAX];
	int error;

	printf("The kernel module sent a message.\n");

	nl_hdr = nlmsg_hdr(msg);
	genl_hdr = genlmsg_hdr(nl_hdr);

	if (genl_hdr->cmd != COMMAND_HELLO)
	{
		printf("Oops? The message type is not Hello; ignoring.\n");
		return 0;
	}

	error = genlmsg_parse(nl_hdr, 0, attrs, __ATTR_MAX - 1, NULL);
	if (error)
	return nl_fail(error, "genlmsg_parse");

	/* Remember: attrs[0] is a throwaway. */

	if (attrs[1])
		printf("ATTR_HELLO: len:%u type:%u data:%s\n",
	        attrs[1]->nla_len,
	        attrs[1]->nla_type,
	        (char *)nla_data(attrs[1]));
	else
		printf("ATTR_HELLO: null\n");

	if (attrs[2])
		printf("ATTR_FOO: len:%u type:%u data:%u\n",
	        attrs[2]->nla_len,
	        attrs[2]->nla_type,
	        *((__u32 *)nla_data(attrs[2])));
	else
		printf("ATTR_FOO: null\n");

	return 0;
}

static int do_things(void)
{
	struct genl_family *family;
	int group;
	int error;

	/* Socket allocation yadda yadda. */
	sk = nl_socket_alloc();
	if (!sk)
		return fail(-1, "nl_socket_alloc");

	nl_socket_disable_seq_check(sk);

	error = nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, cb, NULL);
	if (error)
		return nl_fail(error, "nl_socket_modify_cb");

	error = genl_connect(sk);
	if (error)
		return nl_fail(error, "genl_connect");

	/* Find the multicast group identifier and register ourselves to it. */
	group = genl_ctrl_resolve_grp(sk, "PotatoFamily", "PotatoGroup");
	if (group < 0)
		return nl_fail(group, "genl_ctrl_resolve_grp");

	printf("The group is %u.\n", group);

	error = nl_socket_add_memberships(sk, group, 0);
	if (error)
	{
		printf("nl_socket_add_memberships() failed: %d\n", error);
		return error;
	}

	/* Finally, receive the message. */
	nl_recvmsgs_default(sk);

	return 0;
}

int main(void)
{
	int error;

	error = do_things();

	if (sk)
		nl_socket_free(sk);

	return error;
}

