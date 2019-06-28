
/*
* Common of all Mux programs, both for Player, Server and Web, or RX/TX
* Main runtime data, Plugins, and all about boards defined here
*/

#ifndef	__LIB_MUX_H__
#define	__LIB_MUX_H__

#include <netinet/in.h>
#include <sys/queue.h>

#ifndef LIST_FOREACH_SAFE
#define	LIST_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = LIST_FIRST((head));				\
	    (var) && ((tvar) = LIST_NEXT((var), field), 1);		\
	    (var) = (tvar))
#endif

#include "ms_version.h"



#define	MUX_MAIN_CONFIG_FILE				CONFIG_FILE_HOME_PROJECT"muxMain.conf"

#define	MUX_PLAYER_CONFIG_FILE			CONFIG_FILE_HOME_PROJECT"muxPlayer.conf"
#define	MUX_RECORDER_CONFIG_FILE			CONFIG_FILE_HOME_PROJECT"muxRecorder.conf"

#ifdef	WIN32
#define	DIR_SPLITTER_CHAR					'\\'
#define	HOME_DIR							"d:\\works\\webservers\\webSvr\\"
#define	MUX_WEB_CONFIG_FILE 				HOME_DIR"config\\etc\\webSvr\\webSvr.win.conf" /* locate me in the server root */
#else
#define	DIR_SPLITTER_CHAR					'/'
#define	HOME_DIR							"/works/webserver/webSvr/"
#if ARCH_X86
#define	MUX_WEB_CONFIG_FILE 				CONFIG_FILE_HOME_PROJECT"muxWebX86.conf" /* locate me in the server root */
#else
#define	MUX_WEB_CONFIG_FILE 				CONFIG_FILE_HOME_PROJECT"muxWeb.conf" /* locate me in the server root */
#endif
#endif


#define	MUX_SYSTEM_JSON_CONFIG			CONFIG_FILE_HOME_PROJECT"muxSystem.json"

#define	MUX_SYSTEM_CONFIG_DATA			CONFIG_FILE_HOME_PROJECT"muxConfig.dat"


#define	CRON_TASK_FILE						"/var/spool/cron/crontabs/root"


#define	MUX_AUTH_USER						"admin"
#define	MUX_AUTH_PASSWORD				"admin"

#define	UNIX_SOCKET_SVR_PATH					"/tmp/unixsocketServer"
#define	UNIX_SOCKET_CLIENT_PATH				"/tmp/unixsocketClient"


#define	IPCMD_NAME_GET_PARAM				"get_param"
#define	IPCMD_NAME_SET_PARAM					"set_param"
#define	IPCMD_NAME_SEND_RS232				"send_data_rs232"
#define	IPCMD_NAME_SEND_IR					"send_data_ir"
#define	IPCMD_NAME_SECURITY_CHECK			"security_check"
#define	IPCMD_NAME_BLINK_LED					"link_led"


#define	IPCMD_NAME_SYS_ADMIN					"sys_admin"	/* system admin for muxMedia program */




#define	EXT_IPCMD_STATUS_OK						"OK"
#define	EXT_IPCMD_STATUS_FAIL						"NOK"

#define	EXT_IPCMD_DATA_ARRAY					"data" /* login password or status msg */



#define	IPCMD_SYS_ADMIN_THREADS				"threads"
#define	IPCMD_SYS_ADMIN_VER_INFO				"verInfo"



#define	IPCMD_NAME_KEYWORD_TARG			"targ"
#define	IPCMD_NAME_KEYWORD_CMD				"cmd"
#define	IPCMD_NAME_KEYWORD_LOGIN_ACK		"login-ack"
#define	IPCMD_NAME_KEYWORD_PWD_MSG		"pwd-msg"
#define	IPCMD_NAME_KEYWORD_MSG_DETAIL		"detail"

#define	IPCMD_NAME_KEYWORD_DATA			"data"

#define	MEDIA_CTRL_ACTION						"action"
#define	MEDIA_CTRL_OBJECTS					"objects"
#define	MEDIA_CTRL_STATUS						"status"
#define	MEDIA_CTRL_STATUS_MSG				"message"

#define	MEDIA_CTRL_FTP_SERVER					"ftpServer"
#define	MEDIA_CTRL_FTP_USERNAME				"ftpUser"
#define	MEDIA_CTRL_FTP_PASSWORD				"ftpPwd"
#define	MEDIA_CTRL_FTP_PATH					"ftpPath"
#define	MEDIA_CTRL_FTP_FILES					"ftpFiles"


#define	IPCMD_NAME_OSD_INFO					"osdInfo"


#define	IPCMD_NAME_PLAYER_AUDIO				"audio"
#define	IPCMD_NAME_PLAYER_MUTE				"mute"
#define	IPCMD_NAME_PLAYER_MUTE_ALL			"muteAll"

/* set display mode of one player's window */
#define	IPCMD_NAME_ASPECT_WINDOW			"aspect"

#define	MUX_JSON_NAME_ASPECT_MODE			"mode"


#define	IPCMD_NAME_SERVER_CONFIG			"config"
#define	IPCMD_NAME_SERVER_FEEDS				"feeds"
#define	IPCMD_NAME_SERVER_CONNS				"connnects"
#define	IPCMD_NAME_SERVER_URLS				"urls"



#define	EXT_IPCMD_RS232_DATA_HEX					"hexdata"
#define	EXT_IPCMD_RS232_FEEDBACK					"isFeedback"
#define	EXT_IPCMD_RS232_WAIT_TIME				"waitTime"

#define	EXT_IPCMD_RS232_DATA_FEEDBACK			"feedbackdata"



#define	PLAYLIST_NAME_URL						"URL"
#define	PLAYLIST_NAME_DURATION				"Duration"

#define	UDP_BOARD_ADDRESS				"255.255.255.255"
#define	UDP_SERVER_PORT					(3600)

#define	DEFAULT_MC_ADDRESS				"239.0.0.1"

#define	CMD_TAG_A				(0xa0fa)
#define	CMD_TAG_B				(0xa0fb)

typedef	enum
{
	IPCMD_ERR_IN_PROCESSING		= 102, 	/* request is processing, a long time is needed to process this request */
	IPCMD_ERR_NOERROR			= 200, 	/* OK */

	/* 4xx: client error */
	IPCMD_ERR_BAD_REQUEST 		= 400,
	IPCMD_ERR_UNAUTHORIZED 		= 401,
	IPCMD_ERR_FORBIDDEN 			= 403,
	IPCMD_ERR_NOT_FOUND 			= 404,
	IPCMD_ERR_METHOD_NOT_ALLOWED 		= 405,

	/* 5xx: server error */		
	IPCMD_ERR_COMMUNICATION 		= 500,
	IPCMD_ERR_JSON_CORRUPTED,
	IPCMD_ERR_MAC_MISMATCH,
	IPCMD_ERR_CRC_FAIL,
	IPCMD_ERR_WRONG_PROTOCOL,
	IPCMD_ERR_DATA_ERROR,					/* 505 */
	IPCMD_ERR_NOT_SUPPORT_COMMND,

	/* last command is still in the process. Some commands of Player or FTP download will cost a longer time. 
	For FTP, all commands can be queued; but for Player, only one command can be hold at any time */
	IPCMD_ERR_IS_BUSY_NOW,			

	IPCMD_ERR_SERVER_INTERNEL_ERROR,		/* 508 */		


	/* ERROR info from plugin/thread, not from controller */
	IPCMD_ERR_PLUGIN_NOT_EXISTS = 520,

	/* player plugin */
	IPCMD_ERR_PLUGIN_PLAYER_TIMEOUT = 530,
	IPCMD_ERR_PLUGIN_PLAYER_FAILED,

	IPCMD_ERR_FTP_SERVER_ERROR	= 580,	/* can't connect to server */
	IPCMD_ERR_FTP_USER_PASSWORD,	/* username or password is wrong*/
	IPCMD_ERR_FTP_NO_FREESPACE,		/* no space in RX, can't make directory for the file */
	IPCMD_ERR_FTP_NAME_COLLISION,	/* name collision, filename is a local file or local directory */
	IPCMD_ERR_FTP_DATA_ERROR,		/* failed when get data from remote file or save data as local file  */
	IPCMD_ERR_FTP_PARTLY_FAILED,		/* some files failed, some successed in file list  */
	IPCMD_ERR_UNKNOWN
}IPCMD_ERR_CODE;



typedef	enum _CMD_TYPE
{
	CMD_TYPE_HELP = 1,  /* must be > 0 for send in SysV MQ */

	/* command send to command thread */
	CMD_TYPE_OPEN,
	CMD_TYPE_CLOSE,
	CMD_TYPE_PLAY,
	CMD_TYPE_STOP,
	CMD_TYPE_PAUSE,
	CMD_TYPE_RESUME,
	CMD_TYPE_SEEK,
	CMD_TYPE_POS_SEEK,
	CMD_TYPE_BACKWARD,
	CMD_TYPE_FORWARD,
	CMD_TYPE_METADATA,
	CMD_TYPE_INFO,
	CMD_TYPE_SET_ATTR,
	CMD_TYPE_GET_MEDIA,
	CMD_TYPE_GET_RUNINFO,

	CMD_TYPE_SET_MEDIA,	/* command sent by SetMedia thread */

	CMD_TYPE_SUBTITLE,
	CMD_TYPE_SET_HLS,
	CMD_TYPE_DOLBY,
	CMD_TYPE_HDR10,
	CMD_TYPE_SDR,
	
	CMD_TYPE_DBG,

#ifdef DRM_SUPPORT
	CMD_TYPE_DRM,
#endif
	CMD_TYPE_CMD_MAX,  /* max number of command send to Command thread */


	CMD_TYPE_REC_START,
	CMD_TYPE_REC_STOP,
	CMD_TYPE_REC_PAUSE,
	CMD_TYPE_REC_RESUME,

	CMD_TYPE_REC_STATUS,	/* status of recorder */

	CMD_TYPE_SCREEN_SIZE,

	/* CMD for SERVER */
	CMD_TYPE_SVR_CONFIG,	/* all feeds */
	CMD_TYPE_SVR_FEEDS,	/* all feeds */
	CMD_TYPE_SVR_CONNS,	/* all connections */
	CMD_TYPE_SVR_URLS,	/* all URLs */



	/* CMD for WEB */
	CMD_TYPE_WEB_INFOS,	/* all feeds */


	/* Command send to ScanDir thread */
	CMD_TYPE_SCAN_GET_FILES = 100,	/* web retrive media files list */
	CMD_TYPE_SCAN_BEGIN,


	/* command send to Player thread */
	CMD_TYPE_PLAYER_INFO,
	CMD_TYPE_PLAYER_MEDIA,

	CMD_TYPE_SWAP_WINDOW,
	CMD_TYPE_ROTATE_WINDOW,
	CMD_TYPE_LOCATE_WINDOW,

	CMD_TYPE_ASPECT_WINDOW,

	/* OSD control */
	CMD_TYPE_OSD_ENABLE,
	CMD_TYPE_OSD_POSITION,

	CMD_TYPE_OSD_BACKGROUND,

	CMD_TYPE_OSD_TRANSPANRENCY,

	CMD_TYPE_OSD_FONT_SIZE,
	CMD_TYPE_OSD_FONT_COLOR,
	
	CMD_TYPE_OSD_FONT_INFO,
	
	CMD_TYPE_OSD_LOGO,

	/* global command, with Player or Window parameter */

	CMD_TYPE_MUTE_ALL,		/* mute sound card */
	CMD_TYPE_AUDIO,			/* explicit for audio one player */
	CMD_TYPE_MUTE,			/* toggle audio of one player */
	CMD_TYPE_VOLUME_PLUS,
	CMD_TYPE_VOLUME_MINUS,
	
	CMD_TYPE_ALERT_MESSAGE,

	CMD_TYPE_CEC_STAND_BY,
	CMD_TYPE_CEC_IMAGE_ON,
	CMD_TYPE_CEC_VOLUME_UP,
	CMD_TYPE_CEC_VOLUME_DOWN,
	CMD_TYPE_CEC_MUTE,
	CMD_TYPE_CEC_INFO,

	CMD_TYPE_EDID_RESOLUTION,
	CMD_TYPE_EDID_DEEP_COLOR,

	CMD_TYPE_QUIT,
	CMD_TYPE_UNKNOWN
}CMD_TYPE_T;



#define	MUX_REST_STATUS_CODE				"code"
#define	MUX_REST_STATUS_ERROR			"error"
#define	MUX_REST_STATUS_DEBUG			"debug"


#define	MUX_REST_USER_NAME				"username"
#define	MUX_REST_USER_PWD				"passwd"

#define	MUX_REST_URL						"uri"
#define	MUX_REST_METHOD					"method"


#define	MUX_REST_URI_ROOT					"/"
#define	MUX_REST_URI_SYSTEM				"system"
#define	MUX_REST_URI_VIDEO				"video"
#define	MUX_REST_URI_AUDIO				"audio"
#define	MUX_REST_URI_ANC					"anc"
#define	MUX_REST_URI_SDP					"sdp"
#define	MUX_REST_URI_RS232				"rs232"
#define	MUX_REST_URI_SECURITY				"security"
#define	MUX_REST_URI_IR					"IR"
#define	MUX_REST_URI_OTHERS				"others"

#define	MUX_REST_URI_PARAMS				"/params"	/* only for get_params/set_params in IP Command */


/* fields for system sub-items */
#define	FIELD_SYS_CFG_MODEL						"model"
#define	FIELD_SYS_CFG_PRODUCT					"product"
#define	FIELD_SYS_CFG_NAME						"name"
#define	FIELD_SYS_CFG_VERSION						"ver"

#define	FIELD_SYS_CFG_ADDRESS					"ip"
#define	FIELD_SYS_CFG_NETMASK					"mask"
#define	FIELD_SYS_CFG_GATEWAY					"gateway"
#define	FIELD_SYS_CFG_IS_DHCP						"isDhcp"

#define	FIELD_SYS_CFG_MAC							"mac"

#define	FIELD_SYS_CFG_DIP							"isDipOn"
#define	FIELD_SYS_CFG_RESET						"reset"
#define	FIELD_SYS_CFG_REBOOT						"reboot"
#define	FIELD_SYS_CFG_BLINK						"blinkLed"


/* fields in Data array */
#define	EXT_IPCMD_DATA_P_NAME					"pName"
#define	EXT_IPCMD_DATA_C_NAME					"cName"		/* customer name */
#define	EXT_IPCMD_DATA_MODEL						"model"
#define	EXT_IPCMD_DATA_FW_VER					"fwVer"

#define	EXT_IPCMD_DATA_MAC						"mac"
#define	EXT_IPCMD_DATA_IP							"ip"
#define	EXT_IPCMD_DATA_MASK						"mask"
#define	EXT_IPCMD_DATA_GATEWAY					"gateway"
#define	EXT_IPCMD_DATA_DHCP						"isDhcp"

#define	EXT_IPCMD_DATA_IS_DIP						"isDipOn"



#define	SYS_CFG_FIELD_SDP_VEDIO_IP				"sdpVideoIp"
#define	SYS_CFG_FIELD_SDP_AUDIO_IP				"sdpAudioIp"
#define	SYS_CFG_FIELD_SDP_ANC_IP					"sdpAncIp"

#define	SYS_CFG_FIELD_SDP_VEDIO_PORT				"sdpVideoPort"
#define	SYS_CFG_FIELD_SDP_AUDIO_PORT			"sdpAudioPort"
#define	SYS_CFG_FIELD_SDP_ANC_PORT				"sdpAncPort"

#define	SYS_CFG_FIELD_SDP_VEDIO_URI				"sdpVideoUri"
#define	SYS_CFG_FIELD_SDP_AUDIO_URI				"sdpAudioUri"
#define	SYS_CFG_FIELD_SDP_ANC_URI				"sdpAncUri"

/* video fields */
#define	FIELD_VIDEO_IP								"ip"
#define	FIELD_VIDEO_PORT							"port"
#define	FIELD_VIDEO_SDP							"sdp"

#define	FIELD_VIDEO_WIDTH							"width"
#define	FIELD_VIDEO_HEIGHT						"height"

#define	FIELD_VIDEO_FRAME_RATE					"fps"
#define	FIELD_VIDEO_COLOR_SPACE					"colorSpace"
#define	FIELD_VIDEO_DEPTH							"depth"
#define	FIELD_VIDEO_INTERLACE						"intlce"

#define	FIELD_VIDEO_SEGMENTED					"videoIsSgmt"


/* audio fields */
#define	FIELD_AUDIO_IP								"ip"
#define	FIELD_AUDIO_PORT							"port"
#define	FIELD_AUDIO_SDP							"sdp"

#define	FIELD_AUDIO_SAMPLE						"sample"
#define	FIELD_AUDIO_CHANNELS						"channels"
#define	FIELD_AUDIO_DEPTH							"depth"
#define	FIELD_AUDIO_PKT_SIZE						"pktSize"

/* audio fields */
#define	FIELD_ANC_IP								"ip"
#define	FIELD_ANC_PORT								"port"
#define	FIELD_ANC_SDP								"sdp"

/* SDP fields */
#define	FIELD_SDP_MEDIA							"media"
#define	FIELD_SDP_IP								"ip"
#define	FIELD_SDP_PORT								"port"
#define	FIELD_SDP_URI								"uri"


/* RS232 fields */
#define	FIELD_RS232_BAUDRATE						"baudrate"
#define	FIELD_RS232_DATABITS						"databit"
#define	FIELD_RS232_PARITY							"parity"
#define	FIELD_RS232_STOPBITS						"stopbit"

#define	FIELD_RS232_DATA							"data"
#define	FIELD_RS232_IS_FEEDBACK					"isFeedback"
#define	FIELD_RS232_WAIT_TIME						"waitTime"

#define	FIELD_RS232_DATA_FEEDBACK				"feedbackdata"



/* Security Check */
#define	FIELD_SECURITY_GET_ID						"get_id"
#define	FIELD_SECURITY_SET_KEY					"set_key"
#define	FIELD_SECURITY_GET_STATUS					"get_status"



#define	SYS_CFG_FIELD_IP_AUDIO					"ipAudio"
#define	SYS_CFG_FIELD_IP_ANC						"ipAnc"
#define	SYS_CFG_FIELD_IP_AUX						"ipAux"

#define	SYS_CFG_FIELD_PORT_AUDIO					"portAudio"
#define	SYS_CFG_FIELD_PORT_DATA					"portData"
#define	SYS_CFG_FIELD_PORT_STREM					"portStrem"

#define	SYS_CFG_FIELD_FPGA_AUTO					"mediaSet"

#define	SYS_CFG_FIELD_FPGA_AUTO_V_AUTO			"Auto"
#define	SYS_CFG_FIELD_FPGA_AUTO_V_MANUAL		"Manual"
#define	SYS_CFG_FIELD_FPGA_AUTO_V_SDP			"SDP"


#define	SYS_CFG_FIELD_VIDEO_PARAMS				"videoParams"		/* replace width/height/framerate/interlaced(segmented). 01.15, 2019 */


#define	SYS_CFG_FIELD_AUDIO_CHANNEL				"audioChannels"
#define	SYS_CFG_FIELD_AUDIO_SAMP_RATE			"audioSampRate"
#define	SYS_CFG_FIELD_AUDIO_DEPTH				"audioDepth"
#define	SYS_CFG_FIELD_AUDIO_PKT_SIZE				"audioPktSize"
#define	SYS_CFG_FIELD_IS_CONNECT					"isConnect"	/* 811 send to TX(stop/start) or RX(connect/disconnect) */



typedef	struct	_CMD_EVENT
{
#if 0
	CMD_TYPE_T		type;
#else
	int				type;
#endif
	unsigned char		hasParam;
	void				*params;

	cmn_list_t		cmds;	/* cmds parsed out from TLV buffer */
}CMD_EVENT_T;


enum IPAddressAction
{
	ACL_DISABLE = 0,
	IP_ALLOW,
	IP_DENY,
};

typedef struct IPAddressACL
{
	struct IPAddressACL 		*next;
	enum IPAddressAction 		action;
	/* These are in host order */
	struct in_addr 			first;
	struct in_addr 			last;
} IPAddressACL;

enum	RECT_TYPE
{
	RECT_TYPE_MAIN = 0,	
	RECT_TYPE_PIP,	
	RECT_TYPE_SUBTITLE,
	RECT_TYPE_ALERT,
	RECT_TYPE_LOGO,
	RECT_TYPE_UNKNOWN,
};

typedef enum
{
	ROTATE_TYPE_0 = 0,		/* no ratate */
	ROTATE_TYPE_90 = 1,		/* rotate 90 degree clockwise */
	ROTATE_TYPE_180 = 2 ,
	ROTATE_TYPE_270 = 3,
	ROTATE_TYPE_UNKNOWN
}ROTATE_TYPE_T;


typedef	enum
{
	MUX_DISPLAY_MODE_DEFAULT	= 0, /* fill window */
	MUX_DISPLAY_MODE_BLACKBORDER,	/* add black border to show all content and keep ratio of video*/
	MUX_DISPLAY_MODE_CENTRAL_PART,	/* show central part of video */
	MUX_DISPLAY_MODE_SQEEZE_PART,	/* show sqeezed central part of video */
	
	MUX_DISPLAY_MODE_UNKNOWN,
}MUX_DISPLAY_MODE_T;


struct	CTRL_CONN;

struct _MuxPlugin;
struct _MuxMain;
struct _MuxThread;


typedef	struct
{
	char		configFile[CMN_NAME_LENGTH];

	char		captureName[CMN_NAME_LENGTH];

	char		format[CMN_NAME_LENGTH]; /* output format of container */
	int		noVideo;
	int		noAudio;
	int		noSubtitle;

	int		lineNum;

}CmnMuxRecordConfig;

struct _SUBTITLE_FILE_LIST;

typedef struct _SUBTITLE_FILE_LIST
{
	char							name[CMN_NAME_LENGTH];	/* absoluted path */
	struct _SUBTITLE_FILE_LIST	*next;
}SUBTITLE_FILE_LIST_T;



#define	JSON_ACTION_LENGTH		128

typedef	struct _CMN_PLAY_JSON_EVENT
{
	int			event;	/* compatible field, to differeate with other events (FSM events) in Players/server/webservice */
	char			action[JSON_ACTION_LENGTH];
	cJSON		*object;	/* the first item of OBJECTS array which are object of this action */

	int			status;	/* status of event process */
	
//	cJSON		*resultObject;	/* some commands, such as PlayerStatus, MediaInfo, MetaData, RecordStatus, need resultObject to replace the first item in objects array, eg. event->object */

	void 		*priv;	/* struct DATA_CONN */
}CMN_PLAY_JSON_EVENT;


typedef	enum
{
	CTRL_LINK_UDP,		/* unicast and broadcast UDP */
	CTRL_LINK_MULTICAST, /* unicast, broadcast + multicast, not implemented now */
	CTRL_LINK_TCP,		/* TCP socket*/
	CTRL_LINK_UNIX,		/* unix socket */
	CTRL_LINK_UNKNOWN
}CTRL_LINK_TYPE;


struct CLIENT_CONN
{
	CTRL_LINK_TYPE			type;
	int						port;	/* service port */

	

	struct sockaddr_in			peerAddr;
	socklen_t					addrlen;

	int						socket;

	cJSON					*cmd;
	cJSON					*response;

	int						errCode;
};

typedef	struct CLIENT_CTRL
{
//	struct _MuxMain			*muxMain;
	
	int						inited;

	struct CLIENT_CONN		*conn;

}CLIENT_CTRL;



/* description of window/osd */
struct _RECT_CONFIG
{
	enum RECT_TYPE		type;
	char					name[1024];

	char					url[1024];	/* url for this window */
	
	int					left;
	int					top;
	int					width;
	int					height;

	int					fontSize;
	int					fontColor;
	int					backgroundColor;
	char					alpha;
	
	char					enable;

	ROTATE_TYPE_T		rotateType;

	char					audioEnable;

	int					displayMode;
	
	IPAddressACL			*acl;
	
	struct _MuxPlayerConfig	*mainCfg;

	void					*private;	/* its OSD */
};


typedef struct _RECT_CONFIG		RECT_CONFIG;


typedef struct _PLAY_ITEM
{
	char		filename[CMN_NAME_LENGTH];
	int		duration;		/* in second */
}PLAY_ITEM;


typedef struct _PLAY_LIST
{
	char					name[CMN_NAME_LENGTH];

	cmn_list_t			fileList;
	
	int					repeat;
	
	int					datetime;	/* timestamp of playing */

	int					hour;		/* 0~23 */
	int					minute;		/* 0 ~59 */
	int					dayOfWeek;	/* 0~7, both 0 and 7 are Sunday */

	char					enable;

	struct _MuxMain		*muxMain;

	void					*private;
} PLAY_LIST;


/* data structures used to manage configuration */

typedef struct 
{
	int		type;
	char		*name;

	char		*value;
}TYPE_NAME_T;


/***************    for IP command between RX/TX and 811  ***************/
#define	MUX_IP_CMD_MAX_LENGTH		65535

struct	_cmn_mux_ip_cmd
{
	unsigned short		tag;
	unsigned short		length;
	
	char			data[MUX_IP_CMD_MAX_LENGTH];	/* it should be 65531 to optimize memory allocation */
}__attribute__ ((packed));

typedef struct _cmn_mux_ip_cmd CMN_IP_COMMAND;

struct _ipcmd_agent;

typedef	int (*agentInit)(struct _ipcmd_agent *, void *priv);
typedef	void (*agentDestroy)(struct _ipcmd_agent *);

typedef	int (*agentReceive)(struct _ipcmd_agent *);
typedef	int (*agentValidate)(struct _ipcmd_agent *);
typedef	int (*agentAction)(struct _ipcmd_agent *);
typedef	int (*agentReply)(struct _ipcmd_agent *);




typedef	struct _ipcmd_agent
{
	char					name[CMN_NAME_LENGTH];
	
	struct _MuxMain		*muxMain;
	
//	MuxPlayerConfig		*cfg;
	
//	struct CTRL_CONN		*ctrlConns;

	LIST_HEAD(C_CON_HEAD, CTRL_CONN)	ctrlConns;	/* C_CON_HEAD is name of new struct, CTRL_CONN is `struct CTRL_CONN`; ctrlConns is field name in C_CON_HEAD */
	int									connCount;


#if 0
	cmn_list_t			filelist;	/* downloaded file list */

	agentInit				handleInit;
	agentDestroy			handleDestroy;

	agentReceive			handleReceive;
	agentValidate			handleValidate;
	agentAction			handleAction;
	agentReply			handleReply;
#endif

	void					*priv;	
}CMN_MUX_BROKER;

struct	CTRL_CONN;
struct	DATA_CONN;

typedef	struct DATA_CONN* (*CreateDataConn)(struct CTRL_CONN *ctrlConn);//, void *priv);

typedef	int (*DataConnAuthen)(struct DATA_CONN *dataConn, char *user, char *pwd);

typedef	int (*DataConnInput)(struct DATA_CONN *dataConn);
typedef	int (*DataConnOutput)(struct DATA_CONN *dataConn);

typedef	void (*DataConnDestroy)(struct DATA_CONN *dataConn);


struct	DATA_CONN;


/* Control connection for controller : wait for client's request (controller's command) */
struct	CTRL_CONN
{
	LIST_ENTRY(CTRL_CONN)		list;

	int						port;
	CTRL_LINK_TYPE			type;
	char						name[CMN_NAME_LENGTH];	/* add to simplify debugging */
	
	int						sockCtrl;

	CMN_IP_COMMAND			*cmdBuffer;
	unsigned char				buffer[2048];
	int						length;

	CreateDataConn			handleCreateData;

	struct	CTRL_CONN		*next;

	LIST_HEAD(D_CON_HEAD, DATA_CONN)	dataConns;	/* D_CON_HEAD is name of new struct, DATA_CONN is `struct DATA_CONN`; dataConns is field name in D_CON_HEAD */
	int									connCount;
	int									total;

	CMN_MUX_BROKER 		*broker;
};

enum	DATA_CONN_STATUS
{
	DATA_CONN_STATUS_INIT = 0,			/* after created*/
	DATA_CONN_STATUS_WAITING,		/* submit to manager, and waiting manager reply */
	DATA_CONN_STATUS_CLOSING,		/* timeout or closed by peer, but has been submit to manager */
	DATA_CONN_STATUS_FINISHED,		/* has been submit to manager, and reply data or abort reply by manager; then can be freee */
	DATA_CONN_STATUS_UNKNOWN
};

/* one client connection in socket interface */
struct	DATA_CONN
{
	int							port;	/* port of peer */

	int							sock;	/* for TCP and Unix socket */
	int							timeFd;
	

	struct sockaddr_in				peerAddr;	/* for UDP socket */
	socklen_t						addrlen;

	char							name[CMN_NAME_LENGTH];
	char							cmd[CMN_NAME_LENGTH];
	int							method;		

	unsigned char					buffer[2048];
	int							length;
	
	cJSON						*cmdObjs;
	cJSON						*dataObj;	/* refer to first item of DATA array in cmdObjs; can be null when IP Cmd get_params */
	cJSON						*resultObject;	/* refer to data obj in cmdObjs or particular obj in systemObject  */
	
	int							errCode;
	char							detailedMsg[CMN_NAME_LENGTH];
	cmn_mutex_t					*mutexLock;  /* lock between controller and other threads (plugins, FTP) */

	int							status;

	DataConnAuthen				handleAuthen;
	DataConnInput				handleInput;
	DataConnOutput				handleOutput;
	DataConnDestroy				handleDestroy;


	LIST_ENTRY(DATA_CONN)		list;

	struct CTRL_CONN				*ctrlConn;
};

typedef	enum
{
	CMN_JSON_METHOD_GET 	= 0,
	CMN_JSON_METHOD_POST,
	CMN_JSON_METHOD_PUT,
	CMN_JSON_METHOD_DELETE,
	CMN_JSON_METHOD_PATCH,
	CMN_JSON_METHOD_INVALIDATE
}CMN_JSON_METHOD_T;

#define	CMN_MEHOD_STR_GET				"GET"
#define	CMN_MEHOD_STR_POST				"POST"
#define	CMN_MEHOD_STR_PUT				"PUT"
#define	CMN_MEHOD_STR_DELETE				"DELETE"
#define	CMN_MEHOD_STR_PATCH				"PATCH"
#define	CMN_MEHOD_STR_INVALIDATE		"INVALIDATE"

typedef	struct _CMN_JSON_EVENT
{
	int						event;
	char						cmd[JSON_ACTION_LENGTH];
	int						method;
	
	cJSON					*data;
	cJSON					*resultObject;

	int						status;	/* status of event process */
	
	struct DATA_CONN		*dataConn;
	void 					*priv;
}CMN_JSON_EVENT;



typedef	enum _PLUGIN_TYPE
{
	MUX_PLUGIN_TYPE_MAIN = 0,		/* main program, core component */
	MUX_PLUGIN_TYPE_PLAYER,
	MUX_PLUGIN_TYPE_SERVER,
	MUX_PLUGIN_TYPE_WEB,
	MUX_PLUGIN_TYPE_RECORDER,
	MUX_PLUGIN_TYPE_UNKNOWN,
		
}MUX_PLUGIN_TYPE;

typedef	struct _MuxPlugin
{
	char					name[CMN_NAME_LENGTH];
	MUX_PLUGIN_TYPE		type;
	
	int					enable;
	char					path[CMN_NAME_LENGTH];
	char					version[CMN_NAME_LENGTH];

	int	(*signalExit)(struct _MuxPlugin *, int signal);

	int	(*reportEvent)(struct _MuxPlugin *, CMN_PLAY_JSON_EVENT *);

	void (*destory)(struct _MuxPlugin *);	/* destory this plugin */

	struct _MuxPlugin		*next;
	struct _MuxMain		*muxMain;

	void					*priv;
}MuxPlugIn;



typedef	struct _PLUGIN_JSON_HANDLER
{
	int		type;
	char		*name;
	
	int		(*handler)(void *, struct DATA_CONN *, CMN_PLAY_JSON_EVENT *);
}PluginJSonHandler;


typedef	struct _MuxThread
{
	CmnThread 	*thread;

	struct _MuxThread	*next;
}MuxThread;



typedef struct _MuxPlayerConfig
{
	/* data structure for configuration */
	cmn_list_t				windows;	/* list of RECT_CONFIG */

	cmn_list_t				osds;	/* list of RECT_CONFIG */

	/* configuration items for player */
	int						aspectRatioWindow;
	
	char						displayFormat[CMN_NAME_LENGTH];/* like 1080P_60, etc., used to initialize display HIADP_Disp_Init */
	
	int						deepColor;
	
	int						audioVolume;
	char						drmCommand[CMN_NAME_LENGTH];
	
	SUBTITLE_FILE_LIST_T		*subtitleFiles;
	
	int						isHlsStartMode;

	int						playTimeoutLocal;	/* timeout when setMedia to play local file */
	int						playTimeoutNetwork;	/* timeout when setMedia to play network URL */

	int						timeoutErrorMsg;	/* timeout for error msg when playing failed */

	char						captureName[CMN_NAME_LENGTH];

	char						playUrl[CMN_NAME_LENGTH];
	char						urlHttp[CMN_NAME_LENGTH];
	char						urlRtsp[CMN_NAME_LENGTH];
	char						urlWmsp[CMN_NAME_LENGTH];
	char						urlRtmp[CMN_NAME_LENGTH];
	char						urlRtp[CMN_NAME_LENGTH];
	char						urlUdp[CMN_NAME_LENGTH];


	int						enableScreenDebug;	/* output log in banner OSD */
	int						keepLastFrame;		/* keep last frame after playing */

	int						enableLowDelay;		/* double buffer to minize the delay when switch between playing image and video */
	
	/* watchdog info */
	int 						enableWd;
	int						wdTimeout;

	char						configFileName[CMN_NAME_LENGTH];	/* save the configuration file where the configuration info is come from; used to save configuration info */

	void						*parent;
}MuxPlayerConfig;



typedef	struct	_CmnMuxWebConfig
{
	char		configFileName[1024];
	
	char		*error_log_name;
	char		*access_log_name;
	char		*cgi_log_name;
	int		verbose_cgi_logs;

	
	char		*server_name;
	char		*server_admin;
	char		*server_ip;
	unsigned int server_port;
	
	char		*default_vhost;
	int		maxConnections;
	int		backlog;		/* max link in listen() function */

	int		virtualhost;
	char		*vhost_root;

	char		*document_root;
	char		*user_dir;
	char		*directory_index;
	char		*default_type;	/* default MIME type */
	char		*default_charset;
	char		*dirmaker;		/* executable for directory list */
	char		*cachedir;

	char		*tempdir;

	unsigned int cgi_umask;

	char		*pid_file;
	char		*cgi_path;
	int		single_post_limit;

	char		*baseAuthen;

	char 	*server_root;
#ifdef WIN32	
	char		*server_uid;
	char		*server_gid;
#else
	uid_t	server_uid;
	gid_t	server_gid;
#endif	

	int		debug;

	unsigned int ka_timeout;	/*KeepAlive timeout*/
	unsigned int ka_max;		/* KeepAlive */

	void (*addAlias)(const char *, const char *, int);
	void (*addMimeType)(const char *, const char *);


	int use_localtime;
}CmnMuxWebConfig;


#include "mux7xx.h"


typedef	struct _MuxMain
{
	char						configFileName[CMN_NAME_LENGTH];

	MUX_BOARD_TYPE			boardType;

	int						udpCtrlPort;
	int						tcpCtrlPort;
	char						unixPort[CMN_NAME_LENGTH];

	int						isAuthen;

	int						isClientPolling;		/* polling for SDP client in RX */
	int						pollTime;			/* seconds */
	
//	CTRL_LINK_TYPE			ctrlProtocol;

	cmn_list_t				playlists;
	cmn_mutex_t				*playlistLock;  /* lock between controller and players which update or access playlist. add Jan.22nd, 2018 */

	log_stru_t				muxLog;

	HwButtonCtrl				btnCtrl;
	HwLedCtrl				ledCtrl;
	HwWatchdogCtrl			watchdogCtrl;
	HwRs232Ctrl				rs232Ctrl;


	cJSON					*systemJson;

	EXT_RUNTIME_CFG			runCfg;
	EXT_RUNTIME_CFG			rxCfg;

	/*** data structure of run-time      *******/

	CMN_FTP_CLIENT			ftpClient;
	
	MuxPlugIn				*plugins;
	MuxThread				*threads;	/* all threads */

	/* return thread ID or -1 when error */
	int 	(*initThread)(struct _MuxMain *, CmnThread *, void *);
	int 	(*reportEvent)(struct _MuxMain *, MUX_PLUGIN_TYPE , CMN_PLAY_JSON_EVENT *);

}MuxMain;



/************ Macros like functions **********************/

extern	TYPE_NAME_T jsonErrors[];

extern	TYPE_NAME_T cmnMuxPluginTypes[];
extern	TYPE_NAME_T cmnMuxRequestMethods[];

extern	const	TYPE_NAME_T	_videoColorSpaces[];
extern	const	TYPE_NAME_T	_videoFramerates[];

extern	const short	videoWidthList[];
extern	const short 	videoHeightList[];

extern	const int32_t constRs232Baudrates[];
extern	const short constRs232Databits[];

extern	const	TYPE_NAME_T	_videoFormats[];
extern	const	TYPE_NAME_T	_videoFps4Rest[];


extern	const char 	audioChannelsList[];
extern	const	TYPE_NAME_T	_audioPktSizes[];
extern	const	TYPE_NAME_T	_audioRates[];


extern	const	TYPE_NAME_T	_ipcmdStringRsParities[];
extern	const	TYPE_NAME_T	_ipcmdStringRsStopbits[];


extern	const	EXT_CONST_INT	intVideoColorDepthList[];
extern	const	EXT_CONST_INT	intVideoFpsList[];


#define	CMN_FIND_HC_STATE(type)		\
	cmnMuxTypeFindName(CMN_STR_T_HC_STATES, (type) )

#define	CMN_FIND_HC_EVENT(type)		\
	cmnMuxTypeFindName(CMN_STR_T_HC_EVENTS, (type) )

#define	CMN_FIND_STR_HC_STATE(str)		\
	cmnMuxTypeFindType(CMN_STR_T_HC_STATES, (str) )

#define	CMN_FIND_STR_HC_EVENT(str)		\
	cmnMuxTypeFindType(CMN_STR_T_HC_EVENTS, (str) )




#define	CMN_FIND_STR_RS_PARITY(str)		\
	cmnMuxTypeFindType(_ipcmdStringRsParities, (str) )

#define	CMN_FIND_RS_PARITY(type)		\
	cmnMuxTypeFindName(_ipcmdStringRsParities, (type) )


#define	CMN_FIND_STR_V_COLORSPACE(str)		\
	cmnMuxTypeFindType(_videoColorSpaces, (str) )

#define	CMN_FIND_V_COLORSPACE(type)		\
	cmnMuxTypeFindName(_videoColorSpaces, (type) )


/* fps for SDP */
#define	CMN_FIND_V_FRAME_RATE(type)		\
	cmnMuxTypeFindName(_videoFramerates, (type) )

#define	CMN_FIND_STR_V_FRAME_RATE(str)		\
	cmnMuxTypeFindType(_videoFramerates, (str) )

#define	CMN_FIND_V_FPS_4_REST(type)		\
	cmnMuxTypeFindName(_videoFps4Rest, (type) )

#define	CMN_FIND_STR_V_FPS_4_REST(str)		\
	cmnMuxTypeFindType(_videoFps4Rest, (str) )


#define	CMN_FIND_V_FORMAT(type)		\
	cmnMuxTypeFindName(_videoFormats, (type) )

#define	CMN_FIND_STR_V_FORMAT(str)		\
	cmnMuxTypeFindType(_videoFormats, (str) )



#define	CMN_FIND_A_PKTSIZE(type)		\
	cmnMuxTypeFindName(_audioPktSizes, (type) )

#define	CMN_FIND_STR_A_PKTSIZE(str)		\
	cmnMuxTypeFindType(_audioPktSizes, (str) )


#define	CMN_FIND_STR_A_RATE(str)		\
	cmnMuxTypeFindType(_audioRates, (str) )

#define	CMN_FIND_A_RATE(type)		\
	cmnMuxTypeFindName(_audioRates, (type) )

	

#define	CMN_INT_FIND_NAME_V_DEPTH(type)		\
	extCmnIntFindName(CMN_INT_T_V_DEPTH, (type) )
	
#define	CMN_INT_FIND_TYPE_V_DEPTH(name)		\
	extCmnIntFindType(CMN_INT_T_V_DEPTH, (name) )


#define	CMN_INT_FIND_NAME_V_FPS(type)		\
	cmnMuxTypeFindName(intVideoFpsList, (type) )

#define	CMN_INT_FIND_TYPE_V_FPS(name)		\
	cmnMuxTypeFindType(intVideoFpsList, (name) )





/* plugin type */
#define	CMN_MUX_FIND_PLUGIN_TYPE( name) \
		cmnMuxTypeFindType(cmnMuxPluginTypes, (name))
		
#define	CMN_MUX_FIND_PLUGIN_NAME( type) \
		cmnMuxTypeFindName(cmnMuxPluginTypes, (type))

/* request method name */
#define	CMN_MUX_FIND_METHOD_NAME( type) \
		cmnMuxTypeFindName(cmnMuxRequestMethods, (type))

#define	CMN_MUX_FIND_METHOD_TYPE( name) \
		cmnMuxTypeFindType(cmnMuxRequestMethods, (name))



#define	MUX_JSON_ERROR_STR( type)		\
			cmnMuxTypeFindName( jsonErrors, (type))



#define	SYS_PLAYLISTS(muxMain)	\
			(&(muxMain)->playlists)


#define	SYS_PLAYLIST_LOCK(muxMain)	\
		cmn_mutex_lock(muxMain->playlistLock)


#define	SYS_PLAYLIST_UNLOCK(muxMain)	\
		cmn_mutex_unlock(muxMain->playlistLock)



#define	SYS_MAIN(dataConn)		\
			( (dataConn)->ctrlConn->broker->muxMain)


/* free dataConn and its cJSON, and then free jsonEvent */
#define	MUX_PLUGIN_FSM_JSON_REPLY( jsonEvent , statusCode) \
		{ struct DATA_CONN *dataConn = (struct DATA_CONN *)(jsonEvent)->priv; \
			if(!dataConn) {CMN_ABORT("DATA_CONN is null");}	\
			 (dataConn)->errCode = (statusCode); cmnMuxJsonPluginReplay(dataConn, (jsonEvent));}



#define	MUX_PLUGIN_JSON_STATUS_MESSAGE(jsonEvent, errCode, ...)	\
	{ struct DATA_CONN *dataConn = (struct DATA_CONN *)(jsonEvent)->priv; \
		if(!dataConn) {CMN_ABORT("DATA_CONN is null");}	\
		cmnMuxJsonControllerReply( (dataConn), (errCode),  __VA_ARGS__); \
		cmnMuxJsonPluginReplay(dataConn, (jsonEvent));}


#define	JEVENT_ADD_STRING(object, name, value) \
	cJSON_AddItemToObject( (object), (name), cJSON_CreateString( value))


#define	JEVENT_ADD_INTEGER(object, name, value) \
	cJSON_AddItemToObject((object), (name), cJSON_CreateNumber( (value)))


#define	JEVENT_ADD_ARRAY(object, name, arrayObj ) \
	{ (arrayObj) = cJSON_CreateArray();	\
	cJSON_AddItemToObject((object), name, arrayObj); }



#define	SET_CGI_EVENT( _type)	\
		CMN_CLEAR_BIT( (_type), 31)

#define	SET_FSM_EVENT( _type)	\
		CMN_SET_BIT( (_type), 31)

#define	CLAER_FSM_EVENT_FLAGS( _type)	\
		CMN_CLEAR_BIT( (_type), 31)


#define	CHECK_FSM_EVENT(_type)	\
		CMN_CHECK_BIT( (_type), 31)



//cJSON *cmnMuxGetCurrentCfg(struct DATA_CONN *dataConn);

char *cmnMuxCreateErrReply(int errCode, cJSON *cmd);


int cmnMuxJsonReplyError( struct DATA_CONN *dataConn, const char *fmt, ... );



cJSON *cmnMuxClientRequest(cJSON *ipCmd);

extern	CmnThread  threadBroker;
extern	CmnThread  threadManager;
extern	CmnThread  threadSdpClient;


MuxPlugIn *cmnMuxPluginFind(MuxMain *muxMain, MUX_PLUGIN_TYPE type);

int cmnMuxJSonPluginHandle(void *priv, PluginJSonHandler *firstHandler, CMN_PLAY_JSON_EVENT *jsonEvent);
int	cmnMuxJsonPluginReplay(struct DATA_CONN *dataConn, CMN_PLAY_JSON_EVENT *jsonEvent);




char *cmnMuxTypeFindName(const TYPE_NAME_T *types, int type);
int	cmnMuxTypeFindType(const TYPE_NAME_T *types, char *name);

const uint8_t extCmnIntFindName(CMN_INT_TYPE  intType, uint8_t type);
const uint8_t extCmnIntFindType(CMN_INT_TYPE  intType, uint8_t name);


int cmnMuxMainParse(const char *filename, MuxMain *muxMain);
int cmnMuxPlayerParseConfig(const char *filename, MuxPlayerConfig *cfg);

int cmnMuxConfigParseRecord(const char *filename, CmnMuxRecordConfig *recordConfig);


int	cmnMuxRectsSave(FILE *f, cmn_list_t *winsOrOsds);
int	cmnMuxPlaylistSave(FILE *f, cmn_list_t *playlists);
int cmnMuxSavePlayerConfig( MuxPlayerConfig *cfg);
int cmnMuxSaveAllConfig(MuxMain *muxMain);


int cmnMuxJsonControllerReply(struct DATA_CONN *dataConn, int status, const char *fmt, ... );

int cmnMuxJEventReply(CMN_PLAY_JSON_EVENT *jsonEvent, int errCode, const char *fmt, ...);


#define	DATA_CONN_ERR(dataConn, errCode, ...)		\
		do{ MUX_ERROR( __VA_ARGS__);	\
		cmnMuxJsonControllerReply( (dataConn),  (errCode),  __VA_ARGS__ );}while(0)

int cmnMuxDsValidate(void);


cJSON *cmnMuxSystemJSon2Flat(cJSON *systemJson);

cJSON *cmnJsobSystemGetSubItem(cJSON *sysObj, char *item, int index);
cJSON *cmnJsonSystemFindObject(MuxMain *muxMain, const char*objName );

cJSON *cmnMuxJsonLoadConfiguration(char *cfgFileName);

#endif

