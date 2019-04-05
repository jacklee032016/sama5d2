#ifndef  __LIB_CMN_SYS_H__
#define	__LIB_CMN_SYS_H__


int cmnSysI2cWrite(int deviceID, unsigned char offset, unsigned char *buffer, int length);
int cmnSysI2cRead(int deviceID, unsigned char offset, unsigned char *buffer, int length);

/* system network */
uint32_t cmnSysNetGetIp(char * hwName);
uint32_t cmnSysNetGetMask(char *hwName);

uint32_t cmnSysNetGetDefaultGw(char *hwName);


char *cmnSysNetAddress( uint32_t address);

int cmnSysNetGetMacAddress(char *hwName, EXT_MAC_ADDRESS *mac);

#endif

