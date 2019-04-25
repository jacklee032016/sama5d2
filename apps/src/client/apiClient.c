/*
* This is a command line API client, mainly used in crond task to play media based on time
*/


#include "apiClient.h"

/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
*/

/*
send JSON request from json file and parse JSON response, used in CGI and other client, such as controller 811
*/


static void usage(char* base, struct API_PARAMETERS *params)
{
	printf("%s: \n\tCommand line interface for JSON API.\n"\
		"\t%s -c command -a ipaddress/fqdn -m MAC(FF:FF:FF:FF:FFFF) -p 0(UDP)|1(TCP, default) -b port(3600)  -d jsonData -u user(admin) -w passwd(admin)\n"\
		"\t\t Current command:  " \
		"\n\t\t\t"CLIENT_CMD_STR_FIND", "CLIENT_CMD_STR_GET", "CLIENT_CMD_STR_SET", "CLIENT_CMD_STR_RS_DATA", " \
		"\n\t\t\t"CLIENT_CMD_STR_SECURE", " CLIENT_CMD_STR_BLINK", "
		"\n\t\t\t"IPCMD_SYS_ADMIN_THREADS ", " IPCMD_SYS_ADMIN_VER_INFO", quit \n" \
		"\t\t ipaddress/fqdn: default localhost; \n", 
		  base, base);

	exit(-1);
}

struct API_CLIENT_CMD_HANDLER;

typedef	struct API_CLIENT_CMD_HANDLER
{
	char		*name;
	char		*ipCmd;

	/* validate parameters of this command. return 0: success; others: fail */
	int	(*validate)(struct API_CLIENT_CMD_HANDLER *, struct API_PARAMETERS *, char *);

	/* execute this command */
	int	(*execute)(struct API_CLIENT_CMD_HANDLER *,  struct API_PARAMETERS *);
}API_CLIENT_CMD_HANDLER;



static cJSON *_ipCmdRequest(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	cJSON *dataArray = NULL;
	cJSON *ipCmd = cJSON_CreateObject();
	
	if(ipCmd == NULL)
	{
		MUX_ERROR("No memory for JSON IpCmd Object");
		return NULL;
	}
	
	cJSON_AddStringToObject(ipCmd, IPCMD_NAME_KEYWORD_TARG, params->macAddress);
	cJSON_AddStringToObject(ipCmd, IPCMD_NAME_KEYWORD_CMD, handle->ipCmd);
	cJSON_AddStringToObject(ipCmd, IPCMD_NAME_KEYWORD_LOGIN_ACK, params->user);
	cJSON_AddStringToObject(ipCmd, IPCMD_NAME_KEYWORD_PWD_MSG,  params->passwd);
	
	dataArray = cJSON_CreateArray();
	cJSON_AddItemToArray(dataArray, params->dataObj);
	cJSON_AddItemToObject(ipCmd, IPCMD_NAME_KEYWORD_DATA, dataArray);
	
	params->result = cmnMuxClientRequest(ipCmd);
	return params->result;
}

static int _validateMac(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params, char *Program)
{
	EXT_MAC_ADDRESS macAddress;
	
	if(extMacAddressParse(&macAddress, params->macAddress)==EXIT_FAILURE)
	{
		printf("WARNS: '%s' is not validate MAC address\n", params->macAddress);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

static int _validateData(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params, char *Program)
{
	if(params->dataObj == NULL )
	{
		printf("WARNS: No JSON data is input\n");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}


static int _findCmdValidate(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params, char *program)
{
	snprintf(params->address,  sizeof(params->address), "%s",  UDP_BOARD_ADDRESS);
	params->protocol = PROTOCOL_UDP;
	params->port = UDP_SERVER_PORT;

	return EXIT_SUCCESS;
}


static int _findCmdExec(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	if( _ipCmdRequest(handle, params) )
	{
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

static int _getCmdValidate(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params, char *program)
{
	if(_validateMac(handle, params, program) == EXIT_FAILURE )
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}


static int _getCmdExec(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	if( _ipCmdRequest(handle, params) )
	{
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


static int _setCmdValidate(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params, char *program)
{
	if(_validateMac(handle, params, program) == EXIT_FAILURE )
		return EXIT_FAILURE;

	if(_validateData(handle, params, program) == EXIT_FAILURE )
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}


static int _setCmdExec(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	if( _ipCmdRequest(handle, params) )
	{
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

static int _rs232CmdValidate(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params, char *program)
{
	if(_validateMac(handle, params, program) == EXIT_FAILURE )
		return EXIT_FAILURE;

	if(_validateData(handle, params, program) == EXIT_FAILURE )
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}


static int _rs232CmdExec(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	if( _ipCmdRequest(handle, params) )
	{
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


/* commands for WEB */	
static int _clientWebInfosExec(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	params->result = NULL;// muxApiWebInfos();
	return EXIT_SUCCESS;
}
	
static int _clientSysAdminThreadsExec(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	params->result = NULL;//muxApiSysAdminThreads();
	return EXIT_SUCCESS;
}
	
static int _clientSysAdminVerInfoExec(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	params->result = NULL;//muxApiSysAdminVerInfo();
	return EXIT_SUCCESS;
}

static int _quitExec(struct API_CLIENT_CMD_HANDLER *handle, struct API_PARAMETERS *params)
{
	params->result = NULL;//muxApiPlayMediaQuit();
	return EXIT_SUCCESS;
}


API_CLIENT_CMD_HANDLER apiClientCmdHandlers[]=
{
	/* PLAY commands */
	{
		.name = CLIENT_CMD_STR_FIND,
		.ipCmd = 	IPCMD_NAME_GET_PARAM,
		.validate = _findCmdValidate,
		.execute = _findCmdExec
	},
	{
		.name = CLIENT_CMD_STR_GET,
		.ipCmd = 	IPCMD_NAME_GET_PARAM,
		.validate = _getCmdValidate,
		.execute = _getCmdExec
	},
	{
		.name = CLIENT_CMD_STR_SET,
		.ipCmd = 	IPCMD_NAME_SET_PARAM,
		.validate = _setCmdValidate,
		.execute = _setCmdExec
	},
	
	{
		.name = CLIENT_CMD_STR_RS_DATA,
		.ipCmd = 	IPCMD_NAME_SEND_RS232,
		.validate = _rs232CmdValidate,
		.execute = _rs232CmdExec
	},
	
	{
		.name = CLIENT_CMD_STR_SECURE,
		.ipCmd = 	IPCMD_NAME_SECURITY_CHECK,
		.validate = _setCmdValidate,
		.execute = _findCmdExec
	},
	{
		.name = CLIENT_CMD_STR_BLINK,
		.ipCmd = 	IPCMD_NAME_BLINK_LED,
		.validate = _setCmdValidate,
		.execute = _findCmdExec
	},
	
	/* commands for Sys Admin */
	{
		.name = IPCMD_SYS_ADMIN_THREADS,
		.ipCmd = 	IPCMD_NAME_GET_PARAM,
		.validate = NULL,
		.execute = _clientSysAdminThreadsExec
	},
	
	{
		.name = IPCMD_SYS_ADMIN_VER_INFO,
		.ipCmd = 	IPCMD_NAME_GET_PARAM,
		.validate = NULL,
		.execute = _clientSysAdminVerInfoExec
	},

	{
		.name = "quit",
		.ipCmd = 	IPCMD_NAME_GET_PARAM,
		.validate = NULL,
		.execute = _quitExec
	},

	{
		.name = NULL,
		.validate = NULL,
		.execute = NULL
	}
};

static int	_apiFindCmd(struct API_PARAMETERS *params, char *programName)
{
	API_CLIENT_CMD_HANDLER *handle = apiClientCmdHandlers;

	while(handle->execute )
	{
		if(IS_STRING_EQUAL(handle->name, params->cmd) )
		{
			return EXIT_SUCCESS;
		}
		
		handle++;
	}
	return EXIT_FAILURE;
}


static int	_apiHandleCmd(struct API_PARAMETERS *params, char *programName)
{
	int ret = EXIT_SUCCESS;

	API_CLIENT_CMD_HANDLER *handle = apiClientCmdHandlers;

	while(handle->execute )
	{
		if(IS_STRING_EQUAL(handle->name, params->cmd) )
		{
			if( handle->validate==NULL ||  (handle->validate)(handle, params, programName ) == EXIT_SUCCESS)
			{
					CTRL_LINK_TYPE linkType = CTRL_LINK_UDP;
					
					if(params->protocol == PROTOCOL_TCP)
						linkType = CTRL_LINK_TCP;
					else if(params->protocol == PROTOCOL_UNIX)
						linkType = CTRL_LINK_UNIX;
					
					fprintf(stderr, "Client connectting to %s:%d on %s protocol.....\n", params->address, params->port, (linkType == CTRL_LINK_TCP)?"TCP":(linkType == CTRL_LINK_UDP)?"UDP":"Unix");
					
#if 1
					ret = cmnMuxClientInit(params->port, linkType, params->address);
#else					
					ret = CLIENT_INIT_IP_CMD(params->port, params->address);
#endif
					if(ret == EXIT_FAILURE)
					{
						printf("JSON API initialization failed, please check your configuration and server address\n");
						exit(1);
						return EXIT_FAILURE;
					}

					fprintf(stderr, "Client execute command of '%s'.....\n", params->cmd);
					return (handle->execute)(handle, params);
			}
			else
			{
				return EXIT_FAILURE;
			}
			
		}
		
		handle++;
	}

	printf("WARNS: '%s' command is not found\n\n", params->cmd);
	usage(programName, params);
	return ret;
}


int main(int argc, char *argv[])
{
	int res = EXIT_SUCCESS;
	int opt;

	struct	API_PARAMETERS params;

	memset(&params, 0, sizeof(struct API_PARAMETERS));

	cmn_list_init(&params.files);
	snprintf(params.address, sizeof(params.address), "%s", "127.0.0.1");
	snprintf(params.macAddress, sizeof(params.macAddress), "%s", "FF:FF:FF:FF:FF:FF");

	snprintf(params.user, sizeof(params.user), "%s", MUX_AUTH_USER);
	snprintf(params.passwd, sizeof(params.passwd), "%s", MUX_AUTH_PASSWORD);

	params.port = UDP_SERVER_PORT;
	params.protocol = PROTOCOL_TCP;
	
	params.left = -1;
	params.width = -1;
	params.height = -1;
	params.duration = -1;
	params.color = -1;

//	while ((opt = getopt (argc, argv, "a:p:b:c:i:m:l:t:w:h:a:d:C:s:u:f:P:p:b:")) != -1)
	while ((opt = getopt (argc, argv, "a:p:b:c:d:u:w:m:")) != -1)
	{
		switch (opt)
		{
			case 'a':/*address*/
				snprintf(params.address, sizeof(params.address), "%s",optarg);
				break;

			case 'm': /* MAC address */
				snprintf(params.macAddress, sizeof(params.macAddress), "%s",optarg);
				break;

			case 'p': /* protocol */
				params.protocol = atoi(optarg);
				if(params.protocol >= PROTOCOL_UNKNOWN)
					params.protocol = PROTOCOL_UDP;
				break;

			case 'b': /* port */
				params.port = atoi(optarg);
				break;

			case 'c': /* command */
				snprintf(params.cmd, sizeof(params.cmd), "%s", optarg);
				if(_apiFindCmd(&params, argv[0]) == EXIT_FAILURE)
				{
					fprintf(stderr, "'%s' is not validate command\n", optarg );
					exit(1);
				}
				break;

			case 'd': /*JSON data */
				params.dataObj = cJSON_Parse(optarg);
				if(params.dataObj == NULL )
				{
					fprintf(stderr, "data '%s' is not validate JSON format\n", optarg );
					exit(1);
				}
				break;

			case 'u': /* user name */
				snprintf(params.user, sizeof(params.user), "%s", optarg);
				break;
			case 'w': /* password */
				snprintf(params.passwd, sizeof(params.passwd), "%s",optarg);
				break;

			default:
				usage(argv[0], &params);
		}

	}

//	res = cmnMuxPlayerParseConfig(MUX_PLAYER_CONFIG_FILE, &_cfg);

	printf(CMN_VERSION_INFO(CMN_MODULE_APICLIENT_NAME) EXT_NEW_LINE );

	if( IS_STRING_NULL(params.cmd) )
	{
		printf("Command is not defined\n");
		usage(argv[0], &params);
	}
	

	/* res is EXIT_SUCESS when command is handled by this function; params->result is NULL when command failed */
	res = _apiHandleCmd(&params, argv[0]);
	if(res ==  EXIT_FAILURE)
	{
		printf("Unknow command '%s' \n", params.cmd);
		usage(argv[0], &params);
	}

	if(! params.result )
	{
		return 1;
	}

#if 0	
	res = muxApiGetStatus(params.result);
	if( res == 200)
	{
		printf("Command '%s' success\n", params.cmd);
	}
	
	else
#endif		
	{
		MUX_DEBUG_JSON_OBJ(params.result);
		/* bash script will check it to determine whether go on or not */
		return 1;
	}
	
	return 0;
}

