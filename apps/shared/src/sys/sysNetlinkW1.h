
#ifndef __SYS_NETLINK_W1_H__
#define __SYS_NETLINK_W1_H__

#include <asm/types.h>
#include <linux/connector.h>

#ifndef CN_W1_IDX
#define CN_W1_IDX			0x3
#define CN_W1_VAL			1
#endif

struct w1_reg_num
{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u64	family:8,
		id:48,
		crc:8;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u64	crc:8,
		id:48,
		family:8;
#else
#error "Please fix <asm/byteorder.h>"
#endif
};

enum w1_netlink_message_types
{
	W1_SLAVE_ADD = 0,
	W1_SLAVE_REMOVE,
	W1_MASTER_ADD,
	W1_MASTER_REMOVE,
	W1_MASTER_CMD,
	W1_SLAVE_CMD,
	W1_LIST_MASTERS,
};

struct w1_netlink_msg
{
	__u8				type;
	__u8				reserved;
	__u16				len;
	union {
		__u8			id[8];
		struct w1_mst {
			__u32		id;
			__u32		res;
		} mst;
	} id;
	__u8				data[0];
};

#define W1_CMD_READ				0x0
#define W1_CMD_WRITE				0x1
#define W1_CMD_SEARCH				0x2
#define W1_CMD_ALARM_SEARCH		0x3
#define W1_CMD_TOUCH				4
#define W1_CMD_RESET				5
#define W1_CMD_SLAVE_ADD			6
#define W1_CMD_SLAVE_REMOVE		7
#define W1_CMD_LIST_SLAVES			8

struct w1_netlink_cmd
{
	__u8				cmd;
	__u8				res;
	__u16				len;
	__u8				data[0];
};

#define	W1_SLACE_ID_LENGTH			8

struct netlink_parse
{
	struct nlmsghdr			*nlm ;
	struct cn_msg			*cn ;

	struct w1_netlink_msg		*w1m ;
	struct w1_netlink_cmd		*w1c ;	/* cmds are contained in msg */
	
	unsigned char				*data ;
	int						data_size ;

	int						socket;
	
	int						masterId;
	__u8					slaveId[W1_SLACE_ID_LENGTH];

	int						seq;
} ;


#endif


