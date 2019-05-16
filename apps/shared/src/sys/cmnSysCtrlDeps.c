/*
* System control routine dependent on X86 or ARM
*/

#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdint.h>

#include "libCmn.h"
#include "libCmnSys.h"

#include "mux7xx.h"

#ifdef	X86
#define		EXT_REBOOT()	\
				EXT_INFOF("Reboot now!")
#else
#define		EXT_REBOOT()	\
				reboot(LINUX_REBOOT_CMD_CAD_OFF)
#endif

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
		EXT_ERRORF(("Configuration data is corrupted!!"EXT_NEW_LINE));
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


int	cmnSysCtrlBlinkPowerLED(char	isEnable)
{
#if 0//def	0
	unsigned char		data = 0x01;
	if(isEnable == 0)
	{
		data = 0;
	}

	_extFpgaWriteByte(EXT_FPGA_REG_POWER_LED, &data);
#else

#endif
	return EXIT_SUCCESS;
}


static int _cmnSysJobDelay(const char *name, int delayMs, CMN_THREAD_TIMER_CALLBACK callback, void *data)
{
	void	*timer = cmn_add_timer(delayMs, callback, data, name);
	if (timer==NULL)
	{
		EXT_ERRORF("Delay Job '%s' can not be created", name);
		return EXIT_FAILURE;
	}

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

//	cmnSysCfgSave(cfg, EXT_CFG_MAIN);

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


