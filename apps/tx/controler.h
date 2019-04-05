/*
 * Brief of the program:
 * There are Client and Server in a local network.
 * Client sends the data (on JSON format) by UDP protocol to Server for its parameters.
 * Server receives the Client's data and response Client's asks.
 * Example:
 * Client asks the equipment parameters (e.g.:MuxLab) of Server, and Server responses "MuxLab"(name).
 * On Client side, the data is packaged on JSON format = "Key-value pairs".
 * On Server side, it should parse the JSON format data using CJson embedded library.
 *
 * Team member: Marc BOHBOT, Peter JIANG, Dong LING and Serge ACOCA. 
 * Date: 20170214
 */

/*
 *File name: udp_server_json.h
 *
 * GCC version (ubuntu-16.04):
 * gcc (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0
 *
 * Compiler command format below:
 * $gcc main.c udp_server_json.c CJson/cJSON.c -o udp_server_json -lm 
 *
 * JSON library: CJson C language library.
 */

#ifndef __CONTROLER_H__
#define __CONTROLER_H__

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>  
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/reboot.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <net/route.h>
#include "system_info.h"

/* CJson embedded library is chose and it will be transplanted to an embedded board. */
#include "cJSON.h"
#include "mux_player.h"
#include "UDP.h"

#define DEFAULT_NETWORK_NAME "eth0"
#define SERVER_PORT (3600)
#define MAX_MSG_SIZE 65536
#define TAG (0xfaa0)
#define MAC_MISMATCH "MAC-MISMATCH"
#define MSG_CRC_FAIL "CRC-FAILED"
#define GET_STATUS    "get_status"

#define GET_SYS "get_sys"
#define GET_OUTPUT "get_output"
#define GET_INPUT  "get_input"
#define GET_STATUS "get_status"

#define HOST "www.muxlab.com"
#define PAGE "/"
#define PORT 12345
#define USERAGENT "Muxlab"

#define SWITCH_MAGIC 'S'
#define READ_SWITCH  _IOR(SWITCH_MAGIC, 1, unsigned int *) 

#define SYMBOL_CNT 200

#define GET_INPUTCMD(InputCmd) fgets((char *)(InputCmd), (sizeof(InputCmd) - 1), stdin)

#define REMOTE_IP "192.168.168.103"
#define IP_PORT 22222


// MDIO (768 board)
//#define MDIO_DRV_ENABLE     1

// IR  (currently not used) 
//#define IR_PASSTHROUGH_ENABLE 1

// Not Compleat All Info Packets, CEC and other (has to be added later) 
#define SIMPLE_ADV_PACKETS 1

// Use Button for Re-Init some FPGA registers (for Debug Only) 
#define BUTTON_FOR_FPGA_INIT 1

// Periodical SPI transaction (for Debug Only) 
#define SPI_TEST_ENABLE 1

// Select Debug flags by DIP Switch (for Debug Only) 
#define FLAGS_BY_DIP_SWITCH 1

#ifndef FLAGS_BY_DIP_SWITCH
#define MULTICAST_BY_SWITCH 1
#endif


#ifndef MAX_PATH
#define MAX_PATH 256
#endif

struct hardware_info
{
	int aqr_temp;
	int fpga_temp;
	float opr_margin_chnl_A;
	float opr_margin_chnl_B;
	float opr_margin_chnl_C;
	float opr_margin_chnl_D;
	float min_opr_margin_chnl_A;
	float min_opr_margin_chnl_B;
	float min_opr_margin_chnl_C;
	float min_opr_margin_chnl_D;
	float rx_power_ch_A;
	float rx_power_ch_B;
	float rx_power_ch_C;
	float rx_power_ch_D;	
	unsigned char reset_control;
	unsigned char video_config;
	unsigned char ir_control;
	unsigned char stream_en;
	unsigned char clock_status;
	unsigned char ir_modulation;
	unsigned short x_active;
	unsigned short y_active;
	unsigned short x_front_porch;
	unsigned short x_pulse_count;
	unsigned short x_back_porch;
	unsigned short y_front_porch;
	unsigned short y_pulse_count;
	unsigned short y_back_porch;	
	unsigned int   pclk_freq;
	unsigned char  audio_ch_sel;
	unsigned char  audio_ch_mask;
	unsigned char  fpga_version;
	unsigned char  fpga_revision;
	unsigned char  fpga_model;
	unsigned short packet_cnt_rx;
	unsigned short packet_cnt_tx;
	unsigned int   delta_pixel_cnt;
	unsigned int   prev_delta_pxixel_cnt;		
	unsigned char  raw_mode;
	unsigned char  color_space;
	unsigned char  color_depth;	
	unsigned char  mac_control_0;
	unsigned char  mac_control_1;
	unsigned char  mac_control_2;
	unsigned char  mac_control_3;
	unsigned char  mac_local[6];
	unsigned char  mac_dest[6];
	unsigned char  ip_addr_local[4];
	unsigned char  ip_addr_dest_v[4];
	unsigned char  ip_addr_dest_a[4];
};	

struct rtp_control_info
{
	unsigned int  rtp_load;
	unsigned char xaui_config;
	unsigned char xaui_status;
	unsigned int rtp_read_value;
};

struct fpga_ip_info
{
	unsigned char mac_control[4];
	unsigned char mac_local[6];
	unsigned char mac_dest[6];
	unsigned char ip_addr_local[4];
	unsigned char ip_addr_dest_v[4];
	unsigned char ip_addr_dest_a[4];
	unsigned short video_udp_port_local;
	unsigned short video_udp_port_dest;
	unsigned short audio_udp_port_local;
	unsigned short audio_udp_port_dest;
	unsigned short qos_tci;
	unsigned char multicast_ip_rx_v[4];
	unsigned char multicast_ip_rx_a[4];
};
	


unsigned char connection_enable_flag;

typedef unsigned char		bool;
typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;




/* Keeping a default parameter list on JSON format for Server. */
//char *PARAMETER_LIST_JSON;
//char *PARAMETER_LIST_UPDATED_JSON;
//char *PARAMETER_ERROR_REPLY;
/* 8Bytes = TAG-2Bytes + LENGTH-2Bytes + CRC-4Bytes */
//extern char JSON_DATA_COPIED_FROM_MSG[MAX_MSG_SIZE];
//extern unsigned char JSON_DATA_COPIED_FROM_MSG[65527];
//char recieved_string[256];
//char default_string[256];
//unsigned int DO_NOTHING_FLAG;
/* Configuration network parameters for establish UDP communication */
int udp_config(void);
cJSON *parameter_list_json_create();
void udp_json_parse(int sockCtrl, cJSON* root_msg);
int controler();
int write_request(int key_ref, unsigned char *value_requested);
unsigned int readDipSwitch();
char* getStrFrmObject(cJSON* json, const char * key);
int CheckMacAddr(char *mac_addr_string);
int getMac(char * mac, int len_limit);
void setip(unsigned char * ip);
void getip(unsigned char * ret, int param);
void fpga_ip_addr_inf(cJSON * root_msg);
void parse_ip_to_bytes(unsigned char * ip_in, unsigned char * data_write);
void parse_bytes_to_ip(unsigned char *data_read, unsigned char * ip_out);
void parse_mac_to_bytes(unsigned char * mac_in, unsigned char * data_write);
void parse_bytes_to_mac(unsigned char *data_read, unsigned char * mac_out);
void parse_ip_to_mcast_mac(unsigned char * ip_in, unsigned char * data_write);
byte I2C_Write(byte deviceID, byte offset, byte *buffer, word length);
byte I2C_Read(byte deviceID, byte offset, byte *buffer, word length);
char* GetGatewayForInterface(const char* interface);
void set_gateway(unsigned char* gateway);
unsigned int crc32b(unsigned char *message);
int read_edid_from_file(char *edid_str);
int write_edid_to_file(char *edid_str);
void StrToHex(char *pbDest, char *pbSrc, int nLen);
int readParamFromDrive(char * partitionName, char * param, int bufLens);
int saveParamToDrive(char * partitionName, char * param, int strlength);
int eraseParamDrive(char * partitionName);
unsigned char Read_FPGA_IR_Emitter_Freqency();
int Write_FPGA_IR_Emitter_Freqency(unsigned char frequency);
int Set_FPGA_IR_Mode(int mode);
int get_hardware_info(struct hardware_info *hard_info);
int save_fpga_log(struct hardware_info *hard_info);
int save_edid_log();
#endif
