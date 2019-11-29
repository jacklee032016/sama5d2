
 #define _GNU_SOURCE  
 #include <string.h>

#include "libCmn.h"
#include "mux7xx.h"
#include "libMux.h"


TYPE_NAME_T jsonErrors[] =
{
	{
		.type = IPCMD_ERR_IN_PROCESSING,
		.name = "InProcess",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_NOERROR,
		.name = "NoError",
		.value = NULL,
	},


	/* 4xx */
	{
		.type = IPCMD_ERR_BAD_REQUEST,
		.name = "Bad Request",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_UNAUTHORIZED,
		.name = "Unauthorized",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_FORBIDDEN,
		.name = "Forbidden",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_NOT_FOUND,
		.name = "Not Found",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_METHOD_NOT_ALLOWED,
		.name = "Method Not Allowed",
		.value = NULL,
	},


	/* 5xx, service error */
	{
		.type = IPCMD_ERR_COMMUNICATION,
		.name = "Error communication",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_JSON_CORRUPTED,
		.name = "CORRUPTED-JSON-DATA-RECEIVED",
		.value = NULL,
	},
	
	{
		.type = IPCMD_ERR_MAC_MISMATCH,
		.name = "MAC-MISMATCH",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_CRC_FAIL,
		.name = "CRC-FAILED",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_WRONG_PROTOCOL,
		.name = "WRONG_PROTOCOL",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_DATA_ERROR,
		.name = "Data Item Error",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_NOT_SUPPORT_COMMND,
		.name = "Unknown Command",
		.value = NULL,
	},
	
	{
		.type = IPCMD_ERR_IS_BUSY_NOW,
		.name = "Unknown Command",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_SERVER_INTERNEL_ERROR,
		.name = "Server internal error",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_PLUGIN_NOT_EXISTS,
		.name = "Module not exist or not load",
		.value = NULL,
	},

	{
		.type = IPCMD_ERR_PLUGIN_PLAYER_TIMEOUT,
		.name = "Timeout when try to play",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_PLUGIN_PLAYER_FAILED,
		.name = "Play fails, media URL is wrong or invalidate format",
		.value = NULL,
	},
	
	{
		.type = IPCMD_ERR_FTP_SERVER_ERROR,
		.name = "can't connect to server",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_FTP_USER_PASSWORD,
		.name = "username or password is wrong",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_FTP_NO_FREESPACE,
		.name = "no space in RX, can't make directory for the file",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_FTP_NAME_COLLISION,
		.name = "name collision, filename is a local file or local directory",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_FTP_DATA_ERROR,
		.name = "failed when get data from remote file or save data as local file",
		.value = NULL,
	},
	{
		.type = IPCMD_ERR_FTP_PARTLY_FAILED,
		.name = "some files failed, some successed in file list",
		.value = NULL,
	},

		
	{
		.type = IPCMD_ERR_UNKNOWN,
		.name = "Unknown Command",
		.value = NULL,
	},
	
	{
		.type = -1,
		.name = NULL,
		.value = NULL,
	},
};


typedef	struct	_CfgTypes
{
	int		type;
	char		*name;
}CfgType;

const TYPE_NAME_T _sdpcStateTypes[] =
{
	{
		.type = SDPC_STATE_WAIT,
		.name = "WAIT",	
		.value = NULL,
	},
	{
		.type = SDPC_STATE_CONNECTED,
		.name = "CONNECTED",	
		.value = NULL,
	},
	{
		.type = SDPC_STATE_DATA,
		.name = "DATA",	
		.value = NULL,
	},
	{
		.type = SDPC_STATE_ERROR,
		.name = "ERROR",	
		.value = NULL,
	},
	{
		.type = -1,
		.name = NULL,	
		.value = NULL,
	}
};

const TYPE_NAME_T _sdpcEventTypes[] =
{
	{
		.type = SDPC_EVENT_NEW,
		.name = "New",	
		.value = NULL,
	},
	{
		.type = SDPC_EVENT_CONNECTED,
		.name = "Connected",	
		.value = NULL,
	},
	{
		.type = SDPC_EVENT_RECV,
		.name = "Recv",	
		.value = NULL,
	},
	{
		.type = SDPC_EVENT_SENT,
		.name = "Sent",	
		.value = NULL,
	},
	{
		.type = SDPC_EVENT_CLOSE,
		.name = "Closed",	
		.value = NULL,
	},
	{
		.type = SDPC_EVENT_TIMEOUT,
		.name = "Timeout",	
		.value = NULL,
	},
	{
		.type = SDPC_EVENT_ERROR,
		.name = "Error",	
		.value = NULL,
	},
	{
		.type = -1,
		.name = NULL,	
		.value = NULL,
	}
};


TYPE_NAME_T cmnMuxPluginTypes[] =
{
	{
		.type = MUX_PLUGIN_TYPE_MAIN,
		.name = "Main",	
		.value = NULL,
	},
	{
		.type = MUX_PLUGIN_TYPE_PLAYER,
		.name = "PLAYER",	
		.value = NULL,
	},
	{
		.type = MUX_PLUGIN_TYPE_SERVER,
		.name = "SERVER",	
		.value = NULL,
	},
	{
		.type = MUX_PLUGIN_TYPE_WEB,
		.name = "WEB",	
		.value = NULL,
	},
	{
		.type = MUX_PLUGIN_TYPE_RECORDER,
		.name = "RECORDER",	
		.value = NULL,
	},
	{
		.type = -1,
		.name = NULL,	
		.value = NULL,
	}
};

TYPE_NAME_T cmnMuxRequestMethods[] =
{
	{
		.type = CMN_JSON_METHOD_GET,
		.name = CMN_MEHOD_STR_GET,	
		.value = NULL,
	},
	{
		.type = CMN_JSON_METHOD_POST,
		.name = CMN_MEHOD_STR_POST,	
		.value = NULL,
	},
	{
		.type = CMN_JSON_METHOD_PUT,
		.name = CMN_MEHOD_STR_PUT,	
		.value = NULL,
	},
	{
		.type = CMN_JSON_METHOD_DELETE,
		.name = CMN_MEHOD_STR_DELETE,	
		.value = NULL,
	},
	{
		.type = CMN_JSON_METHOD_PATCH,
		.name = CMN_MEHOD_STR_PATCH,	
		.value = NULL,
	},
	{
//		.type = CMN_JSON_METHOD_INVALIDATE,
		.type = -1,
		.name = NULL,	
		.value = NULL,
	}
};


const	TYPE_NAME_T	_ipcmdStringRsParities[] =
{
	{
		type	: EXT_RS232_PARITY_NONE,
		name : EXT_RS232_PAR_STR_NONE,
		value  :  NULL,
	},
	{
		type	: EXT_RS232_PARITY_ODD,
		name : EXT_RS232_PAR_STR_ODD,
		value : NULL,
	},
	{
		type	: EXT_RS232_PARITY_EVEN,
		name : EXT_RS232_PAR_STR_EVEN,
		value : NULL,
	},
	{
		type	: -1,
		name : NULL,
		value : NULL,
	}
};

const	TYPE_NAME_T	_ipcmdStringRsStopbits[] =
{
	{
		type	: EXT_RS232_STOP_BITS_1,
		name	: "1",
		value : NULL,
	},
	{
		type	: EXT_RS232_STOP_BITS_2,
		name	: "2",
		value : NULL,
	},
#if 0	
	{
		type	: EXT_RS232_STOP_BITS_15,
		name	: "1.5",
		value : NULL
	},
#endif	
	{
		type	: -1,
		name : NULL,
		value : NULL,
	}
};

const	TYPE_NAME_T _mediaModes[] =
{
	{
		.type = FPGA_CFG_AUTO,
		.name = EXT_WEB_CFG_FIELD_FPGA_AUTO_V_AUTO,	
		.value = NULL,
	},
	{
		.type = FPGA_CFG_MANUAL,
		.name = EXT_WEB_CFG_FIELD_FPGA_AUTO_V_MANUAL,	
		.value = NULL,
	},
	{
		.type = FPGA_CFG_SDP,
		.name = EXT_WEB_CFG_FIELD_FPGA_AUTO_V_SDP,	
		.value = NULL,
	},
	{
		.type = -1,
		.name = NULL,	
		.value = NULL,
	}
};


const	TYPE_NAME_T	_videoFormats[] =
{
	{
		type	: EXT_VIDEO_INTLC_INTERLACED,
		name	: "Interlaced",
		value : NULL,
	},
	{
		type	: EXT_VIDEO_INTLC_A_PROGRESSIVE,
		name	: "Progressive(Level A)",
		value : NULL,
	},
	{
		type	: EXT_VIDEO_INTLC_B_PROGRESSIVE,
		name	: "Progressive(Level B)",
		value : NULL,
	},
	{
		type	: -1,
		name	: NULL,
		value : NULL,
	}
};


const	TYPE_NAME_T	_videoColorSpaces[] =
{/* from page 17 of specs SAMPTE ST 2110-20:2017 */
#if (MUX_BOARD == MUX_BOARD_767 )
	{
		.type = EXT_V_COLORSPACE_YCBCR_422,
		.name = "YCbCr-4:2:2",
		.value = NULL,
	},
	{
		.type = EXT_V_COLORSPACE_YCBCR_444,
		.name = "YCbCr-4:4:4",
		.value = NULL,
	},
	
	{
		.type = EXT_V_COLORSPACE_RGB,
		.name = "RGB",
		value : NULL,
	},
#if 0
	
	{
		type	: EXT_V_COLORSPACE_YCBCR_420,
		name	: "YCbCr-4:2:0",
		value : NULL,
	},
	{
		type	: EXT_V_COLORSPACE_XYZ,
		name	: "XYZ",
		value : NULL,
	},
	{
		type	: EXT_V_COLORSPACE_KEY,
		name	: "KEY",
		value : NULL,
	},
	{
		type	: EXT_V_COLORSPACE_CL_YCBCR_422,
		name	: "CLYCbCr-4:2:2",
		value : NULL,
	},
	{
		type	: EXT_V_COLORSPACE_CL_YCBCR_444,
		name	: "CLYCbCr-4:4:4",
		value : NULL,
	},

	{
		type	: EXT_V_COLORSPACE_CL_YCBCR_420,
		name	: "CLYCbCr-4:2:0",
		value : NULL,
	},
#endif		
#elif (MUX_BOARD == MUX_BOARD_774)

	{
		.type = EXT_V_COLORSPACE_YCBCR_422,
		.name = "YCbCr-4:2:2",
		.value = NULL,
	},
	{
		.type = EXT_V_COLORSPACE_YCBCR_444,
		.name = "YCbCr-4:4:4",
		.value = NULL,
	},
	
	{
		.type = EXT_V_COLORSPACE_RGB,
		.name = "RGB",
		value : NULL,
	},

	{
		.type = EXT_V_COLORSPACE_YCBCR_420,
		.name = "YCbCr-4:2:0",
		.value = NULL,
	},
	{
		.type = EXT_V_COLORSPACE_XYZ,
		.name = "XYZ",
		.value = NULL,
	},
	{
		.type = EXT_V_COLORSPACE_KEY,
		.name = "KEY",
		.value = NULL,
	},
	{
		.type = EXT_V_COLORSPACE_CL_YCBCR_422,
		.name = "CLYCbCr-4:2:2",
		.value = NULL,
	},
	{
		.type = EXT_V_COLORSPACE_CL_YCBCR_444,
		.name = "CLYCbCr-4:4:4",
		.value = NULL,
	},

	{
		.type = EXT_V_COLORSPACE_CL_YCBCR_420,
		.name = "CLYCbCr-4:2:0",
		.value = NULL,
	},

/*
	{
		type	: EXT_V_COLORSPACE_YCBCR_444,
		name	: "YCbCr-4:4:4",
		value : NULL,
	},
	{
		type	: EXT_V_COLORSPACE_YCBCR_422,
		name	: "YCbCr-4:2:2",
		value : NULL,
	},
	{
		type	: EXT_V_COLORSPACE_YCBCR_420,
		name	: "YCbCr-4:2:0",
		value : NULL,
	},
	{
		type	: EXT_V_COLORSPACE_YCBCR_RAW,
		name	: "YCbCr-RAW",
		value : NULL,
	},
	
	{
		type	: EXT_V_COLORSPACE_RESERVED,
		name	: "Reserved",
		value : NULL,
	},
	
	{
		type	: EXT_V_COLORSPACE_Y4K_420,
		name	: "Y4K-4:2:0",
		value : NULL,
	},
	{
		type	: EXT_V_COLORSPACE_RGB_RAW,
		name	: "RGB-RAW",
		value : NULL,
	},
*/

#else
#error 	Not support board definition
#endif
	{
		type	: -1,
		name	: NULL,
		value : NULL,
	}
};


const	TYPE_NAME_T	_videoFramerates[] =
{
	{
		type	: EXT_V_FRAMERATE_T_23,
		name	: "24000/1001",
		value : NULL,
	},
	{
		type	: EXT_V_FRAMERATE_T_24,
		name	: "24",
		value : NULL,
	},

	{
		type	: EXT_V_FRAMERATE_T_25,
		name	: "25",
		value : NULL,
	},
	{
		type	: EXT_V_FRAMERATE_T_29,
		name	: "30000/1001",
		value : NULL,
	},

	{
		type	: EXT_V_FRAMERATE_T_30,
		name	: "30",
		value : NULL,
	},
	
	{
		type	: EXT_V_FRAMERATE_T_50,
		name	: "50",
		value : NULL,
	},
	
	{
		type	: EXT_V_FRAMERATE_T_59,
		name	: "60000/1001",
		value : NULL,
	},
	
	{
		type	: EXT_V_FRAMERATE_T_60,
		name	: "60",
		value : NULL,
	},
	
	{
		type	: -1,
		name	: NULL,
		value : NULL,
	}
};



const	TYPE_NAME_T	_videoFps4Rest[] =
{
	{
		type	: EXT_V_FRAMERATE_T_23,
		name	: "23.98",
		value : NULL,
	},
	{
		type	: EXT_V_FRAMERATE_T_24,
		name	: "24",
		value : NULL,
	},

	{
		type	: EXT_V_FRAMERATE_T_25,
		name	: "25",
		value : NULL,
	},
	{
		type	: EXT_V_FRAMERATE_T_29,
		name	: "29.97",
		value : NULL,
	},

	{
		type	: EXT_V_FRAMERATE_T_30,
		name	: "30",
		value : NULL,
	},
	
	{
		type	: EXT_V_FRAMERATE_T_50,
		name	: "50",
		value : NULL,
	},
	
	{
		type	: EXT_V_FRAMERATE_T_59,
		name	: "59.94",
		value : NULL,
	},
	
	{
		type	: EXT_V_FRAMERATE_T_60,
		name	: "60",
		value : NULL,
	},
	
	{
		type	: -1,
		name	: NULL,
		value : NULL,
	}
};


const	TYPE_NAME_T	_audioPktSizes[] =
{
	{
		type	: EXT_A_PKT_SIZE_1MS,
		name	: "1ms",
		value : NULL,
	},
	{
		type	: EXT_A_PKT_SIZE_125US,
		name	: "125us",
		value : NULL,
	},
	{
		type	: -1,
		name	: NULL,
		value : NULL,
	}
};


const	TYPE_NAME_T	_audioRates[] =
{
#if 0
	{
		type	: EXT_A_RATE_32K,
		name	: "32000",
		value : NULL,
	},
#endif	
	{
		type	: EXT_A_RATE_44K,
		name	: "44100",
		value : NULL,
	},
	{
		type	: EXT_A_RATE_48K,
		name	: "48000",
		value : NULL,
	},
#if 0	
	{
		type	: EXT_A_RATE_88K,
		name	: "88200",
		value : NULL,
	},
#endif	
	{
		type	: EXT_A_RATE_96K,
		name	: "96000",
		value : NULL,
	},
#if 0	
	{
		type	: EXT_A_RATE_176K,
		name	: "176400",
		value : NULL,
	},
	{
		type	: EXT_A_RATE_192K,
		name	: "192000",
		value : NULL,
	},
#endif

/*
	{
		type	: EXT_A_RATE_768K,
		name	: "768000",
		value : NULL,
	},
*/	
	{
		type	: -1,
		name	: NULL,
		value : NULL,
	}
};

const TYPE_NAME_T _strTypes[] =
{
	{
		type	: CMN_STR_T_RS_PARITY,
		name	: "RS232Parity",
		value : NULL,
	},
	{
		type	: CMN_STR_T_V_COLORSPACE,
		name	: "ColorSpace",
		value : NULL,
	},
	{
		type	: CMN_STR_T_V_FRAME_RATE,
		name	: "VideoFps",
		value : NULL,
	},

	{
		type	: CMN_STR_T_HTTP_STATES,
		name	: "sState",
		value : NULL,
	},
	{
		type	: CMN_STR_T_HTTP_EVENTS,
		name	: "sEvent",
		value : NULL,
	},
	{
		type	: CMN_STR_T_HC_STATES,
		name	: "ClientState",
		value : NULL,
	},
	{
		type	: CMN_STR_T_HC_EVENTS,
		name	: "ClientEvent",
		value : NULL,
	},
	{
		type	: CMN_STR_T_A_PKTSIZE,
		name	: "AudioPktSize",
		value : NULL,
	},
	{
		type	: CMN_STR_T_A_RATE,
		name	: "AudoRate",
		value : NULL,
	},

	{
		type	: -1,
		name	: NULL,
		value : NULL,
	}
};




const short	videoWidthList[]=
{
//	480,
//	576,	
//	640,
	720,
//	800,
	960,
//	1024,
	1280,
//	1360,
//	1440,
//	1600,
//	1680,
	1920,
//	2048,
//	2560,
//	3840,
//	4096,
	0
};

const short videoHeightList[]=
{
	480,
//	600,
	576,
	720,
//	768,
//	800,
//	1024,
//	1050,
	1080,
//	1200,
//	1440,
//	1600,
//	2160,
	0
};

const int32_t constRs232Baudrates[]=
{
	EXT_BAUDRATE_9600,
	EXT_BAUDRATE_19200,
	EXT_BAUDRATE_38400,
	EXT_BAUDRATE_57600,
	EXT_BAUDRATE_115200,
	0
};


const short constRs232Databits[]=
{
#if 0
	EXT_RS232_CHAR_LENGTH_5,
	EXT_RS232_CHAR_LENGTH_6,
#endif	
	EXT_RS232_CHAR_LENGTH_7,
	EXT_RS232_CHAR_LENGTH_8,
	0
};


const char audioChannelsList[] =
{
	4,	
	8,
	12,
	16,	
	0
};


int	cmnMuxTypeFindType(const TYPE_NAME_T *types, char *name)
{
	const TYPE_NAME_T *tmp = types;

	while(tmp->type != -1 && tmp->name != NULL ) 
	{
		//if( !strcasecmp(tmp->name, name) )
		if( strcasestr(tmp->name, name) )
		{		
			return tmp->type;
		}
		tmp++;
	}

#ifndef   __CMN_RELEASE__
	{
//		CMN_ABORT("Type of name %s is not found, the first item is :%s", name, types->name);
		MUX_ERROR("Type of name %s is not found, the first item is :%d(%s)", name, types->type, types->name);
	}
#endif
	return INVALIDATE_VALUE_U32;
}


char *cmnMuxTypeFindName(const TYPE_NAME_T *types, int type)
{
	const TYPE_NAME_T *tmp = types;

	while(tmp->type != -1 && tmp->name != NULL ) 
	{
		if( tmp->type == type )
		{		
			return tmp->name;
		}
		tmp++;
	}


#ifndef   __CMN_RELEASE__
	{
//		CMN_ABORT("Type %d is not found, the first item is :%s", type, types->name);
		MUX_ERROR("Type %d is not found, the first item is :%d(%s)", type, types->type, types->name);
	}
//	return NULL;
	return "Unknown";
#endif
	return "Unknown";
}

unsigned int cmnMuxCRC32b(void *message, int len)
{
	int i, j;
	unsigned int byte, crc, mask;
	char * tmp = (char*) message;
	i = 0;
	crc = 0xFFFFFFFF;
	
	//while (tmp[i] != 0)
	for (i=0; i< len; i++)	
	{
		byte = tmp[i];            // Get next byte.
		crc = crc ^ byte;
		for (j = 7; j >= 0; j--)
		{    // Do eight times.
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		
//		i = i + 1;
	}
	
	return ~crc;
}

MuxPlugIn *cmnMuxPluginFind(MuxMain *muxMain, MUX_PLUGIN_TYPE type)
{
	MuxPlugIn *plugin = muxMain->plugins;

	while(plugin)
	{
		if(plugin->type == type)
		{
			return plugin;
		}
		
		plugin = plugin->next;
	}

	return NULL;
}

void extBcd2Ascii(unsigned char src, char *dest)
{ 
	static const char outputs[] = "0123456789ABCDEF";
	
	*dest++ = outputs[src>>4];
	*dest++ = outputs[src&0xf];
	*dest = '\0';
}

int extSysAtoInt8(const char *str, unsigned char *value)
{
	char key;
	unsigned int dw = 0;

	*value = 0;
	for (dw = 0; dw < 2; dw++)
	{
		key = *str;

		if (key >= '0' && key <= '9')
		{
			*value = (*value * 16) + (key - '0');
		}
		else
		{
			if (key >= 'A' && key <= 'F')
			{
				*value = (*value * 16) + (key - 'A' + 10);
			}
			else
			{
				if (key >= 'a' && key <= 'f')
				{
					*value = (*value * 16) + (key - 'a' + 10);
				}
				else
				{
					printf("'%c' is not a hexa character!\n\r", key);
					return EXIT_FAILURE;
				}
			}
		}

		str++;
	}
	
//	printf("'%2x' \n\r", value);
	return EXIT_SUCCESS;
}


/*mac address in string format of "xx:xx:xx:xx:xx:xx" into structure */
int	extMacAddressParse(EXT_MAC_ADDRESS *macAddress, const char *macStr)
{
	const char *tmp = macStr;
	int i;

	for(i=0; i< EXT_MAC_ADDRESS_LENGTH; i++)
	{
		if(i != 0)
		{
			tmp = strchr(tmp, ':');
			if(tmp==NULL)
			{
				return EXIT_FAILURE;
			}
			tmp++;
		}
		
		if( tmp )
		{
			if( extSysAtoInt8(tmp, &macAddress->address[i]) )
			{
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}


const	EXT_CONST_INT	intVideoColorDepthList[] =
{
	{
		type	: EXT_V_DEPTH_8,
		name	: 8
	},
	{
		type	: EXT_V_DEPTH_10,
		name	: 10
	},
	{
		type	: EXT_V_DEPTH_12,
		name	: 12
	},
	{
		type	: EXT_V_DEPTH_16,
		name	: 16
	},
	{
		type	: 0xFF,
		name	: 0xFF
	}
};

const	EXT_CONST_INT	intVideoFpsList[] =
{
	{
		type	: EXT_V_FRAMERATE_T_23,
		name	: 23
	},
	{
		type	: EXT_V_FRAMERATE_T_24,
		name	: 24
	},
	{
		type	: EXT_V_FRAMERATE_T_25,
		name	: 25
	},
	{
		type	: EXT_V_FRAMERATE_T_29,
		name	: 29
	},
	{
		type	: EXT_V_FRAMERATE_T_30,
		name	: 30
	},
	{
		type	: EXT_V_FRAMERATE_T_50,
		name	: 50
	},
	{
		type	: EXT_V_FRAMERATE_T_59,
		name	: 59
	},
	{
		type	: EXT_V_FRAMERATE_T_60,
		name	: 60
	},
	{
		type	: 0xFF,
		name	: 0xFF
	}
};


const uint8_t extCmnIntFindName(CMN_INT_TYPE  intType, uint8_t type)
{
	const EXT_CONST_INT *_str;

	switch(intType)
	{
		case CMN_INT_T_V_DEPTH:
			_str = intVideoColorDepthList;
			break;
		case CMN_INT_T_V_FPS:
			_str = intVideoFpsList;
			break;

		default:
			return 0xFF;
			break;
	}

	while(_str->type!= 0xFF)
	{
		if(_str->type == type)
		{
			return _str->name;
		}

		_str++;
	}

	EXT_ERRORF("unknown type %d in constant type :%d", type, intType);
	return 0xFF;
}


const uint8_t extCmnIntFindType(CMN_INT_TYPE  intType, uint8_t name)
{
	const EXT_CONST_INT *_str;

	switch(intType)
	{
		case CMN_INT_T_V_DEPTH:
			_str = intVideoColorDepthList;
			break;
		case CMN_INT_T_V_FPS:
			_str = intVideoFpsList;
			break;

		default:
			return -1;
			break;
	}

	while(_str->type!= 0xFF)
	{
		if(_str->name == name) 
		{
			return _str->type;
		}

		_str++;
	}
	
	EXT_ERRORF("unknown type %d in constant type :%d", name, intType);
	return 0xFF;
}


const MediaParam constMediaParams[] = 
{
	/* 720 */
	{/* 1 */
		index: 0,
		desc	:	"720p23.98",
		width:	VIDEO_WIDTH_1280,
		height:	VIDEO_HEIGHT_720,
		fps	:	EXT_V_FRAMERATE_T_23,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 2 */
		index: 1,
		desc	:	"720p24",
		width:	VIDEO_WIDTH_1280,
		height:	VIDEO_HEIGHT_720,
		fps	:	EXT_V_FRAMERATE_T_24,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 3 */
		index: 2,
		desc	:	"720p25",
		width:	VIDEO_WIDTH_1280,
		height:	VIDEO_HEIGHT_720,
		fps	:	EXT_V_FRAMERATE_T_25,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 4 */
		index: 3,
		desc	:	"720p29.97",
		width:	VIDEO_WIDTH_1280,
		height:	VIDEO_HEIGHT_720,
		fps	:	EXT_V_FRAMERATE_T_29,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/*5 */
		index: 4,
		desc	:	"720p30",
		width:	VIDEO_WIDTH_1280,
		height:	VIDEO_HEIGHT_720,
		fps	:	EXT_V_FRAMERATE_T_30,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 6 */
		index: 5,
		desc	:	"720p50",
		width:	VIDEO_WIDTH_1280,
		height:	VIDEO_HEIGHT_720,
		fps	:	EXT_V_FRAMERATE_T_50,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 7 */
		index: 6,
		desc	:	"720p59.94",
		width:	VIDEO_WIDTH_1280,
		height:	VIDEO_HEIGHT_720,
		fps	:	EXT_V_FRAMERATE_T_59,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 8 */
		index: 7,
		desc	:	"720p60",
		width:	VIDEO_WIDTH_1280,
		height:	VIDEO_HEIGHT_720,
		fps	:	EXT_V_FRAMERATE_T_60,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},


	/* 1080 */
	{/* 1 */
		index: 8,
		desc	:	"1080p23.98",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_23,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 2 */
		index: 9,
		desc	:	"1080p24",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_24,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 3 */
		index: 10,
		desc	:	"1080p25",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_25,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 4 */
		index: 11,
		desc	:	"1080p29.97",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_29,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/*5 */
		index: 12,
		desc	:	"1080p30",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_30,
		interlaced 	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 6 */
		index: 13,
		desc	:	"1080i50",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
//		fps	:	EXT_V_FRAMERATE_T_50,
		fps	:	EXT_V_FRAMERATE_T_25,
		interlaced	: EXT_VIDEO_INTLC_INTERLACED
	},
	{/* 7 */
		index: 14,
		desc	:	"1080i59.94",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
//		fps	:	EXT_V_FRAMERATE_T_59,
		fps	:	EXT_V_FRAMERATE_T_29,
		interlaced	: EXT_VIDEO_INTLC_INTERLACED
	},
	{/* 8 */
		index: 15,
		desc	:	"1080i60",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
//		fps	:	EXT_V_FRAMERATE_T_60,
		fps	:	EXT_V_FRAMERATE_T_30,
		interlaced	: EXT_VIDEO_INTLC_INTERLACED
	},


	/* 1080 3GA */
	{/* 1 */
		index: 16,
		desc	:	"3GA 1080p50",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_50,
		interlaced	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 2 */
		index: 17,
		desc	:	"3GA 1080p59.94",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_59,
		interlaced	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 3 */
		index: 18,
		desc	:	"3GA 1080p60",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_60,
		interlaced	: EXT_VIDEO_INTLC_A_PROGRESSIVE
	},
	{/* 4 */
		index: 19,
		desc	:	"3GB 1080p50",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_50,
		interlaced	: EXT_VIDEO_INTLC_B_PROGRESSIVE
	},
	{/*5 */
		index: 20,
		desc	:	"3GB 1080p59.94",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_59,
		interlaced	: EXT_VIDEO_INTLC_B_PROGRESSIVE
	},
	{/* 6 */
		index: 21,
		desc	:	"3GB 1080p60",
		width:	VIDEO_WIDTH_1920,
		height:	VIDEO_HEIGHT_1080,
		fps	:	EXT_V_FRAMERATE_T_60,
		interlaced	: EXT_VIDEO_INTLC_B_PROGRESSIVE
	},


	{
		index: INVALIDATE_VALUE_U32,
		desc	:	NULL,
		width:	INVALIDATE_VALUE_U16,
		height:	INVALIDATE_VALUE_U16,

		fps	:	INVALIDATE_VALUE_U8,

		interlaced	: INVALIDATE_VALUE_U8
	}

};


const MediaParam *extCmnVideoParamFind(EXT_RUNTIME_CFG *runCfg)
{
	const MediaParam *videoP = constMediaParams;

	while(videoP->desc != NULL)
	{
		if(videoP->width == runCfg->runtime.vWidth && videoP->height == runCfg->runtime.vHeight &&
			videoP->fps == runCfg->runtime.vFrameRate && videoP->interlaced == runCfg->runtime.vIsInterlaced )
		{
			return videoP;
		}
		videoP ++;
	}

	EXT_ERRORF("Video Param: 'w:%d; h:%d; fps:%d; interlace:%d' not found", runCfg->runtime.vWidth, runCfg->runtime.vHeight, runCfg->runtime.vFrameRate, runCfg->runtime.vIsInterlaced);

	return NULL;
}

char extCmnVideoParamPopulate(EXT_RUNTIME_CFG *runCfg, uint8_t index)
{
//	const MediaParam *videoP = constMediaParams;

	if(index >= EXT_ARRAYSIZE(constMediaParams) )
	{
		EXT_ERRORF("Index %d is out of the range %d of params", index, (int)EXT_ARRAYSIZE(constMediaParams) );
		return EXIT_FAILURE;
	}

	runCfg->runtime.vWidth = constMediaParams[index].width;
	runCfg->runtime.vHeight = constMediaParams[index].height;
	runCfg->runtime.vFrameRate = constMediaParams[index].fps;
	runCfg->runtime.vIsInterlaced = constMediaParams[index].interlaced;

	return EXIT_SUCCESS;
}


