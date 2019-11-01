/*
* System control routine dependent on X86 or ARM
*/

#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdint.h>
#include <dirent.h>
#include <signal.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#ifdef	X86
#define		EXT_REBOOT()	\
				EXT_INFOF("Reboot now!")
#else
#define		EXT_REBOOT()	cmnSysForkCmd("reboot")

// \ 			{sync(); reboot(RB_AUTOBOOT); EXT_INFOF("Reboot now!");}

//				reboot(LINUX_REBOOT_CMD_CAD_OFF)
#endif

/* get an unicast and local mac address */
unsigned char *cmnSysRandomMacAddress(void)
{
#define	DEV_RANDOM			"/dev/random"

	unsigned char *address;
	int fd;
	uint32_t len;

	address = cmn_malloc(EXT_MAC_ADDRESS_LENGTH);
	memset(address, INVALIDATE_VALUE_U8, EXT_MAC_ADDRESS_LENGTH);

	fd = open(DEV_RANDOM, O_RDONLY);
	if(fd< 0)
	{
		EXT_ERRORF("Open random device failed");
		return address;
	}

	len = read(fd, address, EXT_MAC_ADDRESS_LENGTH);
	if(len != EXT_MAC_ADDRESS_LENGTH)
	{
		EXT_ERRORF("Read from random device error: %m");
	}
	close(fd);

	address[0] = address[0]*0xFE|0x02;	

	return address;
}


int cmnSysCheckFirstRunFlags(void)
{
	uint32_t size;
	
	void *data = cmn_read_file(MUX_SYSTEM_INIT_FLAGS, &size);
	if(data == NULL)
	{/* this is first time system start */
		return EXT_TRUE;
	}

	free(data);

	return EXT_FALSE;
}

int cmnSysSetFirstRunFlags(void)
{
	uint32_t size = 1;
	
	if(cmn_write_file(MUX_SYSTEM_INIT_FLAGS, &size,  size) != size)
	{
		EXT_ERRORF("Write start flags data failed: %m");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


int cmnSysCfgRead(EXT_RUNTIME_CFG *cfg, EXT_CFG_TYPE cfgType)
{
//	int startPage = (cfgType== EXT_CFG_MAIN)?FLASH_START_PAGE_CONFIGURATION:FLASH_START_PAGE_CONFIG_BACKUP;
	uint32_t size;
	EXT_RUNTIME_CFG *_readOut;
	void *data = cmn_read_file(MUX_SYSTEM_CONFIG_DATA, &size);
	if(data == NULL)
	{
		EXT_ERRORF("Read configuration data from %s failed", MUX_SYSTEM_CONFIG_DATA);
		return EXIT_FAILURE;
	}

	_readOut = (EXT_RUNTIME_CFG *)data;
//	EXT_DBG_ERRORF(("Read configuration"EXT_NEW_LINE), (ret==EXIT_SUCCESS), return EXIT_FAILURE);

#if 0	
	if(cfg->magic[0]!=EXT_MAGIC_VALUE_A ||cfg->magic[1] != EXT_MAGIC_VALUE_B ||
		cfg->endMagic[0]!=EXT_MAGIC_VALUE_B ||cfg->endMagic[1] != EXT_MAGIC_VALUE_A)
#else		
	if(_readOut->magic[0]!=EXT_MAGIC_VALUE_A ||_readOut->magic[1] != EXT_MAGIC_VALUE_B )
#endif		
	{
		EXT_ERRORF("Configuration data is corrupted!!");
		return EXIT_FAILURE;
	}

	memcpy(cfg, data, (size>sizeof(EXT_RUNTIME_CFG))?sizeof(EXT_RUNTIME_CFG):size);
	free(data);
	
	return EXIT_SUCCESS;
}


int cmnSysCfgSave( EXT_RUNTIME_CFG *cfg, EXT_CFG_TYPE cfgType )
{
	uint32_t size = EXT_RUNTIME_CFG_WRITE_SIZE;
	if(cmn_write_file(MUX_SYSTEM_CONFIG_DATA, (void *)cfg,  size) != size)
	{
		EXT_ERRORF("Write configuration data failed");
		return EXIT_FAILURE;
	}

//	EXT_DBG_ERRORF(("Save configuration"EXT_NEW_LINE), (ret==EXIT_SUCCESS), return EXIT_FAILURE);
	
	return EXIT_SUCCESS;
}

int cmnSysEthernetConfig( EXT_RUNTIME_CFG *cfg )
{
#define	DHCP_CLIENT	"udhcpc"

	FILE *f;
	int res = EXIT_SUCCESS;
	char cmd[CMN_NAME_LENGTH];

	f = fopen(SYS_NET_CONFIG_FILE, "w");
	if (!f)
	{
		EXT_ERRORF("Open network file "SYS_NET_CONFIG_FILE" failed: %m");
		return EXIT_FAILURE;
	}

	if( fprintf(f, "# /etc/network/interfaces by %s\n# %s\n\n", CMN_MODULE_MAIN_NAME, cmnTimestampStr() ) <= 0)
	{
		EXT_ERRORF("Write "SYS_NET_CONFIG_FILE" failed: %m");
		goto writeError;
	}
	

	/* loop back */
	if( fprintf(f, "# The loopback interface\nauto lo\niface lo inet loopback\n\n") <= 0)
	{
		EXT_ERRORF("Write "SYS_NET_CONFIG_FILE" failed: %m");
		goto writeError;
	}
	
	/* eth0 */
	res = fprintf(f, "# wire interface %s\n", MUX_ETH_DEVICE);
	res += fprintf(f, "auto %s\n", MUX_ETH_DEVICE);
	if(EXT_DHCP_IS_ENABLE(cfg) )
	{
		res += fprintf(f, "iface %s inet dhcp\n", MUX_ETH_DEVICE);
		res += fprintf(f, "\thwaddress ether %02x:%02x:%02x:%02x:%02x:%02x\n", 
			cfg->local.mac.address[0], cfg->local.mac.address[1], cfg->local.mac.address[2], cfg->local.mac.address[3], cfg->local.mac.address[4], cfg->local.mac.address[5]);
		
		char cmd[CMN_NAME_LENGTH];
		sprintf(cmd, DHCP_CLIENT" -R -b -p /var/run/udhcpc.eth0.pid -i %s ", MUX_ETH_DEVICE );

		cmnSysForkCmd(cmd);
	}
	else
	{
		uint32_t netAddress = cfg->local.ip & cfg->ipMask;
		
		res += fprintf(f, "iface %s inet static\n\taddress %s\n", MUX_ETH_DEVICE, cmnSysNetAddress(cfg->local.ip) );
		res += fprintf(f, "\tnetmask %s\n", cmnSysNetAddress(cfg->ipMask) );
		res += fprintf(f, "\tnetwork %s\n",  cmnSysNetAddress(netAddress) );
		res += fprintf(f, "\tgateway %s\n", cmnSysNetAddress(cfg->ipGateway) );
		res += fprintf(f, "\thwaddress ether %02x:%02x:%02x:%02x:%02x:%02x\n", 
			cfg->local.mac.address[0], cfg->local.mac.address[1], cfg->local.mac.address[2], cfg->local.mac.address[3], cfg->local.mac.address[4], cfg->local.mac.address[5]);
 	
		/* even new ip address and gateway can be used at once, but muxMgr can't work at once */
#if 1
		/* kill process udhcpc */
		cmnSysKillProcess(DHCP_CLIENT);

		/* configure address and gateway */
		sprintf(cmd, "/sbin/ifconfig %s %s", MUX_ETH_DEVICE , cmnSysNetAddress(cfg->local.ip) );
		cmnSysForkCmd(cmd);

		cmnSysNetSetGateway(cmnSysNetAddress(cfg->ipGateway), MUX_ETH_DEVICE);
#endif		
	}
	res += fprintf(f, "\n\n" );
	if(res <= 0)
	{
		EXT_ERRORF("Write "SYS_NET_CONFIG_FILE" failed: %m");
		goto writeError;
	}
	EXT_INFOF("Write IP address as %s", EXT_DHCP_IS_ENABLE(cfg)?"DHCP":"Static");

writeError:
	fclose(f);
	
	return EXIT_SUCCESS;
}


int cmnSysSaveMac2Uboot( EXT_RUNTIME_CFG *cfg )
{
	char cmd[CMN_NAME_LENGTH];

	sprintf(cmd, "fw_setenv ethaddr %.2x:%.2x:%.2x:%.2x:%.2x:%.2x > /dev/null", 
		cfg->local.mac.address[0], cfg->local.mac.address[1], cfg->local.mac.address[2], 
		cfg->local.mac.address[3], cfg->local.mac.address[4], cfg->local.mac.address[5] );
#if 0
	if(system(cmd) <0 )
	{
		EXT_ERRORF("Write MAC failed with cmd:%s", cmd);
		return EXIT_FAILURE;
	}
#else
	cmnSysForkCmd(cmd);
#endif
	EXT_INFOF("Write MAC as:'%.*s'", 17, cmd+18 );
	
	return EXIT_SUCCESS;
}


int	cmnSysCtrlBlinkPowerLED(char	isEnable)
{
	cmnSysLedCtrl(LED_TYPE_POWER, (isEnable)?LED_MODE_BLINK:LED_MODE_ON); /* 09.29, 2019, no blink means ON */

	return EXIT_SUCCESS;
}


static int _cmnSysJobDelay(const char *name, int delayMs, CMN_THREAD_TIMER_CALLBACK callback, void *data)
{
	void	*timer = cmn_add_timer(delayMs, callback, data, cmn_timer_type_once, name);
	if (timer==NULL)
	{
		EXT_ERRORF("Delay Job '%s' can not be created", name);
		return EXIT_FAILURE;
	}
	EXT_DEBUGF(EXT_DBG_ON, "Delay Job '%s' about %d ms has been created", name, delayMs);

	return EXIT_SUCCESS;
}


static int _delayReboot(int delayMs, void *data)
{
	EXT_REBOOT();
	return EXIT_SUCCESS;
}

void extCfgFactoryKeepMac( EXT_RUNTIME_CFG *cfg )
{
	EXT_MAC_ADDRESS macAddress;
	
	unsigned char isMacConfiged = cfg->isMacConfiged;
	memcpy(&macAddress, &cfg->local.mac, sizeof(EXT_MAC_ADDRESS));

	cmnSysCfgFromFactory(cfg);

	memcpy(&cfg->local.mac, &macAddress, sizeof(EXT_MAC_ADDRESS));
	cfg->isMacConfiged = isMacConfiged;

	cmnSysCfgSave(cfg, EXT_CFG_MAIN);
}


int cmnSysCtrlDelayReboot(unsigned short delayMs, void *data)
{
	return _cmnSysJobDelay("reboot", delayMs, _delayReboot, data);
}

static int _delayReset(int delayMs, void *data)
{
	EXT_RUNTIME_CFG *runCfg = (EXT_RUNTIME_CFG *)data;
	extCfgFactoryKeepMac(runCfg);
	
	EXT_REBOOT();
	return EXIT_SUCCESS;
}


int cmnSysCtrlDelayReset(unsigned short delayMs, void *data)
{
	return _cmnSysJobDelay("reset", delayMs, _delayReset, data);
}


static int _directoryFiltByNumber(const struct dirent *namelist)
{
	char  *c;
	int isnum=1;

	c = (char *)namelist->d_name;
	while( *c != '\0' )
	{
		if((*c)<48 || (*c)>57)/*'0'=48,'9'=57*/
		{
			isnum=0;
			break;
		}
		c++;
	}
	return isnum;
}


/* get process ID, not thread ID */
int cmnSysGetPidByName(char *progName)
{
#define	_DEBUG_THREAD_LOOK		EXT_DBG_OFF

	struct dirent **namelist;
	int n;
	char file[256];
	FILE *fp;
	int pid = -1;
	
	struct info_type
	{
		char waste[16];
		char value[64];
	}serverinfo;
	
	n = scandir("/proc", &namelist,  _directoryFiltByNumber, alphasort);
	EXT_DEBUGF(_DEBUG_THREAD_LOOK, "Number: %d", n);
	while(n--)
	{
		snprintf(file, sizeof(file), "/proc/%s/status", namelist[n]->d_name);		
		EXT_DEBUGF(_DEBUG_THREAD_LOOK, "\tOpen: %s", file);
		
		if((fp=fopen(file,"r"))==NULL)
		{
			MUX_ERROR("find process ID of %s: open status file '%s' fail: %m", progName, file );
			continue;
		}
	
		memset(&serverinfo, 0, sizeof(struct info_type));
			
		/*get the server name*/
		if(fscanf(fp, "%s %s", serverinfo.waste, serverinfo.value) != 2)
		{
			MUX_ERROR("Parse status of %s failed: %m", file);
		}
		EXT_DEBUGF(_DEBUG_THREAD_LOOK, "\twaste: '%s'; value:'%s'", serverinfo.waste, serverinfo.value);
		
		if(!strcmp( serverinfo.value, progName) )
		{
			pid = atoi( namelist[n]->d_name );
		}

		if(!strcmp(serverinfo.waste,"State:"))
		{
			if(strchr(serverinfo.value,'Z'))
			{
				MUX_ERROR("'%s' status Z", file );		
				pid = -1;
			}
		}
		
		fclose(fp);
		free(namelist[n]);
	}
		
	free(namelist);
	
	return pid;
}


int cmnSysSigSend(pid_t pid, int sigid, int opcode)
{
	union sigval sval;

	sval.sival_int=opcode;	/* opcode can be parsed by receiving process */

	// should enhanced uClibc, lzj
	if(sigqueue(pid, sigid, sval) )
	{
		MUX_ERROR("Send signal %d to process %d failed: %m", sigid, pid);

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int cmnSysKillProcess(char *name)
{
	pid_t pid;

	pid = cmnSysGetPidByName(name);
	if(pid < 0)
	{
		MUX_ERROR("Process %s is not started now", name);

		return EXIT_FAILURE;
	}

	return cmnSysSigSend(pid, SIGKILL, 0);
}

#if 0
void cgi_send_signal_to_process_by_name(int sig, char *prog_name)
{
	pid_t pid;

	pid = (pid_t)cgi_get_pid_by_name( prog_name);
	if( pid <0 )
		return;

	kill(pid, sig);

	cgi_send_signal_to_process_by_name(SIGKILL, node->name);

}
#endif

void cmnSysForkCmd(const char *cmd) 
{
	char command[1024];
//	snprintf(command, sizeof(command), "%s >> /dev/null" , cmd);
//	snprintf(command, sizeof(command), "%s 1>&2 /dev/null" , cmd);
	/*
	* >: truncate; >>: append, for output redirect
	* 2:error; 1: output; 2>&1 : redirect 2 to 1 
	*/
	snprintf(command, sizeof(command), "%s > /dev/null 2>&1" , cmd);

	EXT_INFOF("Exec '%s' ...", command);

#if 	0
	if(!fork()) 
	/* if fork new child process to execute this command, then thread list in 'ps' will add one with name of this thread after every command. 
	* And the new child process will execute the function which call this function, such as IP address configuration: write twice in network/interfaces
	08.08, 2019 */	
#else
#endif
	{/* child */
		if(system(command) < 0)
		{
			EXT_ERRORF("Exec '%s' failed", cmd);
			return ;//EXIT_FAILURE;
		}
		
//		exit(0);
	}

	/* parent process(thread) can't wait */	
//	sleep(1); /* one second */
	return;
}

