#ifndef  __LIB_CMN_SYS_H__
#define	__LIB_CMN_SYS_H__


#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"

#include "mux7xx.h"


struct _CmnGroup;

typedef struct _CmnGroup
{
	char			groupAddress[CMN_NAME_LENGTH];
	char			devName[CMN_NAME_LENGTH];

	uint32_t		address;
	int			ifIndex;	/* index of net interface */
	int			socket;

	int		(*changeGroup)(struct _CmnGroup *group, char *newAddress);
}CmnMGroup;


int cmnSysI2cWrite(int deviceID, unsigned char offset, unsigned char *buffer, int length);
int cmnSysI2cRead(int deviceID, unsigned char offset, unsigned char *buffer, int length);

/* system network */
uint32_t cmnSysNetGetIp(char * hwName);
uint32_t cmnSysNetGetMask(char *hwName);

uint32_t cmnSysNetGetDefaultGw(char *hwName);


char *cmnSysNetAddress( uint32_t address);
uint32_t cmnSystemNetIp(char *ip);

int cmnSysNetGetMacAddress(char *hwName, EXT_MAC_ADDRESS *mac);


CmnMGroup *cmnSysNetMGroupInit(const char *devName, char *groupIp);
void cmnSysNetMGroupDestory(CmnMGroup *_group);


cJSON *extSystemFindObject(MuxMain *muxMain, const char*objName );
cJSON *extSystemGetSubItem(cJSON *sysObj, char *item, int index);




#endif

