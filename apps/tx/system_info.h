
#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*=======================================
 * Data types
 *======================================*/

/*=======================================
 * 
 *======================================*/

#define EDID_DEFAULT	"/tmp/app/edid_default.bin"
#define EDID_CONFIG_INI "/dev/mtdblock6"
#define CONFIG_INI  "/dev/mtdblock5"
#define SECURITY_DEVICE "/dev/muxds"
#define BLOCK_LENGTH (64*1024)

#define FPGA_LOG_FILE "/tmp/log/fpga.log"
#define ADV_LOG_FILE  "/tmp/log/adv.log"
#define EDID_LOG_FILE "/tmp/log/edid.log"

//int saveParamToDrive(char * partitionName, char * param);
//int readParamFromDrive(char * partitionName, char * param, int bufLens);
//int eraseParamDrive(char * partitionName);
void GetGateWay(char * gateway);
void getNetmask(char *hw_name, char *netmask);
int getLocalIp(char * hw_name, char *ip);

#endif
