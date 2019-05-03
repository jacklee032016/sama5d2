
#ifndef	__EXT_UDP_CMD_H__
#define	__EXT_UDP_CMD_H__

#define		EXT_CTRL_PORT							3600

#define		CMD_TAG_REQUEST						(0xa0fa)		/* request */
#define		CMD_TAG_RESPONSE						(0xa0fb)		/* response */




#define	EXT_IP_CMD_MAX_LENGTH					1020

#define	IPCMD_HEADER_LENGTH						4

struct	_cmn_ext_ip_cmd
{
	unsigned short		tag;
	unsigned short		length;	/* include CRC 4 bytes */
	
	unsigned char			data[EXT_IP_CMD_MAX_LENGTH];	/* it should be 65531 to optimize memory allocation */
}__attribute__ ((packed));


typedef struct _cmn_ext_ip_cmd CMN_IP_COMMAND;

/* IP command lists */
#define	EXT_IPCMD_CMD_GET_PARAMS				"get_param"
#define	EXT_IPCMD_CMD_SET_PARAMS				"set_param"

#define	EXT_IPCMD_CMD_SEND_RS232				"send_data_rs232"
#define	EXT_IPCMD_CMD_SECURITY_CHECK			"security_check"


/* fields in IP command */
#define	EXT_IPCMD_KEY_TARGET						"targ"
#define	EXT_IPCMD_KEY_COMMAND					"cmd"

#define	EXT_IPCMD_LOGIN_ACK						"login-ack" /* login user name or status */
#define	EXT_IPCMD_PWD_MSG						"pwd-msg" /* login password or status msg */

#define	EXT_IPCMD_DATA_IS_MCAST					"isMCAST"
#define	EXT_IPCMD_DATA_MCAST_IP					"MCASTip"


#define	EXT_IPCMD_DATA_BACKUP_MC_IP				"bMcIp"		/* backup Mutlicast IP */

#define	EXT_IPCMD_DATA_RS_BAUDRATE				"RS232Baudrate"
#define	EXT_IPCMD_DATA_RS_DATABITS				"RS232Databits"
#define	EXT_IPCMD_DATA_RS_PARITY					"RS232Parity"
#define	EXT_IPCMD_DATA_RS_STOPBITS				"RS232Stopbits"
#define	EXT_IPCMD_DATA_RS_FEEDBACKIP			"RS232FeedbackIp"


#define	EXT_IPCMD_DATA_VIDEO_PORT				"vidPort"
#define	EXT_IPCMD_DATA_AUDIO_PORT				"audPort"
#define	EXT_IPCMD_DATA_AD_PORT					"datPort"
#define	EXT_IPCMD_DATA_ST_PORT					"strPort"


#define	EXT_IPCMD_DATA_VIDEO_WIDTH				"vidW"
#define	EXT_IPCMD_DATA_VIDEO_HEIGHT				"vidH"
#define	EXT_IPCMD_DATA_VIDEO_FRAMERATE			"vidFps"
#define	EXT_IPCMD_DATA_VIDEO_COLORSPACE		"vidClrSpace"
#define	EXT_IPCMD_DATA_VIDEO_DEPTH				"vidDepth"

#define	EXT_IPCMD_DATA_VIDEO_INTERLACED		"vidIsIntlc"
#define	EXT_IPCMD_DATA_VIDEO_SEGMENTED			"vidIsSgmt"


#define	EXT_IPCMD_DATA_AUDIO_SAMPE_RATE		"audFrq"
#define	EXT_IPCMD_DATA_AUDIO_DEPTH				"audBit"
#define	EXT_IPCMD_DATA_AUDIO_CHANNELS			"audChan"

#define	EXT_IPCMD_DATA_IS_CONNECT				"IsConnect"	/* 811 send to TX(stop/start) or RX(connect/disconnect) */

#define	EXT_IPCMD_DATA_811_IP						"811Ip"
#define	EXT_IPCMD_DATA_811_PORT					"811Port"



void extIpCmdAgentInit(EXT_JSON_PARSER  *parser);


char extJsonRequestParse(EXT_JSON_PARSER *parserr, EXT_RUNTIME_CFG	*runCfg);


char extIpCmdResponseHeaderPrint(EXT_JSON_PARSER  *parser);
char extIpCmdResponseTailCalculate(EXT_JSON_PARSER  *parser, char isRequest);

char	extIpCmdResponseReply(EXT_JSON_PARSER  *parser);
char	extJsonResponsePrintConfig(EXT_JSON_PARSER  *parser);


char extIpCmdSecurityCheck(EXT_JSON_PARSER  *parser);
char extIpCmdSendRsData(EXT_JSON_PARSER  *parser);

char extIpCmdSetupParams(EXT_JSON_PARSER  *parser);

char extJsonUserValidate(EXT_JSON_PARSER  *parser);

int	extIpCmdPrintMediaCfg(EXT_JSON_PARSER  *parser, char *data, int size);
/* send IP command by TX */

char extIpCmdRequestHeaderPrint(EXT_JSON_PARSER  *parser, const char *cmd);

char	extIpCmdSendMediaData(EXT_JSON_PARSER  *parser, char isRequest);

char extIpCmdSendout(EXT_JSON_PARSER  *parser, uint32_t *ip, unsigned short port);


char extIpCmdIsLocal(EXT_JSON_PARSER  *parser);


#endif


