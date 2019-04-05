#include "controler.h"
#include "mux_player.h"
#include "multithread.h"
#include "globals.h"
#include "sha256_software.h"
#include "serial.h" 
#include "link_10G.h"
#include "led.h"

#define port_name "eth0"

enum get_net
{
	GET_IP,
	GET_MASK,
	GET_BCAST_ADDR,
	GET_DEST_ADDR	
};


typedef enum {
	msg_targ,
	msg_cmd,
	msg_login_ack,
	msg_pwd_msg,
	msg_data,
	msg_login,
	msg_password,
	msg_pName,
	msg_model,
	msg_fwVer,
	msg_cName,
	msg_mac,
	msg_ip,	
	msg_mask,
	msg_gateway,
	msg_isDhcp,
	msg_isDipOn,
	msg_isReset,	
	msg_isReboot,
	msg_IRState,
	msg_IRFeedbackIp,
	msg_RS232Baudrate,
	msg_RS232Databits,
	msg_RS232Parity,
	msg_RS232Stopbits,
	msg_RS232FeedbackIp,
	msg_VideoPortLocal,
	msg_VideoPortDest,
	msg_isConnect,
	msg_isMCAST,
	msg_MCASTip,
	msg_MCASTport,	
	msg_FPGAVer,	
	msg_res,
	msg_isRawMode,
	msg_x_FrontPorch,
	msg_x_PulseCount,
	msg_x_BackPorch,
	msg_y_FrontPorch,
	msg_y_PulseCount,
	msg_y_BackPorch,
	msg_ColorSpace,
	msg_ColorDepth,
	msg_HdcpMode,
	msg_CPU_Usage,								
    msg_Memory_Usage,								   
	msg_num											 
} RSV_MSG;


//extern const HASH_MAP default_state[msg_num] = {
const HASH_MAP default_state[msg_num] = {
	{ "targ", "00:22:33:44:55:66" },
	{ "cmd", "get_param" },
	{ "login_ack", "OK" },
	{ "pwd_msg", "" },
	{ "data", "" },
	{ "login", "" },
	{ "password", "" },
	{ "pName", "HdmiOverIP-774" },
	{ "model", "000774-TX" },
	{ "fwVer", "0.0.3" },
	{ "cName", "myTXname" },
	{ "mac", "00:12:23:34:56:78" },
	{ "ip", "10.0.1.175" },
	{ "mask", "255.255.255.0" },	
	{ "gateway", "10.0.1.1" },	
	{ "isDhcp", "0" },                             
	{ "isDipOn", "1" },
	{ "isReset", "0" },	
	{ "isReboot", "0" },
	{ "IRState", "2" },
	{ "IRFeedbackIp", "10.0.1.176" },
	{ "RS232Baudrate", "9600" },
	{ "RS232Databits", "8" },
	{ "RS232Parity", "none" },
	{ "RS232Stopbits", "1" },
	{ "RS232FeedbackIp", "10.0.1.176" },	
	{ "VideoPortLocal", "49712" },
	{ "VideoPortDest", "49696" },
	{ "isConnect", "1" },
	{ "isMCAST", "1" },
	{ "MCASTip", "239.100.0.254" },	
	{ "MCASTport", "37000" },		
	{ "FPGAVer", "1.0"},	
	{ "res", "1920x1080" },
	{ "isRawMode", "0"},
	{ "X_FrontPorch", "0" },
	{ "X_PulseCount", "0"},
	{ "X_BackPorch", "0"},
	{ "Y_FrontPorch", "0" },
	{ "Y_PulseCount", "0"},
	{ "Y_BackPorch", "0"},
	{ "ColosSpace", "RGB"},
	{ "ColorDepth", "8bit"},	
	{ "HdcpMode", "Off"},
	{ "CPU_Usage", "" },
	{ "Memory_Usage", "" }
};

extern CEC_Cmd_Data g_CEC_Cmd_Data;
extern char RS232_REMOTE_IP[16];
extern int rs232PauseFlag;
extern char IR_REMOTE_IP[16];
extern int irPauseFlag;
extern unsigned short IR_MAGIC_HEADER;

//const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int REQUEST_SUCCESS_FLAG = 0;
int DO_NOTHING_FLAG = 0;
int SAVE_FLAG = 0;
int dip_enabled_flag = 0;
int dip_check_flag = 0;
int DipOnStatus = 0;	//0--dip switch disabled, 1--dip switch enabled
int led_flash;
int setUartFlag = 0;
int isRebootOn = 0;
int isResetOn = 0;
char g_multicast_ip[16];

pthread_cond_t cv;
pthread_mutex_t led_lock;

char *json_value_update(cJSON* root_msg, char * received_msg);
int build_reply(char *reply_message, char *json_message);
char *correct_value(char *error_value, char *original_value);
int clean_json(char *input_JSON, unsigned char *output);
extern void HexToStr(char *pbDest, char *pbSrc, int nLen);

extern int initIR();
extern void deinitIR();
extern int initIRPassThrough();
extern int deinitIRPassThrough();
extern int irPassThrough();
extern int setChipIRMode(char * mode);
extern int controlIR_rawdata(int timeout, unsigned short *ir_data);
extern unsigned char ReadIRMode();

unsigned int crc32b(unsigned char *message) 
{
	int i, j;
	unsigned int byte, crc, mask;

	i = 0;
	crc = 0xFFFFFFFF;
	while (message[i] != 0) 
	{
		byte = message[i];             // Get next byte.
		crc = crc ^ byte;
		for (j = 7; j >= 0; j--) 
		{   // Do eight times.
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		i = i + 1;
	}
	return ~crc;
}

int udp_config(void) 
{
	struct sockaddr_in addr;
	int sockCtrl;

	sockCtrl = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockCtrl < 0) 
	{
		fprintf(stderr, "Socket Error:%s\n", strerror(errno));
		return -1;
	}

	bzero(&addr, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);

	if (bind(sockCtrl, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "UDP communication Bind Error:%s\n", strerror(errno));
	//	exit(0);
		return -2;
	}

	return sockCtrl;
}

cJSON *parameter_list_json_create() 
{
	cJSON *json_root = cJSON_CreateObject();
	cJSON *item_array = cJSON_CreateArray();
	cJSON *parameter_pool = cJSON_CreateObject();
	int ret;
	int i;
	for (i = 0; i < msg_data; i++) 
	{
		cJSON_AddItemToObject(json_root, default_state[i].key, cJSON_CreateString(default_state[i].value));
	}

	cJSON_AddItemToObject(json_root, default_state[msg_data].key, item_array);
	cJSON_AddItemToObject(item_array, default_state[msg_data].key, parameter_pool);

	//////////////////////////Update value with existing config.ini////////////////////
	cJSON * json_root_ref = NULL;    // = cJSON_CreateObject();	//Existing config file
	cJSON *json_array_ref = NULL;
	cJSON *json_object_parameter_pool_ref = NULL;
	char buffer[MAX_MSG_SIZE];

	if (access(CONFIG_INI, F_OK) != -1) 
	{ 	
		//config file exists		
		int status = readParamFromDrive(CONFIG_INI, buffer, MAX_MSG_SIZE);
		if (0 == status) 
		{
			json_root_ref = cJSON_Parse(buffer);
			if (json_root_ref) 
			{
				json_array_ref = cJSON_GetObjectItem(json_root_ref, default_state[msg_data].key);
				if (json_array_ref != NULL) 
				{
					json_object_parameter_pool_ref = cJSON_GetArrayItem(json_array_ref, 0);
				}
			}
		}
	}
	//sleep(1);
	for (i = msg_login; i < msg_num; i++) 
	{
		cJSON_AddItemToObject(parameter_pool, default_state[i].key, cJSON_CreateString(default_state[i].value));

		//replace with existing config file's value if comfortable
		if (json_object_parameter_pool_ref != NULL) 
		{
			if (i != msg_fwVer) 
			{
				char *content_of_value = getStrFrmObject(json_object_parameter_pool_ref, default_state[i].key);
				if (content_of_value != NULL) 
				{
					cJSON_ReplaceItemInObject(parameter_pool, default_state[i].key, cJSON_CreateString(content_of_value));
				}
			}
		}		
	}

	if (json_root_ref)
		cJSON_Delete(json_root_ref);
	
	
	char mac_addr[6];		
	char mac_addr_str[20];	
	char model_id_str[20];
	mac_addr[0] = 0x00;
	for (i = 1; i < 6; i++)  //set a random mac in case of read chip error or first time
		mac_addr[i] = rand() % 256;
		
	for (i = 0; i < 5; i++)
	{		
		ret = SHA256_Read_Mac_ModelID_Once(mac_addr_str, model_id_str);
		if (ret == 0)
			break;					
	}
	if (i == 5)	//get mac from chip fail
	{
		sprintf(mac_addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);		
		sprintf(model_id_str, "500768-TX");
	}	
	//sprintf(mac_addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);		
	sprintf(model_id_str, "500768-TX");
	
	if (CheckMacAddr(mac_addr_str) == 0) 
	{
		cJSON_ReplaceItemInObject(parameter_pool, default_state[msg_mac].key, cJSON_CreateString(mac_addr_str));
		cJSON_ReplaceItemInObject(json_root, default_state[msg_targ].key, cJSON_CreateString(mac_addr_str));
	}			
	cJSON_ReplaceItemInObject(parameter_pool, default_state[msg_model].key, cJSON_CreateString(model_id_str));	
	return json_root;
}

char *create_err_msg(char * err, cJSON* root) {

	cJSON *json_err_reply = cJSON_CreateObject();
	cJSON *item_array = cJSON_CreateArray();
	cJSON *parameter_pool = cJSON_CreateObject();

	cJSON_AddItemToObject(json_err_reply, default_state[msg_targ].key, cJSON_CreateString(cJSON_GetObjectItem(root, default_state[msg_targ].key)->valuestring));
	cJSON_AddItemToObject(json_err_reply, default_state[msg_cmd].key, cJSON_CreateString(cJSON_GetObjectItem(root, default_state[msg_cmd].key)->string));
	cJSON_AddItemToObject(json_err_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
	cJSON_AddItemToObject(json_err_reply, default_state[msg_pwd_msg].key, cJSON_CreateString(err));
	cJSON_AddItemToObject(json_err_reply, default_state[msg_data].key, item_array);
	cJSON_AddItemToObject(item_array, default_state[msg_data].key, parameter_pool);

	char* reply = cJSON_Print(json_err_reply);

	cJSON_Delete(json_err_reply);

	return reply;
}

cJSON *create_err_msg_template() {

	cJSON *json_root = cJSON_CreateObject();
	cJSON *item_array = cJSON_CreateArray();
	cJSON *parameter_pool = cJSON_CreateObject();

	cJSON_AddItemToObject(json_root, "targ", cJSON_CreateString(""));
	cJSON_AddItemToObject(json_root, "cmd", cJSON_CreateString(""));
	cJSON_AddItemToObject(json_root, "login_ack", cJSON_CreateString(""));
	cJSON_AddItemToObject(json_root, "pwd_msg", cJSON_CreateString(""));
	cJSON_AddItemToObject(json_root, "data", item_array);
	cJSON_AddItemToObject(item_array, "data", parameter_pool);
	return json_root;
}

///**********************************************************
///**********************************************************

void udp_json_parse(int sockCtrl, cJSON* root_msg) 
{
	struct sockaddr_in addr;
	int n;
	socklen_t addrlen;
	char msg[MAX_MSG_SIZE];
	int replyLen;
	unsigned int CRC_Decoded, CRC_Received;
	unsigned char CRC_Buffer[4];
	unsigned char JSON_DATA_COPIED_FROM_MSG[2048]; 
	unsigned char tag_command[] = { 0xfa, 0xa0 };
	unsigned char tag_response[] = { 0xfb, 0xa0 };
	unsigned int length_json_data;
	char msg_reply[MAX_MSG_SIZE];

	cJSON *json_root_default = root_msg;
	cJSON *json_array_default;
	cJSON *json_object_parameter_pool_default;

	json_array_default = cJSON_GetObjectItem(json_root_default, default_state[msg_data].key);
	json_object_parameter_pool_default = cJSON_GetArrayItem(json_array_default, 0);

	while (1)
	{		
		memset(msg, 0, sizeof(msg));
		addrlen = sizeof(struct sockaddr);
		n = recvfrom(sockCtrl, msg, MAX_MSG_SIZE, 0, (struct sockaddr *) &addr, &addrlen);

		REQUEST_SUCCESS_FLAG = 0;                                                                  

		if (tag_command[0] == msg[0] && tag_command[1] == msg[1]) 
		{
			printf("\n\nA valid command recieved!\n\n");
			length_json_data = msg[2] << 8 | msg[3];

			memset(JSON_DATA_COPIED_FROM_MSG, 0, sizeof(JSON_DATA_COPIED_FROM_MSG));
			memcpy(JSON_DATA_COPIED_FROM_MSG, msg + 4, length_json_data - 4);

			printf("Recieved JSON data is \n\n %s\n", JSON_DATA_COPIED_FROM_MSG);
		
			CRC_Decoded = crc32b(JSON_DATA_COPIED_FROM_MSG);
			CRC_Buffer[0] = msg[length_json_data];
			CRC_Buffer[1] = msg[length_json_data + 1];
			CRC_Buffer[2] = msg[length_json_data + 2];
			CRC_Buffer[3] = msg[length_json_data + 3];
			CRC_Received = CRC_Buffer[0] << 24 | CRC_Buffer[1] << 16 | CRC_Buffer[2] << 8 | CRC_Buffer[3];

			if (CRC_Received != CRC_Decoded) 
			{	//Wrong CRC				
				char * tmp;
				tmp = create_err_msg(MSG_CRC_FAIL, root_msg);
				n = build_reply(msg_reply, tmp);
				if (tmp)
					free(tmp);
				//build_packet
			    sendto(sockCtrl, msg_reply, n /*+ 8*/, 0, (struct sockaddr *) &addr, addrlen);  //  if suceeded the updated data is send back
			}
			else 
			{		     
				char *json_to_replay;
				json_to_replay = json_value_update(root_msg, JSON_DATA_COPIED_FROM_MSG);				   
				if (DO_NOTHING_FLAG != 1) 
				{
					if (REQUEST_SUCCESS_FLAG == 1) 
					{  //n = strlen(PARAMETER_LIST_UPDATED_JSON);			    
						memset(msg_reply, 0, sizeof(msg_reply));
						replyLen = build_reply(msg_reply, json_to_replay);			     
						sendto(sockCtrl, msg_reply, replyLen /*+ 8*/, 0, (struct sockaddr *) &addr, addrlen);  //  if suceeded the updated data is send back
					}
					else if (REQUEST_SUCCESS_FLAG == 0) 
					{
						//n = strlen(PARAMETER_ERROR_REPLY);			     
						memset(msg_reply, 0, sizeof(msg_reply));
						replyLen = build_reply(msg_reply, json_to_replay);			     
						sendto(sockCtrl, msg_reply, replyLen /*+ 8*/, 0, (struct sockaddr *) &addr, addrlen);  //  if suceeded the updated data is send back
					}
				}
				else 
				{
					char * tmp;
					tmp = create_err_msg(MAC_MISMATCH, root_msg);			     	    
					memset(msg_reply, 0, sizeof(msg_reply));
					replyLen = build_reply(msg_reply, tmp);			     	    
					//build_packet
					sendto(sockCtrl, msg_reply, replyLen /*+ 8*/, 0, (struct sockaddr *) &addr, addrlen);  //  if suceeded the updated data is send back
				}
				if (json_to_replay)
					free(json_to_replay);
				if (isResetOn == 1)
				{
					eraseParamDrive(CONFIG_INI);
					eraseParamDrive(EDID_CONFIG_INI);
					isRebootOn = 1;
					printf("factory default.\n");
				}
				if (isRebootOn == 1) 
				{
					isRebootOn = 0;
					//Cleanup system and reboot					
					system("//usr/sbin/avahi-daemon -k");
					sync(); //
					system("/bin/umount -a -r");	//unmount sd card, usb key, ...
					reboot(RB_AUTOBOOT);					
				}
			}
		}
	}
}

char *json_value_update(cJSON* root_msg, char * received_msg) {

	cJSON *json_root_default = root_msg;
	cJSON *json_array_default;
	cJSON *json_object_parameter_pool_default;

	cJSON *json_root_recieved;
	cJSON *json_array_recieved;
	cJSON *json_object_parameter_pool_recieved;

	cJSON *json_root_error_reply = create_err_msg_template();

	char *default_value;
	
	char *GET_PARAM = "get_param";
	char *SET_PARAM = "set_param";
	char *SEND_DATA_RS232 = "send_data_rs232";
	char *SEND_DATA_IR = "send_data_ir";
	char *SECURITY_CHECK_SHA256 = "security_check_sha256";	
	char *EDID_MANAGE = "edid_manage";	
	char *SEND_CMD_CEC = "send_cmd_cec";	
	char *SET_CMD = "set_cmd";
	
	char *empty_mac = "";

	char *production_passwd = "admin";
	char *production_uname = "Angelica";

	char *success1 = "SUCCESS";
	char *Acknwoldge = "OK";
	char *Not_Acknwoldge = "NOK";

	char *error5 = "PERMISSION-LIMITED";

	int prod_error_flag;
	int set_mac_model_flag;
	int success_flag;
	int return_value;

	char *default_dip_status;
	char *recieved_dip_status;

	RSV_MSG i_key_list;
	
	struct hardware_info sys_hard_info;

	i_key_list = msg_data;

	prod_error_flag = 0;
	set_mac_model_flag = 0;
	success_flag = 0;
	dip_check_flag = 0;

	DO_NOTHING_FLAG = 0;   // global variable to check whether it is my Mac, if it is no my mac then do nothing
	REQUEST_SUCCESS_FLAG = 0;   // This is for indicating whether the request is suceeded
	SAVE_FLAG = 0;

	json_array_default = cJSON_GetObjectItem(json_root_default, default_state[msg_data].key);
	json_object_parameter_pool_default = cJSON_GetArrayItem(json_array_default, 0);

	json_root_recieved = cJSON_Parse(received_msg);  // work on here to check the mac, cmd, uname, passwd

	if(json_root_recieved != NULL)
	{
		if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_data].key) == NULL)
		{
			json_array_recieved = cJSON_CreateArray();
			json_object_parameter_pool_recieved = cJSON_CreateObject();
			cJSON_AddItemToObject(json_root_recieved, default_state[msg_data].key, json_array_recieved);
			cJSON_AddItemToObject(json_array_recieved, default_state[msg_data].key, json_object_parameter_pool_recieved);
			int i;
			for (i = msg_login; i < msg_num; i++)
				cJSON_AddItemToObject(json_object_parameter_pool_recieved, default_state[i].key, cJSON_CreateString(default_state[i].value));
		}					
		else
		{
			json_array_recieved = cJSON_GetObjectItem(json_root_recieved, default_state[msg_data].key);
			json_object_parameter_pool_recieved = cJSON_GetArrayItem(json_array_recieved, 0);
		}
				
		if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_targ].key) != NULL)
		{
			// found a Mac address
			char received_string[256];
			char default_string[256];
			
			char * value_of_key_recieved = cJSON_GetObjectItem(json_root_recieved, default_state[msg_targ].key)->valuestring;
			correct_value(value_of_key_recieved, received_string);  /// this is the mac address in the recieved message

			default_value = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_mac].key)->valuestring;  /// this is the mac address of the system
			correct_value(default_value, default_string);

			if (strcmp(received_string, default_string) == 0 || strcmp(received_string, empty_mac) == 0) // checking the mac	for unicast
				{
					//mac address matching
					if(strcmp(received_string, empty_mac) == 0)
					{
						printf(" \n recieved a packet with empty MAC \n");
						default_value = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_mac].key)->valuestring;  /// this is the mac address of the system
						correct_value(default_value, default_string);
						cJSON_ReplaceItemInObject(json_root_recieved, default_state[msg_targ].key, cJSON_CreateString(default_string));  // saving Mac for error reply
					}
					cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_targ].key, cJSON_CreateString(default_string));  // saving Mac for error reply

					if(cJSON_GetObjectItem(json_root_recieved, default_state[msg_cmd].key) != NULL) // checking if there is a command
					{
						//found a command
						value_of_key_recieved = cJSON_GetObjectItem(json_root_recieved, default_state[msg_cmd].key)->valuestring;
						correct_value(value_of_key_recieved, received_string);
						// the request is for get the parameters
						
						if(strcmp(received_string, GET_PARAM) == 0)
						{
							cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_cmd].key, cJSON_CreateString(received_string));  // saving command for error reply
							
							pthread_mutex_lock(&i2c_bus_lock);
							get_hardware_info(&sys_hard_info);
							pthread_mutex_unlock(&i2c_bus_lock);
							save_fpga_log(&sys_hard_info);
							save_edid_log();
							for (i_key_list = msg_login; i_key_list < msg_num; i_key_list++) 
							{
								memset(default_string, 0, sizeof(default_string));
							
								switch (i_key_list) 
								{
									struct sockaddr_in addr4;
									char ipaddr[30] = "";
									char para_buff[40] = "";
									memset(ipaddr, 0, 30);
									
									case msg_ip:
										if (getLocalIp(DEFAULT_NETWORK_NAME, ipaddr) == 0) 
										{
											if (ipaddr[0] != 0 && (inet_pton(AF_INET, ipaddr, &addr4.sin_addr) > 0))
												cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_ip].key, cJSON_CreateString(ipaddr));
											strncpy(default_string, ipaddr, 15);											
										}
									break;
							
									case msg_mask:
										getNetmask(DEFAULT_NETWORK_NAME, ipaddr);
										if (ipaddr[0] != 0 && (inet_pton(AF_INET, ipaddr, &addr4.sin_addr) > 0)) 
										{
											cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_mask].key, cJSON_CreateString(ipaddr));
											strncpy(default_string, ipaddr, 15);											
										}
									break;
								
									case msg_gateway:
										GetGateWay(ipaddr);
										if (ipaddr[0] != 0 && (inet_pton(AF_INET, ipaddr, &addr4.sin_addr) > 0)) 
										{
											cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_gateway].key, cJSON_CreateString(ipaddr));
											strncpy(default_string, ipaddr, 15);											
										}
									break;							
																		
									case msg_x_FrontPorch:
										sprintf(para_buff, "%d", sys_hard_info.x_front_porch);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_x_FrontPorch].key, cJSON_CreateString(para_buff));										
									break;				

									case msg_x_PulseCount:
										sprintf(para_buff, "%d", sys_hard_info.x_pulse_count);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_x_PulseCount].key, cJSON_CreateString(para_buff));										
									break;				

									case msg_x_BackPorch:
										sprintf(para_buff, "%d", sys_hard_info.x_back_porch);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_x_BackPorch].key, cJSON_CreateString(para_buff));										
									break;													
									
									case msg_y_FrontPorch:
										sprintf(para_buff, "%d", sys_hard_info.y_front_porch);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_y_FrontPorch].key, cJSON_CreateString(para_buff));										
									break;				

									case msg_y_PulseCount:
										sprintf(para_buff, "%d", sys_hard_info.y_pulse_count);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_y_PulseCount].key, cJSON_CreateString(para_buff));										
									break;				

									case msg_y_BackPorch:
										sprintf(para_buff, "%d", sys_hard_info.y_back_porch);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_y_BackPorch].key, cJSON_CreateString(para_buff));										
									break;				

									case msg_res:
										sprintf(para_buff, "%d x %d", sys_hard_info.x_active, sys_hard_info.y_active);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_res].key, cJSON_CreateString(para_buff));										
									break;													
									
									case msg_isRawMode:
										sprintf(para_buff, "%d", sys_hard_info.raw_mode);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_isRawMode].key, cJSON_CreateString(para_buff));										
									break;											
								
									case msg_ColorSpace:
										switch (sys_hard_info.color_space)
										{
											case 0: sprintf(para_buff, "%s", "YCbCr444"); break;
											case 1: sprintf(para_buff, "%s", "YCbCr422"); break;
											case 2: sprintf(para_buff, "%s", "x420"); break;
											case 3: sprintf(para_buff, "%s", "x_raw"); break;
											case 4: sprintf(para_buff, "%s", "RGB"); break;					
										}																	
										sprintf(para_buff, "%d", sys_hard_info.color_space);
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_ColorSpace].key, cJSON_CreateString(para_buff));										
									break;									
									
									case msg_ColorDepth:										
										switch(sys_hard_info.color_depth)
										{
											case 0: sprintf(para_buff, "%s", "8 bit"); break;
											case 1: sprintf(para_buff, "%s", "10 bit"); break;
											case 2: sprintf(para_buff, "%s", "12 bit"); break;
											case 3: sprintf(para_buff, "%s", "16 bit"); break;
										}	
										cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_ColorDepth].key, cJSON_CreateString(para_buff));
									break;	
									
									default:
										default_value = getStrFrmObject(json_object_parameter_pool_default, default_state[i_key_list].key);
										correct_value(default_value, default_string);
									break;
								}

								cJSON_ReplaceItemInObject(json_object_parameter_pool_recieved, default_state[i_key_list].key, cJSON_CreateString(default_string));
								
							}	
							success_flag = 1;
							SAVE_FLAG = 1;							
						}
					
						else if(strcmp(received_string, SET_PARAM) == 0)
						{				  							
							//Remove the unsupported or wrong attrbutes in incoming command
							cJSON *cJsonObj = json_object_parameter_pool_recieved->child;
							while (cJsonObj) 
							{
								cJSON *objTmp = cJsonObj;
								cJsonObj = cJsonObj->next;
								if (cJSON_GetObjectItem(json_object_parameter_pool_default,	objTmp->string) == NULL) 
								{								
									cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, objTmp->string);
								}
							}							
							
							cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_cmd].key, cJSON_CreateString(received_string));  // saving command for error reply
							
							if(cJSON_GetObjectItem(json_root_recieved, default_state[msg_login_ack].key) != NULL) // checking if there is a user name
							{
								value_of_key_recieved = cJSON_GetObjectItem(json_root_recieved, default_state[msg_login_ack].key)->valuestring;
								correct_value(value_of_key_recieved, received_string);

								default_value = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_login].key)->valuestring;
								correct_value(default_value, default_string);

								if (strcmp(received_string, default_string) == 0 || strcmp(received_string, production_uname) == 0) // checking the username
									{												
										if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_pwd_msg].key) != NULL) // checking if there is a password feild
										{
											value_of_key_recieved = cJSON_GetObjectItem(json_root_recieved, default_state[msg_pwd_msg].key)->valuestring;
											correct_value(value_of_key_recieved, received_string);
											default_value = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_password].key)->valuestring;
											correct_value(default_value, default_string);
									
											if (strcmp(received_string, default_string) == 0 || strcmp(received_string, production_passwd) == 0) // checking the password
											{				
												for (i_key_list = msg_login; i_key_list < msg_num; i_key_list++)																																																			 
												{																											 																																																	 
													if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[i_key_list].key) != NULL)																													 
													{
														value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[i_key_list].key)->valuestring;
														if (i_key_list == msg_pName || i_key_list == msg_model || i_key_list == msg_mac)
														{																																 																																																		 
															//	printf("\n\n These value can only be changed by production: Value requested is  %s  Value in the system is  %s \n\n", corrected_value, default_string);													 
														 	if(cJSON_GetObjectItem(json_root_recieved, default_state[msg_login_ack].key) != NULL) // checking if there is a user name																					 
															{																																																			 
																value_of_key_recieved = cJSON_GetObjectItem(json_root_recieved, default_state[msg_login_ack].key)->valuestring;																											 
																correct_value(value_of_key_recieved, received_string);																																					 
																if (strcmp(received_string, production_uname) == 0)  // checking the username																															 
																{																																																		 
																	if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_pwd_msg].key) != NULL) // checking if there is a password feild																		 
																	{																																																	 
																		value_of_key_recieved =	cJSON_GetObjectItem(json_root_recieved, default_state[msg_pwd_msg].key)->valuestring;
																		correct_value(value_of_key_recieved, received_string);																																			 
																																																												 
																		if (strcmp(received_string, production_passwd) == 0)  // checking the password																													 
																		{																								
																			value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[i_key_list].key)->valuestring;																 
																			correct_value(value_of_key_recieved, received_string);	
																			if ((i_key_list == msg_mac)||(i_key_list == msg_model))
																				set_mac_model_flag = 1;																				
																		}																																																 
																		else //set mac or model fail because of WRONG password
																		{																																															 
																			//// this operation is not allowed since the password doent match with production password																																	 
																			cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
																			cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("PASSWORD-MISMATCH"));																			 
																			prod_error_flag = 1;																																										 																																												 
																		}																																																 
																	}
																	else  //set mac or model fail because of NO password field
																	{
																		//// this operation is not allowed since the password is missing
																		cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
																		cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("PASSWORD-MISSING"));
																		prod_error_flag = 1;	
																	}
																}																																																		 
																else  //set mac or model fail because of NO production field
																{																																																	 
																	//// this operation is not allowed since the username doesnt match with the production username																																			 
																	cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
																	cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISMATCH"));																			 
																	prod_error_flag = 1;																																												 
																}																																																		 
															}
															else
															{
																//// this operation is not allowed since the username is missing
																cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
																cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISSING"));
																prod_error_flag = 1;	
															}													
														}																																																				 																																						                                                                             
														else  //NOT (i_key_list == msg_pName || i_key_list == msg_model || i_key_list == msg_mac)
														{
															value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[i_key_list].key)->valuestring;
															correct_value(value_of_key_recieved, received_string);
															
															
															if (i_key_list == msg_isDhcp) 
															{
																if (strcmp(received_string, "0") == 0)	//dhcp is off, static ip
																{  
																	if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[msg_ip].key) != NULL)
																	{
																		value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[msg_ip].key)->valuestring;
																		correct_value(value_of_key_recieved, received_string);	
																		return_value = write_request(msg_ip, received_string);
																	}
																	else
																	{
																		value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_ip].key)->valuestring;
																		correct_value(value_of_key_recieved, received_string);	
																		return_value = write_request(msg_ip, received_string);
																	}
																	if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[msg_mask].key) != NULL)
																	{
																		value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[msg_mask].key)->valuestring;
																		correct_value(value_of_key_recieved, received_string);		
																		return_value = write_request(msg_mask, received_string);
																	}
																	else
																	{
																		value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_mask].key)->valuestring;
																		correct_value(value_of_key_recieved, received_string);		
																		return_value = write_request(msg_mask, received_string);
																	}
														
																	if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[msg_gateway].key) != NULL)
																	{
																		value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[msg_gateway].key)->valuestring;
																		correct_value(value_of_key_recieved, received_string);	
																		return_value = write_request(msg_gateway, received_string);
																	}
																	else
																	{
																		value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_gateway].key)->valuestring;
																		correct_value(value_of_key_recieved, received_string);																																												 
																		return_value = write_request(msg_gateway, received_string);
																	}
																	//Retrieve the received DHCP value
																	value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[i_key_list].key)->valuestring;
																	correct_value(value_of_key_recieved, received_string);
																	return_value = write_request(i_key_list, received_string);
																	
																	success_flag = 1;			
																}
																else
																{
																	//Received DHCP value is 1
																	char buffer1[256];
																	unsigned char * gateway;
																	value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[i_key_list].key)->valuestring;
																	correct_value(value_of_key_recieved, received_string);
																	return_value = write_request(i_key_list, received_string);
																	getip(buffer1, GET_IP);
																	cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_ip].key, cJSON_CreateString(buffer1));
																	getip(buffer1, GET_MASK);
																	cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_mask].key, cJSON_CreateString(buffer1));
																	gateway = GetGatewayForInterface("eth0");
																	cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_gateway].key, cJSON_CreateString(gateway));
																	success_flag = 1;
																}
															}																																																																													 
															else 
															{
																// Not mac,pname,model or DHCP	
																value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[i_key_list].key)->valuestring;
																correct_value(value_of_key_recieved, received_string);
																return_value = write_request(i_key_list, received_string);
																success_flag = 1;																																				                                                                                                             
															}	
															
															value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[i_key_list].key)->valuestring;
															correct_value(value_of_key_recieved, received_string); 
															return_value = write_request(i_key_list, received_string);
															success_flag = 1;
															
															
														}
														if (return_value == 0)                              
														{																										                                 
															SAVE_FLAG = 1;																																											 
															cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[i_key_list].key, cJSON_CreateString(received_string));														 
														}	
													}                                                                                                                                                                                                                                                                                                                                                                                                                                     
													else	//i_key_list empty input string
													{																																																				 																																	 																																																							 														 																																																											 
														default_value = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i_key_list].key)->valuestring;																							 
														correct_value(default_value, default_string); 																																									 																																																																																		//cJSON_ReplaceItemInObject(json_object_parameter_pool_recieved, default_state[i_key_list], cJSON_CreateString(default_string));																				 
														cJSON_AddItemToObject(json_object_parameter_pool_recieved, default_state[i_key_list].key, cJSON_CreateString(default_string));																					 
														success_flag = 1;																																																 
													}																																																					 
												} // end of for i_key_list loop
												
												if(set_mac_model_flag == 1)			//write mac address and model id to security chip										
												{
													set_mac_model_flag = 0;
													char mac_addr_str[20];	
													char model_id_str[20];													
													value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, "mac")->valuestring;		
													correct_value(value_of_key_recieved, mac_addr_str);
													value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, "model")->valuestring;		
													correct_value(value_of_key_recieved, model_id_str);													
													if (SHA256_Write_Mac_ModelID_Once(mac_addr_str, model_id_str) == 0)
													{
														cJSON_ReplaceItemInObject(json_object_parameter_pool_default, "mac", cJSON_CreateString(mac_addr_str));		
														cJSON_ReplaceItemInObject(json_object_parameter_pool_default, "model", cJSON_CreateString(model_id_str));	
														success_flag = 1;	
														SAVE_FLAG = 1;	
													}																									
												}																			
												
												/*
												if(set_mac_model_flag == 1)			//write mac address and model id to security chip										
												{
													set_mac_model_flag = 0;
													char mac_addr_str[20];	
													char model_id_str[20];													
													value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, "mac")->valuestring;		
													correct_value(value_of_key_recieved, mac_addr_str);
													value_of_key_recieved = cJSON_GetObjectItem(json_object_parameter_pool_recieved, "model")->valuestring;		
													correct_value(value_of_key_recieved, model_id_str);													
													if (SHA256_Write_Mac_ModelID_Once(mac_addr_str, model_id_str) == 0)
													{
														cJSON_ReplaceItemInObject(json_object_parameter_pool_default, "mac", cJSON_CreateString(mac_addr_str));		
														cJSON_ReplaceItemInObject(json_object_parameter_pool_default, "model", cJSON_CreateString(model_id_str));	
														success_flag = 1;	
														SAVE_FLAG = 1;	
													}																									
												}		
												*/												
													
												if(setUartFlag == 1)
												{													
													setUartFlag = 0;
													
													char * tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232Baudrate].key);
													int baudrate = 9600;
													if (tmp)
														baudrate = atoi(tmp);

													tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232Databits].key);
													int databits = 8;
													if (tmp)
														databits = atoi(tmp);

													tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232Stopbits].key);
														int stopbits = 1;
													if (tmp)
														stopbits = atoi(tmp);

													tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232Parity].key);
													char parity = 'n';
													if (tmp) 
													{
														if (strcmp(tmp, "none") == 0) 
														{
															parity = 'n';
														} 
														else if (strcmp(tmp, "odd") == 0) 
														{
															parity = 'o';
														} 
														else if (strcmp(tmp, "even") == 0) 
														{
															parity = 'e';
														}
													}
													printf("set serial baudrate:%d %d %d %c\n", baudrate, databits, stopbits, parity);
													int ret = setUart(baudrate, databits, stopbits, parity, 0);
													if (ret != 0) 
													{
														printf(" Set rs232 failed!\n");
													}												
												}
											}
											else
											{
												// send back password not matching
												printf(" \n password not matching \n");
												cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
												cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("PASSWORD-MISMATCH"));
												REQUEST_SUCCESS_FLAG = 0;
											}
										}
										else
										{
											//password missing json returned null
											printf(" \n missing password \n");
											cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
											cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("PASSWORD-MISSING"));
											REQUEST_SUCCESS_FLAG = 0;
										}
									}
								else
								{
									// send back username not matching
									printf(" \n username not matching \n");
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISMATCH"));
									REQUEST_SUCCESS_FLAG = 0;
								}
							}
							else
							{
								//username missing json returned null
								printf(" \n missing username \n");
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISSING"));
								REQUEST_SUCCESS_FLAG = 0;
							}
						}
										
						
						else if (strcmp(received_string, EDID_MANAGE) == 0) 
						{
							cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_cmd].key, cJSON_CreateString(received_string)); // saving command for error reply
							if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_login_ack].key) != NULL) 
							{ // checking if there is a user name
								value_of_key_recieved = getStrFrmObject(json_root_recieved, default_state[msg_login_ack].key);
								correct_value(value_of_key_recieved, received_string); 	// recieved_uname = corrected_value;
								default_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_login].key);
								correct_value(default_value, default_string);														
								if (strcmp(received_string, default_string) == 0 || strcmp(received_string, production_uname) == 0) 
								{ // checking the username										
							
									if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_pwd_msg].key) != NULL) 
									{ // checking if there is a password faild
										value_of_key_recieved = getStrFrmObject(json_root_recieved, default_state[msg_pwd_msg].key);
										correct_value(value_of_key_recieved, received_string);

										default_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_password].key);
										correct_value(default_value, default_string);
										if (strcmp(received_string, default_string) == 0 || strcmp(received_string, production_passwd) == 0) 
										{					
											char* set_edid_str;																					
											char edid_str[514];										
											int edid_ret;										
											edid_ret = -1;											
											if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, "get_edid") != NULL)
											{												
												edid_ret = read_edid_from_file(edid_str);																					
												if (edid_ret == 0) 
												{													
													cJSON_ReplaceItemInObject(json_object_parameter_pool_recieved, "get_edid", cJSON_CreateString(edid_str));
													success_flag = 1;	
												}
												else
												{													
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("READ_FAILED"));
													REQUEST_SUCCESS_FLAG = 0;
												}
											}											
										
											if ((cJSON_GetObjectItem(json_object_parameter_pool_recieved, "set_edid") != NULL) )
											{
												set_edid_str = getStrFrmObject(json_object_parameter_pool_recieved, "set_edid");
												edid_ret =  write_edid_to_file(set_edid_str);
												if (edid_ret == 0) 
												{
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("OK"));												
													REQUEST_SUCCESS_FLAG = 0;																				
												}
												else
												{
													printf("\nset key error\n");
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("WRITE_FAILED")); 
													REQUEST_SUCCESS_FLAG = 0;	
												}											
											}												
										}
										else 
										{
											// send back password not matching
											printf(" \n password not matching \n");
											cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
											cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("PASSWORD-MISMATCH"));
											REQUEST_SUCCESS_FLAG = 0;
										}
									}
									else 
									{
										//password missing json returned null
										printf(" \n missing password \n");
										cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
										cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("PASSWORD-MISSING"));
										REQUEST_SUCCESS_FLAG = 0;
									}
								}
								else 
								{
									// send back username not matching
									printf(" \n username not matching \n");
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISMATCH"));
									REQUEST_SUCCESS_FLAG = 0;
								}
							}
							else 
							{
								//username missing json returned null
								printf(" \n missing username \n");
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISSING"));
								REQUEST_SUCCESS_FLAG = 0;
							}
						}												
						
						else if (strcmp(received_string, SEND_CMD_CEC) == 0) 
						{											
							cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_cmd].key, cJSON_CreateString(received_string)); // saving command for error reply
							if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_login_ack].key) != NULL) 
							{ // checking if there is a user name
								value_of_key_recieved = getStrFrmObject(json_root_recieved, default_state[msg_login_ack].key);
								correct_value(value_of_key_recieved, received_string);
								default_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_login].key);
								correct_value(default_value, default_string);

								if (strcmp(received_string, default_string) == 0 || strcmp(received_string, production_uname) == 0) 
								{ // checking the username
									if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_pwd_msg].key) != NULL) 
									{ // checking if there is a password failed
										value_of_key_recieved = getStrFrmObject(json_root_recieved, default_state[msg_pwd_msg].key);
										correct_value(value_of_key_recieved, received_string);

										default_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_password].key);
										correct_value(default_value, default_string);
										if (strcmp(received_string, default_string) == 0 || strcmp(received_string, production_passwd) == 0) 
										{
											if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, "cmd") != NULL) 
											{
												value_of_key_recieved = getStrFrmObject(json_object_parameter_pool_recieved, "cmd");
												correct_value(value_of_key_recieved, received_string);											
												int cec_ret = 0;
												printf("\nGet cec msg : %s.\n", value_of_key_recieved);
												pthread_mutex_lock(&cec_code_lock);
												if(strcmp(received_string, "STANDBY") == 0)
												{
													g_CEC_Cmd_Data.Cmd_Code = 1;
												}
												else if(strcmp(received_string, "IMAGE_VIEW_ON") == 0)
												{
													g_CEC_Cmd_Data.Cmd_Code = 2;
												}
												else if(strcmp(received_string, "VOLUME_UP") == 0)
												{
													g_CEC_Cmd_Data.Cmd_Code = 3;
												}
												else if(strcmp(received_string, "VOLUME_DOWN") == 0)
												{
													g_CEC_Cmd_Data.Cmd_Code = 4;
												}
												else if(strcmp(received_string, "MUTE") == 0)
												{
													g_CEC_Cmd_Data.Cmd_Code = 5;
												}
												else
												{   
													g_CEC_Cmd_Data.Cmd_Code = 0;
													cec_ret = -1;
												}
												pthread_mutex_unlock(&cec_code_lock);
												
												if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, "src_addr") != NULL) 
												{
													value_of_key_recieved = getStrFrmObject(json_object_parameter_pool_recieved, "src_addr");
													correct_value(value_of_key_recieved, received_string);											
													long int cec_src_addr = 0;
													cec_src_addr = strtol(value_of_key_recieved, NULL, 16);
																									
													if ((cec_src_addr < 0xF) && (cec_src_addr > 0))
													{
														pthread_mutex_lock(&cec_code_lock);
														g_CEC_Cmd_Data.Src_Addr = cec_src_addr & 0xF;
														pthread_mutex_unlock(&cec_code_lock);
														success_flag = 1;												
													}
													else	
														cec_ret = -2;
												}
												
												if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, "dest_addr") != NULL) 
												{
													value_of_key_recieved = getStrFrmObject(json_object_parameter_pool_recieved, "dest_addr");
													correct_value(value_of_key_recieved, received_string);											
													long int cec_dest_addr = 0;
													cec_dest_addr = strtol(value_of_key_recieved, NULL, 16);													
												
													if ((cec_dest_addr <= 0xF) && (cec_dest_addr >= 0))
													{
														pthread_mutex_lock(&cec_code_lock);
														g_CEC_Cmd_Data.Dest_Addr = cec_dest_addr & 0xF;
														pthread_mutex_unlock(&cec_code_lock);
														success_flag = 1;												
													}
													else	
													{	
														cec_ret = -2;
														pthread_mutex_lock(&cec_code_lock);
														g_CEC_Cmd_Data.Dest_Addr = 0x0;
														pthread_mutex_unlock(&cec_code_lock);
													}
												}												
												
												if (cec_ret == 0)																									
													success_flag = 1;												
												else if (cec_ret == -1) 
												{
													printf("Wrong CEC command code\n");
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("Wrong CEC cmd code"));
													REQUEST_SUCCESS_FLAG = 0;
												}
												else if (cec_ret == -2) 
												{
													printf("Wrong CEC src address\n");
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("Wrong CEC src address"));
													REQUEST_SUCCESS_FLAG = 0;
												}	
												else if (cec_ret == -3) 
												{
													printf("Wrong CEC dest address\n");
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("Wrong CEC dest address"));
													REQUEST_SUCCESS_FLAG = 0;
												}												
											}
											else 
											{
												printf("CEC no command\n");
												cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
												cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key,cJSON_CreateString("CEC_NO_COMMAND"));
												REQUEST_SUCCESS_FLAG = 0;
											}											
										}
										else 
										{
											// send back password not matching
											printf(" \n password not matching \n");
											cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
											cJSON_ReplaceItemInObject(json_root_error_reply,default_state[msg_pwd_msg].key,	cJSON_CreateString("PASSWORD-MISMATCH"));
											REQUEST_SUCCESS_FLAG = 0;
										}
									}
									else 
									{
										//password missing json returned null
										printf(" \n missing password \n");
										cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
										cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("PASSWORD-MISSING"));
										REQUEST_SUCCESS_FLAG = 0;
									}
								}
								else 
								{
									// send back username not matching
									printf(" \n username not matching \n");
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISMATCH"));
									REQUEST_SUCCESS_FLAG = 0;
								}
							}
							else 
							{
								//username missing json returned null
								printf(" \n missing username \n");
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISSING"));
								REQUEST_SUCCESS_FLAG = 0;
							}
						}					
									
						else if (strcmp(received_string, SECURITY_CHECK_SHA256) == 0) 
						{
							cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_cmd].key, cJSON_CreateString(received_string)); // saving command for error reply
							if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_login_ack].key) != NULL) 
							{ // checking if there is a user name
								value_of_key_recieved = getStrFrmObject(json_root_recieved, default_state[msg_login_ack].key);
								correct_value(value_of_key_recieved, received_string);
								// recieved_uname = corrected_value;
								default_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_login].key);
								correct_value(default_value, default_string);							
								int security_name_flag, security_pwd_flag;							
								if (strcmp(received_string, default_string) == 0 || strcmp(received_string, production_uname) == 0) 
								{ 
									// checking the username								
									security_name_flag = strcmp(received_string, production_uname);														
									if (cJSON_GetObjectItem(json_root_recieved, default_state[msg_pwd_msg].key) != NULL) 
									{ 
										// checking if there is a password feild
										value_of_key_recieved = getStrFrmObject(json_root_recieved, default_state[msg_pwd_msg].key);
										correct_value(value_of_key_recieved, received_string);

										default_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_password].key);
										correct_value(default_value, default_string);
										if (strcmp(received_string, default_string) == 0 || strcmp(received_string, production_passwd) == 0) 
										{												
											security_pwd_flag = strcmp(received_string, production_passwd);							
											char* set_key_str;										
											int set_key_str_len;																														
											char rom_id[8];
											int security_ret;
										
											if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, "get_id") != NULL)
											{												
												security_ret = SHA256_Read_Rom_Id((unsigned char *)rom_id);						
											
												if (security_ret == 0)
												{													
													char rom_id_str[18];
													HexToStr(rom_id_str, rom_id, 8);
													cJSON_ReplaceItemInObject(json_object_parameter_pool_recieved, "get_id", cJSON_CreateString(rom_id_str));
													success_flag = 1;	
												}
												else
												{
													printf("Read rom id Error!\n");
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
													cJSON_ReplaceItemInObject(json_root_error_reply,
														default_state[msg_pwd_msg].key,
														cJSON_CreateString("READ_FAILED"));
													REQUEST_SUCCESS_FLAG = 0;
												}
											}										
										
											if ((cJSON_GetObjectItem(json_object_parameter_pool_recieved, "set_key") != NULL) )
											{
												value_of_key_recieved = getStrFrmObject(json_object_parameter_pool_recieved, "set_key");
												unsigned char key[32];												
												StrToHex((char *)key, value_of_key_recieved, 32);												
												security_ret = SHA256_Write_Key(key);												
												if(security_ret == 0)
												{
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("OK"));												
													REQUEST_SUCCESS_FLAG = 0;																				
												}
												else
												{
													printf("\nset key error\n");
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
													cJSON_ReplaceItemInObject(json_root_error_reply,
														default_state[msg_pwd_msg].key,
														cJSON_CreateString("WRITE_FAILED"));
													REQUEST_SUCCESS_FLAG = 0;	
												}											
											}			
										
											if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, "get_status") != NULL)
											{																															
												security_ret = SHA256_App_Check();												
												if(security_ret == 0)
												{
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("OK"));
													//cJSON_ReplaceItemInObject(json_root_error_reply,default_state[msg_pwd_msg].key,cJSON_CreateString("FAILED_TO_PASS_VERIFICATION"));																								
													//success_flag = 1;	
													REQUEST_SUCCESS_FLAG = 0;		
												}
												else
												{
													printf(" \n check_SHA_MAC get status error\n");
													cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
													cJSON_ReplaceItemInObject(json_root_error_reply,
														default_state[msg_pwd_msg].key,
														cJSON_CreateString("FAILED_TO_PASS_VERIFICATION"));
													REQUEST_SUCCESS_FLAG = 0;		
												}
											}
										}
										else 
										{
											// send back password not matching
											printf(" \n password not matching \n");
											cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
											cJSON_ReplaceItemInObject(json_root_error_reply,
												default_state[msg_pwd_msg].key,
												cJSON_CreateString("PASSWORD-MISMATCH"));

											REQUEST_SUCCESS_FLAG = 0;
										}
									}
									else 
									{
										//password missing json returned null
										printf(" \n missing password \n");
										cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
										cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("PASSWORD-MISSING"));
										REQUEST_SUCCESS_FLAG = 0;
									}

								}
								else 
								{
									// send back username not matching
									printf(" \n username not matching \n");
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISMATCH"));
									REQUEST_SUCCESS_FLAG = 0;
								}
							}
							else 
							{
								//username missing json returned null
								printf(" \n missing username \n");
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("USERNAME-MISSING"));
								REQUEST_SUCCESS_FLAG = 0;
							}
						}					
						
						else if (strcmp(received_string, SEND_DATA_RS232) == 0) 
						{
							cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_cmd].key, cJSON_CreateString(received_string)); // saving command for error reply
							if (cJSON_GetObjectItem(json_object_parameter_pool_recieved,"hexdata") != NULL) 
							{
								value_of_key_recieved = getStrFrmObject(json_object_parameter_pool_recieved,"hexdata");
								correct_value(value_of_key_recieved, received_string);
								int timeout = 0;
								char* isFeedback = getStrFrmObject(json_object_parameter_pool_recieved,	"isFeedback");
								if (isFeedback && strcmp(isFeedback, "1") == 0) 
								{
									timeout = atoi(getStrFrmObject(json_object_parameter_pool_recieved, "waitTime"));
								}

								char reply[512 + 2];	//max 256 num, * 2 for each num = 512
								memset(reply, 0, sizeof(reply));

								rs232PauseFlag = 1;
								usleep(10000);	// wait for rs232 passthrough paused
								int ret = sendtoUart(timeout, value_of_key_recieved, reply);
								if (ret == 0) 
								{
									printf("RS232 Received: %s\n", reply);
									if (isFeedback && strcmp(isFeedback, "1") == 0) 
									{
										cJSON_AddItemToObject(json_object_parameter_pool_recieved, "feedbackdata", cJSON_CreateString(reply));
									}
									else
										cJSON_AddItemToObject(json_object_parameter_pool_recieved, "feedbackdata", cJSON_CreateString(""));
									
									cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, "hexdata");
									cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, "isFeedback");
									cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, "waitTime");
									success_flag = 1;
								} 
								else if (ret == -1) 
								{
									printf(" \n send to rs232 error\n");
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("SEND_TO_RS232_ERROR"));
									REQUEST_SUCCESS_FLAG = 0;
								} 
								else if (ret == -2) 
								{
									printf(" \n receive from rs232 error\n");
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("RECEIVE_FROM_RS232_ERROR"));
									REQUEST_SUCCESS_FLAG = 0;
								}
								rs232PauseFlag = 0;
							} 
							else 
							{
								printf(" \n no hexdata \n");
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("NO_HEXDATA"));
								REQUEST_SUCCESS_FLAG = 0;
							}
						} 
#ifdef IR_PASSTHROUGH_ENABLE	
						else if (strcmp(received_string, SEND_DATA_IR) == 0) 
						{
							cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_cmd].key, cJSON_CreateString(received_string)); // saving command for error reply
							if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, "hexdata") != NULL) 
							{
								value_of_key_recieved = getStrFrmObject(json_object_parameter_pool_recieved, "hexdata");
								correct_value(value_of_key_recieved, received_string);
								int timeout = 0;
								char* isFeedback = getStrFrmObject(json_object_parameter_pool_recieved, "isFeedback");
								if (isFeedback && strcmp(isFeedback, "1") == 0) 
								{
									timeout = atoi(getStrFrmObject(json_object_parameter_pool_recieved, "waitTime"));
								}
								char reply[1600];	//max number for one command is 200, type for each number is 4, so 200 * 4
								unsigned short ir_data[200];								
								unsigned char fpga_data[10];
								unsigned char carrier_freq;
								unsigned char ir_ctrl_data;
								int length;
								int ir_ret;
								unsigned int ir_mode;
								memset(reply, 0, sizeof(reply));								
								irPauseFlag = 1;
								printf("Send_data_IR Pause irPassThrough \n");
								usleep(500000);	// wait for ir passthrough paused
								if (timeout == 0) 
								{						
									ir_mode = ReadIRMode();
									if(ir_mode != 1)
										ir_ret = -4;
									else
									{
										//emitter mode, set carrier frequency in FPGA
										length = strlen(value_of_key_recieved) / 2;
										if (length > SYMBOL_CNT * 2)
											length = SYMBOL_CNT * 2;
										StrToHex((unsigned char*)ir_data, value_of_key_recieved, length);
										if (ir_data[0] != IR_MAGIC_HEADER)
											ir_ret = -1;
										else
										{
											carrier_freq = ir_data[1] & 0x0F;
											//set chip IR to idle mode
											setChipIRMode("0");
											//set IR emitter mode in FPGA
											Write_FPGA_IR_Emitter_Freqency(carrier_freq);																														
											//set chip IR to emitter mode
											setChipIRMode("2");										
											ir_ret = controlIR_rawdata(timeout, ir_data);																				
										}									
									}
								}		
								else if(timeout > 0)			
								{																			
									ir_mode = ReadIRMode();
									if(ir_mode != 0)
										ir_ret = -4;
									else
									{	
										//read data from IR driver
										ir_ret = controlIR_rawdata(timeout, ir_data);											
										carrier_freq = Read_FPGA_IR_Emitter_Freqency();											
										switch (carrier_freq)
										{
											case 0x0:
												printf("carrier :  30K\n");
											break;				
											case 0x1:
												printf("carrier :  33K\n");
											break;				
											case 0x2:
												printf("carrier :  36K\n");
											break;
											case 0x3:
												printf("carrier :  38K\n");
											break;				
											case 0x4:
												printf("carrier :  40K\n");				
											break;				
											case 0x5:
												printf("carrier :  56K\n");				
											break;			
											default:
												printf("carrier unknown\n");																	
										}									
									}
									ir_data[1] = carrier_freq & 0x0F;																
									HexToStr(reply, (char *) ir_data, ir_data[2]*2);									
								}
								else
									ir_ret = -5;
									
								printf("Send_data_IR release irPassThrough \n");
								irPauseFlag = 0;
								
								if (ir_ret == 0) 
								{									
									if (isFeedback && strcmp(isFeedback, "1") == 0)
										cJSON_AddItemToObject(json_object_parameter_pool_recieved, "feedbackdata", cJSON_CreateString(reply));
									else
										cJSON_AddItemToObject(json_object_parameter_pool_recieved, "feedbackdata", cJSON_CreateString(""));
									
									cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, "hexdata");
									cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, "isFeedback");
									cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, "waitTime");

									success_flag = 1;
								} 
								else if (ir_ret == -1) 
								{
									printf(" \n sendtoIR error\n");

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("SEND_TO_IR_ERROR"));

									REQUEST_SUCCESS_FLAG = 0;
								} 
								else if (ir_ret == -2) 
								{
									printf(" \n receive from IR error\n");

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("RECEIVE_FROM_IR_ERROR"));

									REQUEST_SUCCESS_FLAG = 0;
								} 
								else if (ir_ret == -3) 
								{
									printf(" \n sendtoIR error\n");

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("SOCKET_ERROR"));

									REQUEST_SUCCESS_FLAG = 0;
								}
								else if(ir_ret == -4)
								{
									printf(" \n Receive from IR error\n");

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("IR_MODE_ERROR"));

									REQUEST_SUCCESS_FLAG = 0;
								}
								else 
								{
									printf("IR command parameter error\n");

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));

									cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("IR_PARAMETER_ERROR"));

									REQUEST_SUCCESS_FLAG = 0;
								}														
								
							} 
							else
							{
								printf("No hexdata\n");

								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));

								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("NO_HEXDATA"));

								REQUEST_SUCCESS_FLAG = 0;
							}
						}							
#endif						
						else if (strcmp(received_string, SET_CMD) == 0) 
						{
							cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_cmd].key, cJSON_CreateString(received_string)); // saving command for error reply
							
							if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, "blink_led") != NULL) 
							{	
								value_of_key_recieved = getStrFrmObject(json_object_parameter_pool_recieved, "blink_led");
								correct_value(value_of_key_recieved, received_string);

								if (strcmp(received_string, "1") == 0) 
								{
									power_led_mode = LED_MODE_BLINK;	
								} 
								else if (strcmp(received_string, "0") == 0) 
								{
									power_led_mode = LED_MODE_ON;	
								}
								
								success_flag = 1;
								cJSON_ReplaceItemInObject(json_object_parameter_pool_recieved, "blink_led", cJSON_CreateString(received_string));
							}			
						}						
						
						//////////////////////////////////////////////
						else
						{
							//invalid command
							value_of_key_recieved = cJSON_GetObjectItem(json_root_recieved, default_state[msg_cmd].key)->valuestring;
							correct_value(value_of_key_recieved, received_string);

							if (strcmp(received_string, GET_PARAM) != 0 && strcmp(received_string, SET_PARAM) != 0)
							{
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
								cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("INCORRECT-COMMAND"));
								REQUEST_SUCCESS_FLAG = 0;
							}
						}
					}
					else
					{
						//command missing json returned null
						cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));
						cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("COMMAND-MISSING"));
						REQUEST_SUCCESS_FLAG = 0;
					}
				}
			else
			{
				//Invalid mac address
				DO_NOTHING_FLAG = 1; 
			}
		}
		
		if (prod_error_flag == 1)
		{
			/// set the ack and success message in reply, but should say some feild cannot be changed
			cJSON_ReplaceItemInObject(json_root_recieved, default_state[msg_pwd_msg].key, cJSON_CreateString(error5));
			cJSON_ReplaceItemInObject(json_root_recieved, default_state[msg_login_ack].key, cJSON_CreateString(Not_Acknwoldge));
		}
		else
		{
			cJSON_ReplaceItemInObject(json_root_recieved, default_state[msg_pwd_msg].key, cJSON_CreateString(success1));
			cJSON_ReplaceItemInObject(json_root_recieved, default_state[msg_login_ack].key, cJSON_CreateString(Acknwoldge));

		}
		
		if (success_flag == 1)
		{
			REQUEST_SUCCESS_FLAG = 1;
			int j;
			for (j = msg_login; j < msg_num; j++)
			{
				if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[j].key) != NULL)
				{
					if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[j].key) != NULL)
					{
						default_value = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[j].key)->valuestring;				 
																										 
																																											 
						cJSON_ReplaceItemInObject(json_object_parameter_pool_recieved, default_state[j].key, cJSON_CreateString(default_value));	 
					}
				}
			}
			if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[msg_login].key) != NULL)
			{
				cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, default_state[msg_login].key);
			}

			if (cJSON_GetObjectItem(json_object_parameter_pool_recieved, default_state[msg_password].key) != NULL)
			{
				cJSON_DeleteItemFromObject(json_object_parameter_pool_recieved, default_state[msg_password].key);
			}
		}

		char * msg;
		if (REQUEST_SUCCESS_FLAG == 0)
			msg = cJSON_Print(json_root_error_reply);
		else
			msg = cJSON_Print(json_root_recieved);

		if (json_root_error_reply)
			cJSON_Delete(json_root_error_reply);
		if (json_root_recieved)
			cJSON_Delete(json_root_recieved);		

		if (SAVE_FLAG == 1)
		{
			char * tmp = cJSON_Print(json_root_default);			
			saveParamToDrive(CONFIG_INI, tmp, strlen(tmp));			
			//printf("\nThe updated parameters are: \n\n%s\n", tmp);
			if (tmp)
				free(tmp);
		}
		
		return msg;
	}
	else
	{

		char received_string[256];
		char default_string[256];

		default_value = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_mac].key)->valuestring;  /// this is the mac address of the system
		correct_value(default_value, default_string);
		cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_targ].key, cJSON_CreateString(default_string));

		cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_login_ack].key, cJSON_CreateString("NOK"));

		cJSON_ReplaceItemInObject(json_root_error_reply, default_state[msg_pwd_msg].key, cJSON_CreateString("CORRUPTED-JSON-DATA-RECEIVED"));
		char * msg;
		msg = cJSON_Print(json_root_error_reply);

		if (json_root_error_reply)
			cJSON_Delete(json_root_error_reply);

		return msg;
	}
}

char *correct_value(char *error_value, char *original_value) {
	int length_of_value, i, j;
	//char *temp_value;

	length_of_value = strlen(error_value);

	memset(original_value, 0, length_of_value + 2);

	j = 0;

	for (i = 0; i < length_of_value; i++) {
		if (error_value[i] != '"') {

			original_value[j] = error_value[i];

			j = j + 1;
		}
	}
	//original_value = temp_value;

	return original_value;
}

int build_reply(char * reply, char *json_message) {

	unsigned int CRC_Value;
	unsigned char rep_message[MAX_MSG_SIZE];

	int length = clean_json(json_message, rep_message);

	//int n, i, j;
	//int length;
	//length = strlen(rep_message);

	reply[0] = 0xFB;
	reply[1] = 0xA0;
	reply[2] = ((length + 4) >> 8) & 0xFF;
	reply[3] = ((length + 4) & 0xFF);

	CRC_Value = crc32b(rep_message);

	int i;
	for (i = 0; i < length; i++) {
		reply[i + 4] = rep_message[i];
	}

	reply[i + 4] = (CRC_Value >> 24) & 0xFF;
	reply[i + 5] = (CRC_Value >> 16) & 0xFF;
	reply[i + 6] = (CRC_Value >> 8) & 0xFF;
	reply[i + 7] = (CRC_Value) & 0xFF;

	return (i + 8);
}

int clean_json(char *input_JSON, unsigned char *output) {
	int length_of_value, i, j;
	//char *temp_value;

	length_of_value = strlen(input_JSON);

	memset(output, 0, length_of_value + 2);

	j = 0;

	for (i = 0; i < length_of_value; i++) {
		if (input_JSON[i] != '\t' && input_JSON[i] != '\n') {

			output[j] = input_JSON[i];

			j = j + 1;
		}
	}
	//original_value = temp_value;

	return j;
}

int get_hardware_info(struct hardware_info *hard_info)
{
	unsigned char fpga_buff[60];	
	unsigned int addr;
	unsigned char  prtad;
	unsigned char  devad;
	unsigned int buff;
	unsigned char length;
	unsigned char data[4];
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
	unsigned int temp;
#ifdef MDIO_DRV_ENABLE
	prtad = 0x00;
	devad = 0x1E;
	addr = 0xC820;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &aqr_temp);
	
	if (aqr_temp >= 0x8000)
	{
		aqr_temp = ((((~aqr_temp & 0xFFFF) + 1) / 256) * -1);
	}
	else
	{
		aqr_temp = aqr_temp / 256;
	}
	hard_info->aqr_temp = aqr_temp;
#endif // MDIO_DRV_ENABLE	
	//get FPGA temperature
	data[0] = 0x0;
	data[1] = 0x0;
	data[2] = 0x40;
	data[3] = 0x03;
	I2C_Write(0x6C, 0x0, data, 4);
	
	devad = 0x6C;
	addr = 0x02;
	data[0] = 0x00;
	data[1] = 0x01;
	length = 4;
	I2C_Write(0x6C, 0x02, data, length);
	
	addr = 0x04;
	length = 3;
	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	I2C_Read(0x6C, 0x04, data, length);
	fpga_temp = (int)(((data[1] << 4 | (data[0] >> 4)) * 0.123) - 273.15);
	hard_info->fpga_temp = fpga_temp;	
#ifdef MDIO_DRV_ENABLE
	devad = 0x01;
	addr = 0x85;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->opr_margin_chnl_A = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x86;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	opr_margin_chnl_B = ((buff - 0x8000) * (float) 0.1);
	hard_info->opr_margin_chnl_B = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x87;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	opr_margin_chnl_C = ((buff - 0x8000) * (float) 0.1);
	hard_info->opr_margin_chnl_C = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x88;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->opr_margin_chnl_D = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x89;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->min_opr_margin_chnl_A = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x8A;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->min_opr_margin_chnl_B = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x8B;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->min_opr_margin_chnl_C = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x8C;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->min_opr_margin_chnl_D = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x8D;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->rx_power_ch_A = ((buff - 0x8000) * (float) 0.1);
	
	devad = 0x01;
	addr = 0x8E;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->rx_power_ch_B = ((buff - 0x8000) * (float) 0.1);

	devad = 0x01;
	addr = 0x8F;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->rx_power_ch_C = ((buff - 0x8000) * (float) 0.1);

	devad = 0x01;
	addr = 0x90;
	mdio_write_addr(prtad, devad, addr);
	mdio_read_data(prtad, devad, &buff);
	hard_info->rx_power_ch_D = ((buff - 0x8000) * (float) 0.1);	
#endif // MDIO_DRV_ENABLE	
	
	devad = 0x60;
	addr = 0x00;
	length = 48;
	memset(fpga_buff, 0x00, sizeof(fpga_buff));
	I2C_Read(devad, addr, fpga_buff, length);
	
	hard_info->reset_control = fpga_buff[0];
	hard_info->video_config = fpga_buff[1];
	hard_info->color_space = (fpga_buff[1] >> 2) & 0x07;
	hard_info->color_depth = fpga_buff[1] & 0x03;
	hard_info->ir_control = fpga_buff[2]; 
	hard_info->stream_en =  fpga_buff[3]; 
	hard_info->clock_status = fpga_buff[4]; 
	hard_info->ir_modulation = fpga_buff[5] & 0xf;
	temp = fpga_buff[7];
	temp = (temp << 8) + fpga_buff[6];
	hard_info->x_active = temp;
	temp = fpga_buff[9];
	temp = (temp << 8) + fpga_buff[8];
	hard_info->y_active = temp;
	hard_info->x_front_porch = fpga_buff[10] + 256*(fpga_buff[11] & 0x7);
	hard_info->x_pulse_count = (fpga_buff[11] >> 3) + 256*(fpga_buff[12] & 0x1F);
	hard_info->x_back_porch = (fpga_buff[12] >> 5) + 256*fpga_buff[13];
	hard_info->y_front_porch = fpga_buff[14] + 256*(fpga_buff[15] & 0x7);
	hard_info->y_pulse_count = (fpga_buff[15] >> 3) + 256*(fpga_buff[16] & 0x1F);
	hard_info->y_back_porch = (fpga_buff[16] >> 5) + 256*fpga_buff[17];
	temp = fpga_buff[20];
	temp = (temp << 8) + fpga_buff[19];
	temp = (temp << 8) + fpga_buff[18];
	hard_info->pclk_freq = temp*1000;
	hard_info->audio_ch_sel = fpga_buff[26];
	hard_info->audio_ch_mask = fpga_buff[27];
	hard_info->fpga_version = fpga_buff[29];	
	hard_info->fpga_revision = fpga_buff[30];	
	hard_info->fpga_model = fpga_buff[31];
	hard_info->packet_cnt_rx = fpga_buff[34] + fpga_buff[35]*256;
	hard_info->packet_cnt_tx = fpga_buff[36] + fpga_buff[37]*256;
	temp = fpga_buff[43];
	temp = (temp << 8) + fpga_buff[42];
	temp = (temp << 8) + fpga_buff[41];
	temp = (temp << 8) + fpga_buff[40];
	hard_info->delta_pixel_cnt = temp;
	temp = fpga_buff[47];
	temp = (temp << 8) + fpga_buff[46];
	temp = (temp << 8) + fpga_buff[45];
	temp = (temp << 8) + fpga_buff[44];
	hard_info->prev_delta_pxixel_cnt = temp;
	if(0x0C == fpga_buff[1]) 
		hard_info->raw_mode = 1;
	else 
		hard_info->raw_mode = 0;

	memset(fpga_buff, 0x00, sizeof(fpga_buff));	
	I2C_Read(0x68, 0x00, fpga_buff, 45);
	hard_info->mac_control_0 = fpga_buff[0];
	hard_info->mac_control_1 = fpga_buff[1];
	hard_info->mac_control_2 = fpga_buff[2];	
	hard_info->mac_control_3 = fpga_buff[3];	
	hard_info->mac_local[5] = fpga_buff[4];	
	hard_info->mac_local[4] = fpga_buff[5];	
	hard_info->mac_local[3] = fpga_buff[6];	
	hard_info->mac_local[2] = fpga_buff[7];	
	hard_info->mac_local[1] = fpga_buff[8];	
	hard_info->mac_local[0] = fpga_buff[9];	
	hard_info->mac_dest[5] = fpga_buff[10];	
	hard_info->mac_dest[4] = fpga_buff[11];	
	hard_info->mac_dest[3] = fpga_buff[12];	
	hard_info->mac_dest[2] = fpga_buff[13];	
	hard_info->mac_dest[1] = fpga_buff[14];	
	hard_info->mac_dest[0] = fpga_buff[15];	
	hard_info->ip_addr_local[3] = fpga_buff[16];	
	hard_info->ip_addr_local[2] = fpga_buff[17];	
	hard_info->ip_addr_local[1] = fpga_buff[18];	
	hard_info->ip_addr_local[0] = fpga_buff[19];	
	hard_info->ip_addr_dest_v[3] = fpga_buff[20];	
	hard_info->ip_addr_dest_v[2] = fpga_buff[21];	
	hard_info->ip_addr_dest_v[1] = fpga_buff[22];	
	hard_info->ip_addr_dest_v[0] = fpga_buff[23];	
	hard_info->ip_addr_dest_a[3] = fpga_buff[24];	 
	hard_info->ip_addr_dest_a[2] = fpga_buff[25];	
	hard_info->ip_addr_dest_a[1] = fpga_buff[26];	
	hard_info->ip_addr_dest_a[0] = fpga_buff[27];		 		
	return 0;	
}

int save_fpga_log(struct hardware_info *hard_info)
{
	FILE *fp;
	fp = fopen(FPGA_LOG_FILE, "w");
	if(NULL == fp)
	{
		printf("FPGA log file open error!");
		return -1;
	}
	fprintf(fp, " Aquantia  : %d\n", hard_info->aqr_temp);
	fprintf(fp, " FPGA : %d\n", hard_info->fpga_temp);
	fprintf(fp, " SNR_ch_A : %3.2f\n", hard_info-> opr_margin_chnl_A);
	fprintf(fp, " SNR_ch_B : %3.2f\n", hard_info-> opr_margin_chnl_B);
	fprintf(fp, " SNR_ch_C : %3.2f\n", hard_info-> opr_margin_chnl_C);
	fprintf(fp, " SNR_ch_D : %3.2f\n", hard_info-> opr_margin_chnl_D);	
	fprintf(fp, " SNR_Min_ch_A : %3.2f\n", hard_info-> min_opr_margin_chnl_A);	
	fprintf(fp, " SNR_Min_ch_B : %3.2f\n", hard_info-> min_opr_margin_chnl_A);	
	fprintf(fp, " SNR_Min_ch_C : %3.2f\n", hard_info-> min_opr_margin_chnl_A);	
	fprintf(fp, " SNR_Min_ch_D : %3.2f\n", hard_info-> min_opr_margin_chnl_A);	
	fprintf(fp, " RX_power_ch_A : %3.2f\n", hard_info-> rx_power_ch_A);	
	fprintf(fp, " RX_power_ch_B : %3.2f\n", hard_info-> rx_power_ch_B);	
	fprintf(fp, " RX_power_ch_C : %3.2f\n", hard_info-> rx_power_ch_C);	
	fprintf(fp, " RX_power_ch_D : %3.2f\n", hard_info-> rx_power_ch_D);	
	fprintf(fp, " FPGA reset control : 0x%x\n", hard_info-> reset_control);		
	fprintf(fp, " FPGA video_config : 0x%x\n", hard_info-> video_config);
	if(hard_info->ir_control & 0x10) //bit 4
	{			
		fprintf(fp, " FPGA IR emitter mode.\n");
		switch((hard_info->ir_control) & 0xF)
		{
			case 0: fprintf(fp, " IR carrier : 30KHz\n"); break;
			case 1: fprintf(fp, " IR carrier : 33KHz\n"); break;
			case 2: fprintf(fp, " IR carrier : 36KHz\n"); break;
			case 3: fprintf(fp, " IR carrier : 38KHz\n"); break;
			case 4: fprintf(fp, " IR carrier : 40KHz\n"); break;			
			case 5: fprintf(fp, " IR carrier : 56KHz\n"); break;			
			default : fprintf(fp, " IR carrier : unknown\n"); break;						
		}
	}
	else
	{	
		if(hard_info-> ir_control & 0x20) //bit 5
			fprintf(fp, " FPGA IR sensor mode, demodulator enable.\n");
		else
			fprintf(fp, " FPGA IR sensor mode, demodulator disabled.\n");
		switch(hard_info->ir_modulation)
		{
			case 0: fprintf(fp, " IR sensor carrier : 30KHz\n"); break;
			case 1: fprintf(fp, " IR sensor carrier : 33KHz\n"); break;
			case 2: fprintf(fp, " IR sensor carrier : 36KHz\n"); break;
			case 3: fprintf(fp, " IR sensor carrier : 38KHz\n"); break;
			case 4: fprintf(fp, " IR sensor carrier : 40KHz\n"); break;			
			case 5: fprintf(fp, " IR sensor carrier : 56KHz\n"); break;			
			default : fprintf(fp, " IR sensor carrier : unknown\n"); break;						
		}
	}
	
	fprintf(fp, " FPGA ir_control : 0x%x\n", hard_info-> ir_control);			
	if((hard_info-> stream_en) & 0x1)
		fprintf(fp, " Video stream enable\n");		
	else
		fprintf(fp, " Video stream disabled\n");		
	if((hard_info-> stream_en) & 0x2)
		fprintf(fp, " Audio stream enable\n");		
	else
		fprintf(fp, " Audio stream disabled\n");			
	
	fprintf(fp, " FPGA clock_status : 0x%x\n", hard_info-> clock_status);				
	fprintf(fp, " Video X active : %d\n", hard_info-> x_active);					
	fprintf(fp, " Video Y active : %d\n", hard_info-> y_active);						
	fprintf(fp, " Video X_front_porch : %d\n", hard_info-> x_front_porch);						
	fprintf(fp, " Video X_pulse_count : %d\n", hard_info-> x_pulse_count);						
	fprintf(fp, " Video X_back_porch : %d\n", hard_info-> x_back_porch);						
	fprintf(fp, " Video Y_front_porch : %d\n", hard_info-> y_front_porch);						
	fprintf(fp, " Video Y_pulse_count : %d\n", hard_info-> y_pulse_count);						
	fprintf(fp, " Video Y_back_porch : %d\n", hard_info-> y_back_porch);							
	fprintf(fp, " Video pclk_freq : %dHz\n", hard_info-> pclk_freq);
	fprintf(fp, " audio_ch_sel : 0x%x\n", hard_info-> audio_ch_sel);
	fprintf(fp, " audio_ch_mask : 0x%x\n", hard_info-> audio_ch_mask);
	if(hard_info-> fpga_version & 0x80)		
		fprintf(fp, " FPGA version : TX 0x%x\n", hard_info-> fpga_version);
	else
		fprintf(fp, " FPGA version : RX 0x%x\n", hard_info-> fpga_version);
	fprintf(fp, " FPGA revision : 0x%x\n", hard_info-> fpga_revision);
	fprintf(fp, " FPGA model : 0x%x\n", hard_info-> fpga_model);	
	fprintf(fp, " packet_cnt_rx : %d\n", hard_info-> packet_cnt_rx);	
	fprintf(fp, " packet_cnt_tx : %d\n", hard_info-> packet_cnt_tx);
	fprintf(fp, " delta_pixel_cnt : 0x%x\n", hard_info-> delta_pixel_cnt);
	fprintf(fp, " prev_delta_pxixel_cnt : %d\n", hard_info-> prev_delta_pxixel_cnt);
	fprintf(fp, " raw_mode : %d\n", hard_info-> raw_mode);
	switch (hard_info->color_space)
	{
		case 0: fprintf(fp, " Color space : %s\n", "YCbCr444"); break;		
		case 1: fprintf(fp, " Color space : %s\n", "YCbCr422"); break;
		case 2: fprintf(fp, " Color space : %s\n", "x420"); break;
		case 3: fprintf(fp, " Color space : %s\n", "x_raw"); break;
		case 4: fprintf(fp, " Color space : %s\n", "RGB"); break;
	}	
	switch (hard_info->color_depth)
	{
		case 0: fprintf(fp, " Color depth : %s\n", "8 bit"); break;	 		
		case 1: fprintf(fp, " Color depth : %s\n", "10 bit"); break;	 
		case 2: fprintf(fp, " Color depth : %s\n", "12 bit"); break;	 
		case 3: fprintf(fp, " Color depth : %s\n", "16 bit"); break;	 	
	}	
	fprintf(fp, " MAC control 0 : 0x%x\n", hard_info->mac_control_0); 	
	fprintf(fp, " MAC control 1 : 0x%x\n", hard_info->mac_control_1); 		
	fprintf(fp, " FPGA local MAC : %02x:%02x:%02x:%02x:%02x:%02x\n", hard_info->mac_local[0], hard_info->mac_local[1], hard_info->mac_local[2],
				hard_info->mac_local[3], hard_info->mac_local[4], hard_info->mac_local[5]); 		
	fprintf(fp, " FPGA dest MAC : %02x:%02x:%02x:%02x:%02x:%02x\n", hard_info->mac_dest[0], hard_info->mac_dest[1], hard_info->mac_dest[2],
				hard_info->mac_dest[3], hard_info->mac_dest[4], hard_info->mac_dest[5]); 			
	fprintf(fp, " FPGA local IP: %d.%d.%d.%d\n", hard_info->ip_addr_local[0], hard_info->ip_addr_local[1], 
				hard_info->ip_addr_local[2], hard_info->ip_addr_local[3]);
	fprintf(fp, " FPGA video dest IP: %d.%d.%d.%d\n", hard_info->ip_addr_dest_v[0], hard_info->ip_addr_dest_v[1], 
				hard_info->ip_addr_dest_v[2], hard_info->ip_addr_dest_v[3]);				
	fprintf(fp, " FPGA audio dest IP: %d.%d.%d.%d\n", hard_info->ip_addr_dest_a[0], hard_info->ip_addr_dest_a[1], 
				hard_info->ip_addr_dest_a[2], hard_info->ip_addr_dest_a[3]);								
				
	fclose(fp);	
	return 0;
}

int save_edid_log()
{
	FILE *fpedid, *fplog;
	unsigned char edid[256];
	int i, cnt;	
	fplog = fopen(EDID_LOG_FILE, "w");
	if(NULL == fplog)
	{
		printf("EDID log file open error!");			
		return -1;
	}		
	else
	{
		fpedid = fopen(EDID_DEFAULT, "rb");	
		if(NULL == fpedid)
		{
			printf("EDID bin file open error!");
			fprintf(fplog," TX edid read error!\n");
			fclose(fplog);
			return -1;
		}
		else
		{
			cnt = fread(edid, 1, 256, fpedid);
			fprintf(fplog, " TX edid is : \n");	
			for(i = 0; i < cnt; i++)
			{
				fprintf(fplog, " %02x", edid[i]);
				if(((i + 1)%8) == 0)
					fprintf(fplog, "\n");		
			}
			fclose(fplog);
			fclose(fpedid);	
			return 0;
		}		
	}	
}		

int write_request(int key_ref, unsigned char *value_requested)
{
	bool connect_flag = 1;
	char value_received[15];
	
	//pthread_mutex_lock(&i2c_bus_lock);
	
	if (key_ref == msg_login)
	{
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref == msg_password) 
	{
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref == msg_pName)
	{
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref == msg_model)
	{
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref == msg_fwVer)
	{
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref == msg_cName) 
	{
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref == msg_mac) 
	{			
		return 0;
	}
	else if (key_ref == msg_ip) 
	{
		setip(value_requested);	
		getip(value_received, GET_IP);
		if (strcmp(value_requested, value_received) == 0)
		{
			printf("\n IP set successfully");
			return 0;
		}
		//else
		//{
		//	return 1;		
		//}		
	}
	else if (key_ref ==  msg_mask) 
	{
		return 0;
	}
	else if (key_ref ==  msg_gateway) 
	{
		return 0;
	}
	else if (key_ref ==  msg_isDhcp) 
	{			
		isRebootOn = 1;	//do udhcpc command after reboot at json_value_init.
		return 0;
	}
	else if (key_ref ==  msg_isDipOn) 
	{		
		return 0;
	}
	else if (key_ref ==  msg_isReset) 
	{
		if (strcmp(value_requested, "1") == 0) 
		{			
			isResetOn = 1;
		}
		return 0;
	}	
	else if (key_ref ==  msg_isReboot) 
	{
		if (strcmp(value_requested, "1") == 0) 
			isRebootOn = 1;		
		return 0;
	}
#ifdef IR_PASSTHROUGH_ENABLE	
	else if (key_ref ==  msg_IRState)                                                    
	{
		irPauseFlag = 1;
		printf("Write request set irPauseFlag = 1\n");
		usleep(800000);	// wait for ir passthrough paused
		if(strcmp(value_requested, "1") == 0)		
		{	//set to idle mode first 
			setChipIRMode("0");			
			Set_FPGA_IR_Mode(1);			
			//set to sensor mode
			setChipIRMode("1");
		}
		else if(strcmp(value_requested, "2") == 0)
		{	//set to idle mode first 
			setChipIRMode("0");			
			Set_FPGA_IR_Mode(2);			
			//set to emitter mode, default 38KHz			
			Write_FPGA_IR_Emitter_Freqency(0x3);
			setChipIRMode("2");
		}
		else //idle mode
		{			
			setChipIRMode("0");
			printf("json value init--IR_STATE parameter error! %s \n", value_requested);
		}
		irPauseFlag = 0;		
		printf("Write request set irPauseFlag = 0\n");
		return 0; 
	}
	else if (key_ref ==  msg_IRFeedbackIp)
	{
		struct sockaddr_in addr4;
		if (inet_pton(AF_INET, value_requested, &addr4.sin_addr) > 0) 		
		{
			irPauseFlag = 1;
			printf("Write request set irPauseFlag = 1\n");
			usleep(500000);	// wait for ir passthrough paused
			strncpy(IR_REMOTE_IP, value_requested, 15);
			irPauseFlag = 0;
			printf("Write request set irPauseFlag = 0\n");
			return 0;
		}
		else
			return -1;				
	}
#endif
	else if (key_ref ==  msg_RS232Baudrate) 
	{
		setUartFlag = 1;
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref ==  msg_RS232Databits)
	{
		setUartFlag = 1;
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref ==  msg_RS232Parity)
	{
		setUartFlag = 1;
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref ==  msg_RS232Stopbits)
	{
		setUartFlag = 1;
		return 0; //let be changed on json root default and saved
	}
	else if (key_ref ==  msg_RS232FeedbackIp)
	{		
		strncpy(RS232_REMOTE_IP, value_requested, 15);
		return 0; 
	}
	else if(key_ref ==  msg_isConnect)
	{		
		if (1 == atoi(value_requested))
		{			
			connection_enable_flag = 1;			
		}
		else
		{			
			pthread_mutex_lock(&i2c_bus_lock);
			value_received[0] =0x00;
			I2C_Write(0x60, 0x03, value_received, 1);
			pthread_mutex_unlock(&i2c_bus_lock);
			connection_enable_flag = 0;			
		}		
		printf("***write_request--msg_isConnect\n");
		return 0; //let be changed on json root default and saved
	}		
	else if(key_ref ==  msg_VideoPortDest)
	{
	//	word ret;
	//	byte deviceId = 0x68;
	//	byte offset = 34;
	//	byte length = 2;
	//
	//	pthread_mutex_lock(&i2c_bus_lock);
	//
	//	memset(value_received, 0, sizeof(value_received));
	//	ret = strtol(value_requested, &value_received, 10);
	//	value_received[0] = ret & 0xFF;
	//	value_received[1] = ((ret & 0xFF00) >> 8);
	//	I2C_Write(deviceId, offset, value_received, length);
	//
	//	pthread_mutex_unlock(&i2c_bus_lock);
		

		return 0; //let be changed on json root default and saved
	}
	else if(key_ref ==  msg_VideoPortLocal)
	{
	//	word ret;
	//	byte deviceId = 0x68;
	//	byte offset = 32;
	//	byte length = 2;
	//
	//	pthread_mutex_lock(&i2c_bus_lock);
	//
	//	memset(value_received, 0, sizeof(value_received));
	//	ret = strtol(value_requested, &value_received, 10);
	//	value_received[0] = ret & 0xFF;
	//	value_received[1] = ((ret & 0xFF00) >> 8);
	//	I2C_Write(deviceId, offset, value_received, length);
	//	pthread_mutex_unlock(&i2c_bus_lock);
		return 0; //let be changed on json root default and saved
	}	
	else if (key_ref ==  msg_isMCAST) 
	{
		return 0;
	}
	else if (key_ref ==  msg_MCASTip) 
	{
		word ret;
		byte deviceId = 0x68;
		byte offset = 10;
		byte length = 6;	

		// copy to global variable
		strncpy(g_multicast_ip, value_requested, 15);
		
		pthread_mutex_lock(&i2c_bus_lock);		
		//disable FPGA video & audio stream
		memset(value_received, 0, sizeof(value_received));
		I2C_Read(0x60, 0x03, value_received, 1);
		value_received[0] = value_received[0] & 0xFE;
		I2C_Write(0x60, 0x03, value_received, 1);
		
		deviceId = 0x68;
		offset = 10;
		length = 6;
		memset(value_received, 0, sizeof(value_received));		

		// ip ("ip0.ip1.ip2.ip3") to mac ("01:00:5e:ip1:ip2:ip3"), ip3 byte first
		parse_ip_to_mcast_mac(value_requested, value_received);
		I2C_Write(deviceId, offset, value_received, length);		
		memset(value_received, 0, sizeof(value_received));		
		
		//Do not enable FPGA stream now. Enable FPGA stream in ADV_routine when it detects a valid HDMI signal
		//I2C_Read(0x60, 0x03, value_received, 1);
		//value_received[0] = value_received[0] | 0x01;
		//value_received[0] = value_received[0] | 0x03;
		//I2C_Write(0x60, 0x03, value_received, 1);
		
		unsigned char *temp = value_requested;
	
		offset = 20; // video ip
		length = 4;	
		memset(value_received, 0, sizeof(value_received));
		// ip ("ip0.ip1.ip2.ip3") to bytes, ip3 byte first
		parse_ip_to_bytes(temp, value_received);
		I2C_Write(deviceId, offset, value_received, length);		
		offset = 24;  // audio ip
		memset(value_received, 0, sizeof(value_received));
		// ip ("ip0.ip1.ip2.ip3") to bytes, ip3 byte first
		parse_ip_to_bytes(temp, value_received);
		I2C_Write(deviceId, offset, value_received, length);		
		pthread_mutex_unlock(&i2c_bus_lock);
		multicast_ip_change_flag = 1;				
		printf("***write_request--msg_MCASTip\n");
		
		return 0;
	}	
	else
	{
		return 0;
	}
}


char* getStrFrmObject(cJSON* json, const char * key) {
	cJSON * obj = cJSON_GetObjectItem(json, key);
	if (obj && obj->type == cJSON_String)
		return obj->valuestring;
	else
		return NULL;
}

int CheckMacAddr(char *mac_addr_string)
{
	if (!mac_addr_string)
		return -1;

	char tmp[20];
	memset(tmp, 0x0, sizeof(tmp));
	memcpy(tmp, mac_addr_string, 17);
	//printf("%s  %d\n", tmp, strlen(tmp));

	char *strtmp = strtok(tmp, ":");
	int hexnum = 0;
	while (strtmp != NULL)
	{
		if (strlen(strtmp) != 2)
			return -1;
		if ((strtmp[0] >= '0' && strtmp[0] <= '9') || (strtmp[0] >= 'A' && strtmp[0] <= 'F') || (strtmp[0] >= 'a' && strtmp[0] <= 'f')) {
			if ((strtmp[1] >= '0' && strtmp[1] <= '9') || (strtmp[1] >= 'A' && strtmp[1] <= 'F') || (strtmp[1] >= 'a' && strtmp[1] <= 'f')) {
				hexnum++;
				strtmp = strtok(NULL, ":");
			}
			else {
				return -1;
			}
		}
		else {
			return -1;
		}

	}
	if (hexnum != 6)
		return -1;
	return 0;
}


int getMac(char * mac, int len_limit)
{
	struct ifreq s;
	
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	strcpy(s.ifr_name, "eth0");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) 
	{
		int i;
		for (i = 0; i < 6; ++i)
			printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
		puts("\n");
	} 
	snprintf(mac,
		len_limit,
		"%02X:%02X:%02X:%02X:%02X:%02X",
		(unsigned char) s.ifr_hwaddr.sa_data[0],
		(unsigned char) s.ifr_hwaddr.sa_data[1],

		(unsigned char) s.ifr_hwaddr.sa_data[2],
		(unsigned char) s.ifr_hwaddr.sa_data[3],
		(unsigned char) s.ifr_hwaddr.sa_data[4],

		(unsigned char) s.ifr_hwaddr.sa_data[5]);

	printf("MAC_ADDRESS: %s\n", mac);
	return 0;

}

void setip(unsigned char * ip)
{
	unsigned char  command[50];	
	/*make command*/
	sprintf((char*)command, "ifconfig %s %s", port_name, ip);	
	system(command);     
	printf("Ip updated sucessfully.\n");
}

void getip(unsigned char * ret, int param)
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];
	char mask[NI_MAXHOST];
	char bcast_addr[NI_MAXHOST];
	char dest_addr[NI_MAXHOST];	
	if (getifaddrs(&ifaddr) == -1) 
	{
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	//printf("ifaddress = %s", ifaddr->ifa_name);

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr == NULL)
			continue;  

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

		if ((strcmp(ifa->ifa_name, "eth0") == 0)&&(ifa->ifa_addr->sa_family == AF_INET))
		{
			if (s != 0)
			{
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}
			//printf("\tInterface : <%s>\n", ifa->ifa_name);
			//printf("\t  Address : <%s>\n", host); 
			
			s = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in), mask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			//printf("\t  Mask : <%s>\n", mask); 

			s = getnameinfo(ifa->ifa_ifu.ifu_broadaddr, sizeof(struct sockaddr_in), bcast_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			//printf("\t  Braodcast address : <%s>\n", bcast_addr); 
			
			s = getnameinfo(ifa->ifa_ifu.ifu_dstaddr, sizeof(struct sockaddr_in), dest_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			//printf("\t  Dest address : <%s>\n", dest_addr); 
			
			switch (param)
			{
			case GET_IP: 
				//printf("\nInside loop\n");
				for (int i = 0; i < 18; i++)
				{
					ret[i] = host[i];
				}
				break;
			case GET_MASK:
				for (int i = 0; i < 18; i++)
				{
					ret[i] = mask[i];
				}
				break;				
			}	
		}
	}

	freeifaddrs(ifaddr);
}

void fpga_ip_addr_inf(cJSON * root_msg)
{
	byte deviceId = 0x68;
	byte offset, ret;
	byte data_write[6];
	byte data_read[46];
	word length;
	int ip_addr;
	unsigned char *ip;
	unsigned char *mcastip;
	unsigned char *mac;
	unsigned char * port;
	unsigned char ip_read[16];
	unsigned char mac_read[18];
	unsigned char *flag;
	char mac_data[15];
	memset(data_read, 0x00, 46);
	
	cJSON *json_root_default = root_msg;
	cJSON *json_array_default;
	cJSON *json_object_parameter_pool_default;

	json_array_default = cJSON_GetObjectItem(json_root_default, default_state[msg_data].key);
	json_object_parameter_pool_default = cJSON_GetArrayItem(json_array_default, 0);
	
	for (int i = 0; i < msg_num; i++) 
	{
		if (i == msg_ip)
		{
			if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key) != NULL)
			{
				ip = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key)->valuestring;
				offset = 16;
				length = 4;
				parse_ip_to_bytes(ip, data_write);
				I2C_Write(deviceId, offset, data_write, length);
			}
		}		
		else if (i == msg_mac)
		{
			if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key) != NULL)
			{
				mac = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key)->valuestring;
				offset = 4;
				length = 6;
				parse_mac_to_bytes(mac, data_write);
				I2C_Write(deviceId, offset, data_write, length);				
			}
		}
		else if(i == msg_VideoPortDest)
		{
			int ret;
			char * ptr;
			if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key) != NULL)
			{
				port = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key)->valuestring;
				offset = 34;
				length = 2;
				ret = strtol(port, &ptr, 10);
				data_write[0] = ret & 0xFF;
				data_write[1] = (ret >> 8);
				I2C_Write(deviceId, offset, data_write, length);
			}
		}
		else if(i == msg_VideoPortLocal)
		{
			int ret;
			char * ptr;
			if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key) != NULL)
			{
				port = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key)->valuestring;
				offset = 32;
				length = 2;
				ret = strtol(port, &ptr, 10);
				data_write[0] = ret & 0xFF;
				data_write[1] = (ret >> 8);
				I2C_Write(deviceId, offset, data_write, length);
			}			
		}
		
		//disable FPGA video stream enable at power on, it should be in ADV_routine. When there is a stable HDMI input.
		/*	
		else if(i == msg_isConnect)
		{
			int ret;
			char * ptr;
			if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key) != NULL)
			{
				flag = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key)->valuestring;
				ret = strtol(flag, &ptr, 10);
				if (1 == ret)
				{
					//I2C_Read(0x60, 0x03, data_write, 1);
					data_write[0] = 0x01;
					I2C_Write(0x60, 0x03, data_write, 1);
					connection_enable_flag = 1;
				}
				else
				{
					//I2C_Read(0x60, 0x03, data_write, 1);
					data_write[0] =0x00;
					I2C_Write(0x60, 0x03, data_write, 1);
					connection_enable_flag = 0;
				}
			}
		}
		*/		
		else if(i == msg_MCASTip)
		{
			if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key) != NULL)
			{
				mcastip = cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[i].key)->valuestring;
				printf("fpga init get mcastip : %s\n", mcastip);
				memset(mac_data, 0, sizeof(mac_data));
				// ip ("ip0.ip1.ip2.ip3") to mac ("01:00:5e:ip1:ip2:ip3") in bytes, ip3 byte first 
				parse_ip_to_mcast_mac(mcastip, mac_data);
				I2C_Write(0x68, 10, mac_data, 6);	
				memset(mac_data, 0, sizeof(mac_data));
				// ip ("ip0.ip1.ip2.ip3") to bytes, ip3 byte first 
				parse_ip_to_bytes(mcastip, mac_data);
				//video IP
				I2C_Write(0x68, 20, mac_data, 4);
				//audio IP
				I2C_Write(0x68, 24, mac_data, 4);
			}
		}
	}	
}

void parse_ip_to_bytes(unsigned char * ip_in, unsigned char * data_write)
{
	char temp[15];
	for (int i = 0; i < 15; i++)
	{
		temp[i] = ip_in[i];
	}
	char* token = malloc(10);
	token = strtok(temp, ".");
	for (int i = 3; i >= 0; i--)
	{
		data_write[i] = (unsigned char)atoi(token);
		token = strtok(NULL, ".");
	}
	free(token);
}

void parse_bytes_to_ip(unsigned char *data_read, unsigned char * ip_out)
{
	sprintf(ip_out, "%d.%d.%d.%d", data_read[3], data_read[2], data_read[1], data_read[0]);
	//printf("%s", ip_out);
}

void parse_bytes_to_mac(unsigned char *data_read, unsigned char * mac_out)
{
	sprintf(mac_out, "%02X:%02X:%02X:%02X:%02X:%02X", data_read[5], data_read[4], data_read[3], data_read[2], data_read[1], data_read[0]);
	//printf("MAC adress : %s\n", mac_out);
}

void parse_mac_to_bytes(unsigned char * mac_in, unsigned char * data_write)
{
	int values[6];
	int i;

	if (6 == sscanf(mac_in,
		"%X:%X:%X:%X:%X:%X",
		&values[0],
		&values[1],
		&values[2],
		&values[3],
		&values[4],
		&values[5]))
	{
		/* convert to uint8_t */
		for (i = 0; i < 6; i++)
			data_write[i] = (unsigned char) values[5 - i];
	}
}

char* GetGatewayForInterface(const char* interface) 
{
	static char  gateway[20];
	char dest[20];

	char cmd[1000] = { 0x0 };
	sprintf(cmd, "route -n | grep %s  | grep UG  ", interface);
	FILE* fp = popen(cmd, "r");
	char line[256] = { 0x0 };

	if (fgets(line, sizeof(line), fp) != NULL)
	{
		sscanf(line, "%s   %s", dest, gateway);
	}
	pclose(fp);
	return gateway;
}

void set_gateway(unsigned char * gateway)
{
	int sockfd;
	struct rtentry rt;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		perror("socket creation failed\n");
		return;
	}

	struct sockaddr_in *sockinfo = (struct sockaddr_in *)&rt.rt_gateway;
	sockinfo->sin_family = AF_INET;
	sockinfo->sin_addr.s_addr = inet_addr(gateway);

	sockinfo = (struct sockaddr_in *)&rt.rt_dst;
	sockinfo->sin_family = AF_INET;
	sockinfo->sin_addr.s_addr = INADDR_ANY;

	sockinfo = (struct sockaddr_in *)&rt.rt_genmask;
	sockinfo->sin_family = AF_INET;
	sockinfo->sin_addr.s_addr = INADDR_ANY;

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = "eth0";

	if (ioctl(sockfd, SIOCADDRT, &rt) < 0)
		perror("ioctl");

	return;
}

void parse_ip_to_mcast_mac(unsigned char * ip_in, unsigned char * data_write)
{
	char temp[15];
	int values[6], i;
	for (int i = 0; i < 15; i++)
	{
		temp[i] = ip_in[i];
	}

	//char * temp = "01:00:5e:00:00:01";

	values[0] = 0x01;
	values[1] = 0x00;
	values[2] = 0x5e;

	char* token = malloc(10);
	token = strtok(temp, ".");
	token = strtok(NULL, ".");
	for (int i = 3; i <= 5; i++)
	{
		values[i] = (unsigned char)atoi(token);
		token = strtok(NULL, ".");
	}
	free(token);
	for (i = 0; i < 6; i++)
		data_write[i] = (unsigned char) values[5 - i];
}

int read_edid_from_file(char *edid_str)
{
	FILE *fp = NULL;
	unsigned char EdidBuffer[256];
	int i;
	int ret;
	memset(EdidBuffer, 0, sizeof(EdidBuffer));
	fp = fopen(EDID_DEFAULT, "rb");
	if (fp == NULL)
	{
		printf("Read edid open file error!\n");
		return -1;
	}
	ret = fread(EdidBuffer, 1, 256, fp);
	if (ret < 128)
	{
		printf("Read edid length error!\n");
		return -1;
	}
	for (int i = 0; i < 256; i++)
		sprintf(edid_str + (i * 2), "%02x", EdidBuffer[i]);
	edid_str[513] = '\0';	
	fclose(fp);
	return 0;
}

int write_edid_to_file(char *edid_str)
{
	FILE *fp = NULL;
	unsigned char EdidBuffer[256];
	int i;
	int ret;
	memset(EdidBuffer, 0, sizeof(EdidBuffer));
	fp = fopen(EDID_DEFAULT, "wb");
	if (fp == NULL)
	{
		printf("Set edid open file error!\n");
		return -1;
	}
	StrToHex(EdidBuffer, edid_str, 256);
		
	ret = fwrite(EdidBuffer, 1, 256, fp);
	if (ret < 256)
	{
		printf("Set edid write length error!\n");
		return -1;
	}
	fclose(fp);
	return 0;	
}

void json_value_init(cJSON* root_msg) 
{
	unsigned int addr;
	unsigned char  prtad;
	unsigned char  devad;
	unsigned int buff;
	unsigned char length;
	unsigned char data[4];
	unsigned char fpga_buff[32];
	char buffer[20];
	
	cJSON *json_root_default = root_msg;
	cJSON *json_array_default;
	cJSON *json_object_parameter_pool_default;

	json_array_default = cJSON_GetObjectItem(json_root_default, default_state[msg_data].key);
	json_object_parameter_pool_default = cJSON_GetArrayItem(json_array_default, 0);

	//dip switch
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_isDipOn].key) != NULL) 
	{
		char * strDip = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_isDipOn].key);
		int dipOn = atoi(strDip);
		unsigned int dipSwitchValue;		
#ifdef MULTICAST_BY_SWITCH
		if (1 == dipOn)
		{
			dipSwitchValue = readDipSwitch() + 1;   //map to ip from 1
			if (dipSwitchValue != 0xff) // ??? check, has to be "!= 0x100)"
			{
				char * multicaststr = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_MCASTip].key);		
				sprintf(multicaststr, "239.0.0.%d", dipSwitchValue);
				sprintf(g_multicast_ip, "239.0.0.%d", dipSwitchValue);
				printf("Dip ON, change multicast ip to : %s\n", multicaststr);				
				cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_MCASTip].key, cJSON_CreateString(multicaststr));
			}
			else
				printf("Dip switch is ON, cannot get dip switch value. Use default multicast ip\n");
		}			
#else // default "239.0.1.254"
		{
			char * multicaststr = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_MCASTip].key);		
			strncpy(multicaststr, g_multicast_ip, 15);
			cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_MCASTip].key, cJSON_CreateString(multicaststr));
			printf("Defined to use default multicast ip : %s\n", g_multicast_ip);
		}
#endif
			}
	
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_isConnect].key) != NULL) 
	{
		char * strisconnect = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_isConnect].key);	
		if (1 == atoi(strisconnect))				
			connection_enable_flag = 1;					
		else
			connection_enable_flag = 0;						
	}
		
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_MCASTip].key) != NULL) 
	{
		char * multicast_ip_init = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_MCASTip].key);	
		strncpy(g_multicast_ip, multicast_ip_init, 15);				
	}		
				
	devad = 0x60;
	addr = 0x00;
	length = 21;
	memset(fpga_buff, 0x00, sizeof(fpga_buff));
	I2C_Read(devad, addr, fpga_buff, length);
			
	buff = fpga_buff[13] << 24 | fpga_buff[12] << 16 | fpga_buff[11] << 8 | fpga_buff[10];
			
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_x_FrontPorch].key) != NULL)
	{	
		unsigned short x_front_porch;
		x_front_porch = buff & 0x07FF;
				
		memset(buffer, 0x00, sizeof(buffer));
		sprintf(buffer, "%d", x_front_porch);
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_x_FrontPorch].key, cJSON_CreateString(buffer));
	}
			
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_x_PulseCount].key) != NULL)
	{	
		unsigned short x_pulse_count;
		x_pulse_count = buff >> 11 & 0x03FF;
				
		memset(buffer, 0x00, sizeof(buffer));
		sprintf(buffer, "%d", x_pulse_count);
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_x_PulseCount].key, cJSON_CreateString(buffer));
	}		
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_x_BackPorch].key) != NULL)
	{	
		unsigned short x_back_porch;
		x_back_porch = buff >> 21 & 0x03FF;
				
		memset(buffer, 0x00, sizeof(buffer));
		sprintf(buffer, "%d", x_back_porch);
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_x_BackPorch].key, cJSON_CreateString(buffer));
	}
			
	buff = fpga_buff[17] << 24 | fpga_buff[16] << 16 | fpga_buff[15] << 8 | fpga_buff[14];
			
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_y_FrontPorch].key) != NULL)
	{	
		unsigned short y_front_porch;
		y_front_porch = buff & 0x07FF;
				
		memset(buffer, 0x00, sizeof(buffer));
		sprintf(buffer, "%d", y_front_porch);
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_y_FrontPorch].key, cJSON_CreateString(buffer));
	}			
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_y_PulseCount].key) != NULL)
	{	
		unsigned short y_pulse_count;
		y_pulse_count = buff >> 11 & 0x03FF;
				
		memset(buffer, 0x00, sizeof(buffer));
		sprintf(buffer, "%d", y_pulse_count);
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_y_PulseCount].key, cJSON_CreateString(buffer));
	}		
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_y_BackPorch].key) != NULL)
	{	
		unsigned short y_back_porch;
		y_back_porch = buff >> 21 & 0x03FF;
				
		memset(buffer, 0x00, sizeof(buffer));
		sprintf(buffer, "%d", y_back_porch);
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_y_BackPorch].key, cJSON_CreateString(buffer));
	}
			
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_res].key) != NULL)
	{	
		unsigned short x, y;
		x = fpga_buff[7] << 8 | fpga_buff[6];
		y = fpga_buff[9] << 8 | fpga_buff[8];
				
		memset(buffer, 0x00, sizeof(buffer));
		sprintf(buffer, "%d%s%d", x, "x", y);
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_res].key, cJSON_CreateString(buffer));
	}
			
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_isRawMode].key) != NULL)
	{	
		unsigned char raw_mode = 0;
		if (0x0C == fpga_buff[1])
			raw_mode = 1;
				
		memset(buffer, 0x00, sizeof(buffer));
		sprintf(buffer, "%d", raw_mode);
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_isRawMode].key, cJSON_CreateString(buffer));
	}
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_ColorSpace].key) != NULL)
	{	
		unsigned char c_space;
				
		devad = 0x60;
		addr = 0x01;
		c_space = fpga_buff[1] >> 2 & 0x03;
		switch (c_space)
		{
		case 0: sprintf(buffer, "%s", "YCbCr444"); break;
		case 1: sprintf(buffer, "%s", "YCbCr422"); break;
		case 2: sprintf(buffer, "%s", "x420"); break;
		case 3: sprintf(buffer, "%s", "x_raw"); break;
		case 4: sprintf(buffer, "%s", "RGB"); break;					
		}
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_ColorSpace].key, cJSON_CreateString(buffer));
	}	
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_ColorDepth].key) != NULL)
	{	
		unsigned char c_depth;
				
		devad = 0x60;
		addr = 0x01;
		c_depth = fpga_buff[1] & 0x03;
		switch (c_depth)
		{
		case 0: sprintf(buffer, "%s", "8 bit"); break;
		case 1: sprintf(buffer, "%s", "10 bit"); break;
		case 2: sprintf(buffer, "%s", "12 bit"); break;
		case 3: sprintf(buffer, "%s", "16 bit"); break;
		}
		cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_ColorDepth].key, cJSON_CreateString(buffer));
	}	
	
	//valid only in release, otherwise visualGDB lost connection
	//First of all define MAC
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_mac].key) != NULL) 
	{
		char mac_addr_str[18];
		int isValidMac = 0;
		//if (readParamFromDrive(CONFIG_INI, macaddr) == 0) 
		if (SHA256_Read_MacAddr(mac_addr_str) == 0) 	
		{ 	//if reading MAC from chip is OK
			printf("Read mac from rom : %s\n", mac_addr_str);			
			if (CheckMacAddr(mac_addr_str) == 0) 
			{
				cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_mac].key,
						cJSON_CreateString(mac_addr_str));
				cJSON_ReplaceItemInObject(json_root_default, default_state[msg_targ].key,
						cJSON_CreateString(mac_addr_str));
				char cmd[256];				
				sprintf(cmd, "ifconfig %s down >/dev/null", "eth0");				
				system(cmd);
				sprintf(cmd, "ifconfig %s hw ether %s >/dev/null", "eth0", mac_addr_str);
				system(cmd);
				sprintf(cmd, "ifconfig %s up >/dev/null", "eth0");				
				system(cmd);
				system("ethtool -s eth0 speed 100 duplex full autoneg off");
				isValidMac = 1;
			}
		}

		if (isValidMac == 0) 			
		{ 	
			//if Failed to get MAC from chip, use the MAC randomly given by kernel
			if (getMac(mac_addr_str, 18) == 0) 
			{
				cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_mac].key,
						cJSON_CreateString(mac_addr_str));
				cJSON_ReplaceItemInObject(json_root_default, default_state[msg_targ].key,
						cJSON_CreateString(mac_addr_str));
			}
		}		
	}		
	
	char * tmp;	
	char cmd[256];
	struct sockaddr_in addr4;
	//valid only in release, otherwise visualGDB lost connection	
	//Init Ethernet related parameters processing
	tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_ip].key);
	if (inet_pton(AF_INET, tmp, &addr4.sin_addr) > 0) 
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "ifconfig %s %s >/dev/null", DEFAULT_NETWORK_NAME, tmp);
		system(cmd);		
	}
	tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_mask].key);
	if (inet_pton(AF_INET, tmp, &addr4.sin_addr) > 0) 
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "ifconfig %s netmask %s >/dev/null", DEFAULT_NETWORK_NAME, tmp);
		system(cmd);
	}
	tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_gateway].key);
	if (inet_pton(AF_INET, tmp, &addr4.sin_addr) > 0) 
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "route add default gw %s dev %s >/dev/null", tmp, DEFAULT_NETWORK_NAME);
		system(cmd);	
	}
	char * isDhcpOn = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_isDhcp].key);
	if (isDhcpOn && strcmp("1", isDhcpOn) == 0)
	{
		memset(cmd, 0, sizeof(cmd));		
		sprintf(cmd, "udhcpc -n -i %s -t 5", DEFAULT_NETWORK_NAME);
		if (system(cmd) == 0) 
		{
			printf("udhcpc succeed!\n");
			//sleep(6);
			struct sockaddr_in addr4;
			char ipaddr[30] = "";
			memset(ipaddr, 0, 30);
			if (getLocalIp(DEFAULT_NETWORK_NAME, ipaddr) == 0) 
			{
				if (ipaddr[0] != 0 && (inet_pton(AF_INET, ipaddr, &addr4.sin_addr) > 0))
					cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_ip].key,
							cJSON_CreateString(ipaddr));				
			}

			memset(ipaddr, 0, 30);
			getNetmask(DEFAULT_NETWORK_NAME, ipaddr);
			if (ipaddr[0] != 0 && (inet_pton(AF_INET, ipaddr, &addr4.sin_addr) > 0))
				cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_mask].key,
						cJSON_CreateString(ipaddr));

			memset(ipaddr, 0, 30);
			GetGateWay(ipaddr);
			if (ipaddr[0] != 0 && (inet_pton(AF_INET, ipaddr, &addr4.sin_addr) > 0))
				cJSON_ReplaceItemInObject(json_object_parameter_pool_default, default_state[msg_gateway].key,
						cJSON_CreateString(ipaddr));
		}
	}

	//rs232 & passthrough init
	tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232Baudrate].key);
	int baudrate = 9600;
	if (tmp)
		baudrate = atoi(tmp);
	tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232Databits].key);
	int databits = 8;
	if (tmp)
		databits = atoi(tmp);
	tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232Stopbits].key);
	int stopbits = 1;
	if (tmp)
		stopbits = atoi(tmp);
	tmp = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232Parity].key);
	char parity = 'n';
	if (tmp) 
	{
		if (strcmp(tmp, "none") == 0) 
		{
			parity = 'n';
		} 
		else if (strcmp(tmp, "odd") == 0) 
		{
			parity = 'o';
		} 
		else if (strcmp(tmp, "even") == 0) 
		{
			parity = 'e';
		}
	}
	int s32Ret = setUart(baudrate, databits, stopbits, parity, 0);
	if (s32Ret != 0) 	
		printf("Set rs232 failed!\n");	

	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_RS232FeedbackIp].key) != NULL) 
	{
		char *content_of_value;
		content_of_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_RS232FeedbackIp].key);		
		strncpy(RS232_REMOTE_IP, content_of_value, 15);
		rs232PauseFlag = 0;
	}
#ifdef IR_PASSTHROUGH_ENABLE	
	//IR & passthrough init
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_IRFeedbackIp].key) != NULL) 
	{
		char *content_of_value;
		content_of_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_IRFeedbackIp].key);
		strncpy(IR_REMOTE_IP, content_of_value, 15);
	}
	if (cJSON_GetObjectItem(json_object_parameter_pool_default, default_state[msg_IRState].key) != NULL) 
	{
		char *content_of_value;		
		content_of_value = getStrFrmObject(json_object_parameter_pool_default, default_state[msg_IRState].key);				
		irPauseFlag = 1;
		printf("json init set irPauseFlag = 1\n");		
		if (strcmp(content_of_value, "1") == 0)
		{	
			printf("init IR to sensor mode.\n");
			//set to idle mode first 
			setChipIRMode("0");
			Set_FPGA_IR_Mode(1);
			//set to sensor mode
			setChipIRMode("1");
		}
		else if(strcmp(content_of_value, "2") == 0)
		{	
			printf("init IR to emitter mode.\n");
			//set to idle mode first 
			setChipIRMode("0");
			Set_FPGA_IR_Mode(2);
			//set to emitter mode, default 38KHz			
			Write_FPGA_IR_Emitter_Freqency(0x3);
			setChipIRMode("2");
		}
		else //idle mode
		{			
			setChipIRMode("0");
			printf("json value init--IR_STATE parameter error! %s \n", content_of_value);
		}		
		irPauseFlag = 0;
		printf("json init set irPauseFlag = 0\n");
	}
#endif
}

int Set_FPGA_IR_Mode(int mode)
{
	unsigned char fpga_data[10];
	unsigned char carrier_data;
	unsigned char ir_ctrl_data;
	if (mode == 1)	//sensor mode
	{		
		ir_ctrl_data = 0;
		ir_ctrl_data |= 0x20;					//IR sensor mode, FPGA demodulation
		fpga_data[0] = ir_ctrl_data;		
		pthread_mutex_lock(&i2c_bus_lock);
		I2C_Write(0x60, 0x02, fpga_data, 1);
		pthread_mutex_unlock(&i2c_bus_lock);	
	}
	else if (mode == 2)
	{										
		//set IR emitter mode in FPGA
		ir_ctrl_data = 0;
		ir_ctrl_data |= 0x10;						//IR emitter mode									
		//ir_ctrl_data |= (carrier_data & 0x0F); 	//IR carrier frequency is unknown at beginning
		fpga_data[0] = ir_ctrl_data;	
		pthread_mutex_lock(&i2c_bus_lock);		
		I2C_Write(0x60, 0x02, fpga_data, 1);													
		pthread_mutex_unlock(&i2c_bus_lock);		
	}
	else
	{
		printf("FPGA IR mode error!\n");
		return -1;
	}
	return 0;
}


//frequency table inside FPGA
//0000 : 30K
//0001 : 33K
//0010 : 36K
//0011 : 38K
//0100 : 40K
//0101 : 56K
//1111 : error 
int Write_FPGA_IR_Emitter_Freqency(unsigned char frequency)
{
	unsigned char fpga_data[10];
	unsigned char carrier_data;
	unsigned char ir_ctrl_data;
	
	if(frequency < 0xF)
	{		
		ir_ctrl_data = 0;
		ir_ctrl_data |= 0x10;						//IR emitter mode
		ir_ctrl_data |= (frequency & 0x0F);
		fpga_data[0] = ir_ctrl_data;
		pthread_mutex_lock(&i2c_bus_lock);
		I2C_Write(0x60, 0x02, fpga_data, 1);
		pthread_mutex_unlock(&i2c_bus_lock);				
		return 0;
	}
	else
	{
		printf("Write_FPGA_IR_Emitter_Freqency frequency( %d )error!\n");
		return -1;
	}
}

unsigned char Read_FPGA_IR_Emitter_Freqency()
{
	unsigned char fpga_data[10];
	unsigned char carrier_data;
	unsigned char ir_ctrl_data;
	unsigned char frequency;
	
	pthread_mutex_lock(&i2c_bus_lock);
	I2C_Read(0x60, 0x05, fpga_data, 1);
	pthread_mutex_unlock(&i2c_bus_lock);
	frequency = fpga_data[0] & 0x0F;		
	return frequency;	
}

