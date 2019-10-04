
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <asm/byteorder.h>
#include <asm/types.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "sysNetlinkW1.h"
#include "libCmnSys.h"

#define	__DEBUG_W1		EXT_DBG_OFF

#define	_W1_DEFAULT_TIMEOUT			5 /* seconds */

#define	W1_MASTER_DESTROY(w1Master) \
		if(w1Master->nlm){ \
			free( w1Master->nlm ) ; w1Master->nlm = NULL ; }

#define	_HEX_DUMP(prompt, data, size ) \
		{int i ; printf((prompt)); for ( i=0 ; i< (size); ++i ){ \
			printf(" %.2X", (__u8) *(data+i)) ; } \
			printf("\n"); }


char *cmnSysW1MessageName(int type)
{
	switch (type)
	{
		case W1_SLAVE_ADD:
			return "SlaveAdd";
			break;
		case W1_SLAVE_REMOVE:
			return "SlaveRemove";
			break;
		case W1_MASTER_ADD:
			return "MasterAdd";
			break;
		case W1_MASTER_REMOVE:
			return "MasterRemove";
			break;
		case W1_MASTER_CMD:
			return "MasterCmd";
			break;
		case W1_SLAVE_CMD:
			return "SlaveCmd";
			break;
		case W1_LIST_MASTERS:
			return "ListMaster";
			break;
		default:
			return "W1-Unknown";
			break;
	}	

	return "W1-Unknown";

}


char *cmnSysW1CommandName(int type)
{
	switch (type)
	{
		case W1_CMD_READ:
			return "READ";
			break;
		case W1_CMD_WRITE:
			return "WRITE";
			break;
		case W1_CMD_SEARCH:
			return "SEARCH";
			break;
		case W1_CMD_ALARM_SEARCH:
			return "ALARM_SEARCH";
			break;
		case W1_CMD_TOUCH:
			return "TOUCH";
			break;
		case W1_CMD_RESET:
			return "RESET";
			break;
		case W1_CMD_SLAVE_ADD:
			return "SlaveAdd";
			break;
		case W1_CMD_SLAVE_REMOVE:
			return "SlaveRemove";
			break;
		case W1_CMD_LIST_SLAVES:
			return "ListSlave";
			break;
		default:
			return "W1-CMD-Unknown";
			break;
	}	

	return "W1-CMD-Unknown";
}

#if __DEBUG_W1
static void _sysW1DebugMsg(struct netlink_parse *w1Master )
{
	if ( w1Master->nlm )
	{
		printf("NLMSGHDR: len=%u type=%u flags=%u seq=%u pid=%u\n",
			w1Master->nlm->nlmsg_len, w1Master->nlm->nlmsg_type, w1Master->nlm->nlmsg_flags, w1Master->nlm->nlmsg_seq, w1Master->nlm->nlmsg_pid ) ;
	}

	if ( w1Master->cn )
	{
		printf("CN_MSG: idx/val=%u/%u seq=%u ack=%u len=%u flags=%u\n",
			w1Master->cn->id.idx, w1Master->cn->id.val, w1Master->cn->seq, w1Master->cn->ack, w1Master->cn->len, w1Master->cn->flags) ;
	}

	if ( w1Master->w1m )
	{
		printf("W1 MSG: type:%s(%u); len=%u; id=%u\n", 
			cmnSysW1MessageName(w1Master->w1m->type), w1Master->w1m->type, w1Master->w1m->len, w1Master->w1m->id.mst.id ) ;
	}

	if( w1Master->w1c )
	{
		printf("W1 CMD: cmd:%s(%u); len=%u\n", 
			cmnSysW1CommandName(w1Master->w1c->cmd), w1Master->w1c->cmd,  w1Master->w1c->len) ;
	}

	if(w1Master->data_size )
	{
	
		int i ;
		printf("DATA:");
		for ( i=0 ; i< w1Master->data_size ; ++i )
		{
			printf(" %.2X", w1Master->data[i]) ;
		}
		printf("\n");
		_HEX_DUMP("DATA:", w1Master->data, w1Master->data_size);
	}
}
#endif

int __sysW1Receive(struct netlink_parse *w1Master)
{
	unsigned char * buffer ;
	struct nlmsghdr peek_nlm ;

	// first peek at message to get length and details
	int recv_len = recv(w1Master->socket, &peek_nlm, sizeof(peek_nlm), MSG_PEEK );
	EXT_DEBUGF(__DEBUG_W1, "Pre-parse header: %d bytes len=%d type=%d seq=%d pid=%d (our pid=%d)\n",recv_len,peek_nlm.nlmsg_len,peek_nlm.nlmsg_type,peek_nlm.nlmsg_seq,peek_nlm.nlmsg_pid,getpid());
	if (recv_len == -1)
	{
		MUX_ERROR("recv header error");
		return -errno ;
	}
	
	// test pid
	if ( peek_nlm.nlmsg_pid != 0 )
	{
		MUX_ERROR("Not our pid on message");
		// non-peek
		recv(w1Master->socket, &peek_nlm, sizeof(peek_nlm), 0 );
		return -EAGAIN ;
	}
	
	// test type
	if ( peek_nlm.nlmsg_type != NLMSG_DONE )
	{
		MUX_ERROR("Bad message type");
		// non-peek
		recv(w1Master->socket, &peek_nlm, sizeof(peek_nlm), 0 );
		return -EINVAL ;
	}
	
	// test length
	if ( peek_nlm.nlmsg_len < sizeof(struct nlmsghdr) + sizeof(struct cn_msg) + sizeof(struct w1_netlink_msg) )
	{
		MUX_ERROR("Bad message length (%d)",peek_nlm.nlmsg_len);
		// non-peek
		recv(w1Master->socket, &peek_nlm, sizeof(peek_nlm), 0 );
		return -EMSGSIZE ;
	}
	
	// allocate space
	buffer = malloc( peek_nlm.nlmsg_len ) ;
	if ( buffer == NULL )
	{
		MUX_ERROR("Cannot allocate %d byte buffer for netlink data", peek_nlm.nlmsg_len) ;
		return -ENOMEM ;
	}

	// read whole packet
	recv_len = recv(w1Master->socket, buffer, peek_nlm.nlmsg_len, 0 );
	if (recv_len == -1)
	{
		MUX_ERROR("recv body error: %m");
		free(buffer);
		return -EIO ;
	}

	w1Master->nlm = (struct nlmsghdr *)&buffer[0] ;
	w1Master->cn  = (struct cn_msg *)&buffer[sizeof(struct nlmsghdr)] ;
	w1Master->w1m = (struct w1_netlink_msg *) &buffer[sizeof(struct nlmsghdr) + sizeof(struct cn_msg)] ;
	w1Master->w1c = (struct w1_netlink_cmd *) &buffer[sizeof(struct nlmsghdr) + sizeof(struct cn_msg) + sizeof(struct w1_netlink_msg)] ;

	switch (w1Master->w1m->type)
	{
		case W1_SLAVE_ADD:
		case W1_SLAVE_REMOVE:
		case W1_MASTER_ADD:
		case W1_MASTER_REMOVE:
		case W1_LIST_MASTERS:
			w1Master->w1c = NULL ;
			w1Master->data = w1Master->w1m->data ;
			w1Master->data_size = w1Master->w1m->len ;
			break ;
		case W1_MASTER_CMD:
		case W1_SLAVE_CMD:
			w1Master->data = w1Master->w1c->data ;
			w1Master->data_size = w1Master->w1c->len ;
	}
	
	if ( w1Master->data_size == 0 )
	{
		w1Master->data = NULL ;
	}

#if __DEBUG_W1
	_sysW1DebugMsg( w1Master) ;
#endif

	return 0 ;
}



static int _sysW1SendCmd(struct netlink_parse *w1Master, struct w1_netlink_msg *msg)
{
	struct nlmsghdr		*nlh;
	struct cn_msg		*cmsg;
	struct w1_netlink_msg	*m;
	
	int size, err;
	
	size = sizeof(struct nlmsghdr) + sizeof(struct cn_msg) + sizeof(struct w1_netlink_msg) + msg->len;
	nlh = malloc(size);
	if (!nlh)
	{
		return -ENOMEM;
	}
	
	memset(nlh, 0, size);
	
	nlh->nlmsg_seq = w1Master->seq++;
	nlh->nlmsg_type = NLMSG_DONE;
	nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(struct w1_netlink_msg) + msg->len);
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_pid = getpid() ;

	/* message to connector, cn_msg */	
	cmsg = NLMSG_DATA(nlh);
	cmsg->id.idx = CN_W1_IDX;
	cmsg->id.val = CN_W1_VAL;
	cmsg->seq = w1Master->seq;
	cmsg->ack = 1;
	cmsg->flags = 0 ;
	cmsg->len = sizeof(struct w1_netlink_msg) + msg->len;
	
	m = (struct w1_netlink_msg *)(cmsg + 1);
	memcpy(m, msg, sizeof(struct w1_netlink_msg));
	memcpy(m+1, msg->data, msg->len);

	err = send(w1Master->socket, nlh, size,  0);
	free(nlh);

	if (err == -1)
	{
		MUX_ERROR("Failed to send W1_LIST_MASTERS: %m");
		return -1;
	}
	
	return 0;
}


static int _sysW1Poll(struct netlink_parse *w1Master)
{
//	int repeat_loop = 1 ;
	int ret = 1;
	
	int select_value ;
	struct timeval tv = { _W1_DEFAULT_TIMEOUT, 0 } ;
		
	fd_set readset ;
	FD_ZERO(&readset) ;
	FD_SET(w1Master->socket, &readset) ;
		
	select_value = select(w1Master->socket+1, &readset, NULL, NULL, &tv) ;
	if ( select_value == -1 )
	{
		if (errno != EINTR)
		{
			MUX_ERROR("Select returned -1");
			return -ret;
		}
	}
	else if ( select_value == 0 )
	{
		MUX_ERROR("Select returned zero (timeout)");
		return -ret;
	}
	else
	{
		if( __sysW1Receive(w1Master ) )
		{
			MUX_ERROR("Receiving and parsing netlink packet failed");
		}
		else
		{
			if( w1Master->nlm->nlmsg_seq != w1Master->seq )
			{
				MUX_ERROR("Non-matching netlink packet");
			}
			else
			{
				ret = 0;
			}
			
			//w1_masters( &nlp );
//			repeat_loop = w1Master->cn->ack ;
		}

	}

	if(ret )
	{
		W1_MASTER_DESTROY(w1Master);
	}

	return ret;
}


int cmnSysW1Init( struct netlink_parse *w1Master)
{
	struct sockaddr_nl l_local;

	w1Master->socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if (w1Master->socket == -1)
	{
		MUX_ERROR("socket of W1: %m");
		return -1;
	}
	
	l_local.nl_family = AF_NETLINK;
	l_local.nl_groups = 23;
	l_local.nl_pid = getpid();
	
	if (bind(w1Master->socket, (struct sockaddr *)&l_local, sizeof(struct sockaddr_nl)) == -1)
	{
		MUX_ERROR("bind W1: %m");
		close(w1Master->socket);
		w1Master->socket = -1 ;
		return -1;
	}

#if 0
	{
		int on = 0x57; /* Additional group number */		
		setsockopt(w1Master->socket, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &on, sizeof(on));	
	}
#endif

	return 0 ;
}

#if 0
static void _w1_masters(struct netlink_parse * nlp)
{
	if (nlp->w1m)
	{
		int bus_master = nlp->w1m->id.mst.id ;
		switch (nlp->w1m->type)
		{
			case W1_MASTER_ADD:
				//AddBus(bus_master) ;
				printf("Master has been added: w1_master%d.\n",	bus_master);
					break;
			case W1_MASTER_REMOVE:
				//RemoveBus(bus_master) ;
				printf("Master has been removed: w1_master%d.\n", bus_master);
				break;
			default:
				printf("Other command (Not master)\n");
				break ;
		}
	}
}
#endif

int cmnSysW1FindMaster(struct netlink_parse *w1Master)
{
	struct w1_netlink_msg w1lm;
	EXT_DEBUGF(__DEBUG_W1, "Send list master command...");
	
	memset(&w1lm, 0, sizeof(w1lm));
	w1lm.type = W1_LIST_MASTERS;
	w1lm.len = 0;
	w1lm.id.mst.id = 0;
	
	if(_sysW1SendCmd(w1Master, &w1lm) < 0 )
	{
		MUX_ERROR("Couldn't send the W1_LIST_MASTERS request");
		return -1;
	}

	if(_sysW1Poll(w1Master) )
	{
		MUX_ERROR("Couldn't recevie or parse the W1_LIST_MASTERS response");
		return -1;
	}

	int *bus_master = (int *) w1Master->data ;
	int i ;

	if(w1Master->w1m->type == W1_LIST_MASTERS)
	{
		for ( i=0; i < w1Master->data_size/4 ; ++i )
		{
			EXT_DEBUGF(__DEBUG_W1,"Add #%d w1_bus_master#%d", i, bus_master[i]) ;
		}
		w1Master->masterId = bus_master[0];

		EXT_DEBUGF(__DEBUG_W1,"Master ID:%d", w1Master->masterId );
	}
	else
	{
		MUX_ERROR("W1 MSG type is not LIST MASTER" );
	}

	W1_MASTER_DESTROY(w1Master);
	
	return 0;
}

int cmnSysW1FindSlave(struct netlink_parse *w1Master)
{
	struct w1_netlink_msg	*w1Msg;
	struct w1_netlink_cmd	*w1Cmd;

	EXT_DEBUGF(__DEBUG_W1,"Send list slave command to master...");

	int size = sizeof(struct w1_netlink_msg) +sizeof(struct w1_netlink_cmd);
	w1Msg = malloc(size);
	if (!w1Msg)
	{
		return -ENOMEM;
	}
	
	memset(w1Msg, 0, size );
	w1Msg->type = W1_MASTER_CMD;
	w1Msg->len = sizeof(struct w1_netlink_cmd);
	w1Msg->id.mst.id = w1Master->masterId;

	w1Cmd = (struct w1_netlink_cmd *)(w1Msg+1);
	w1Cmd->cmd = W1_CMD_LIST_SLAVES;
	w1Cmd->len = 0;
	
	if(_sysW1SendCmd(w1Master, w1Msg) < 0 )
	{
		MUX_ERROR("Couldn't send the LIST_SLAVE request");
		return -1;
	}

	if(_sysW1Poll(w1Master) )
	{
		MUX_ERROR("Couldn't recevie or parse the W1_LIST_MASTERS response");
		return -1;
	}

#if 1

	if(w1Master->w1m->type == W1_MASTER_CMD && w1Master->w1c->cmd == W1_CMD_LIST_SLAVES)
	{
		/* check the master ID is same as local*/

		
		if( w1Master->w1c->len != W1_SLACE_ID_LENGTH)
		{
			MUX_ERROR("Slave ID length is wrong: %d", w1Master->w1c->len) ;
		}
		else
		{
			memcpy(w1Master->slaveId, w1Master->w1c->data, w1Master->w1c->len);
#if __DEBUG_W1			
			_HEX_DUMP("Slave ID:", w1Master->slaveId, sizeof(w1Master->slaveId) );
#endif
		}
	}
	else
	{
		MUX_ERROR("W1 MSG type is not LIST MASTERn" );
	}
#endif

	W1_MASTER_DESTROY(w1Master);
	
	return 0;
}


int cmnSysW1GetRomId(unsigned char *romId)
{
	struct netlink_parse _w1;
	struct netlink_parse	*w1Master = &_w1;
	
	memset(w1Master, 0, sizeof(struct netlink_parse));

	if ( cmnSysW1Init(w1Master) )
	{
		MUX_ERROR("Cannot access netlink to get w1 data");
		return EXIT_FAILURE;
	}

	if(cmnSysW1FindMaster(w1Master) )
	{
		MUX_ERROR("Cannot get w1 master");
		return EXIT_FAILURE;
	}
	
	if(cmnSysW1FindSlave(w1Master) )
	{
		MUX_ERROR("Cannot get w1 salve from master#%d", w1Master->masterId);
		return EXIT_FAILURE;
	}

	memcpy(romId, w1Master->slaveId, sizeof(w1Master->slaveId));
	
	return EXIT_SUCCESS;
}


int testDs2815(void )
{
	struct netlink_parse _w1;
	struct netlink_parse	*w1Master = &_w1;
	
	memset(w1Master, 0, sizeof(struct netlink_parse));

	if ( cmnSysW1Init(w1Master) )
	{
		MUX_ERROR("Cannot access netlink to get w1 data");
		exit(1) ;
	}

	if(cmnSysW1FindMaster(w1Master) )
	{
		MUX_ERROR("Cannot get w1 master");
		exit(1) ;
	}
	
	if(cmnSysW1FindSlave(w1Master) )
	{
		MUX_ERROR("Cannot get w1 salve from master#%d", w1Master->masterId);
		exit(1) ;
	}

	return 0;
}



