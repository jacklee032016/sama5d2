/*
* configure SFP before network is enabled
*/

#include "libCmnSys.h"

#define CMN_MODULE_SFP_CTRL			"sfpCtrl"

int main(int argc, char *argv[])
{
	EXT_RUNTIME_CFG		_runCfg;
	EXT_RUNTIME_CFG		*runCfg = &_runCfg;
	int ret;

	printf(CMN_VERSION_INFO(CMN_MODULE_SFP_CTRL) EXT_NEW_LINE );

	memset(runCfg, 0, sizeof(EXT_RUNTIME_CFG));

//	cmnSysCfgFromFactory(runCfg);

	ret = cmnSysCfgRead(runCfg, EXT_CFG_MAIN);
	if(ret == EXIT_FAILURE)
	{/* TX/RX must be get before default configuration*/
		/* no configuration is found, so no SFP needed to be configured */
		MUX_INFO("Please reboot to make it effective after it finished"EXT_NEW_LINE );
		return EXIT_FAILURE;	/* quit from running */
	}

	if(sysFpgaInit(runCfg)== EXIT_FAILURE)
	{
		MUX_ERROR("FPGA failed");
		return EXIT_FAILURE;
	}

#if 0
	if(!EXT_IS_TX(runCfg))
	{
		MUX_INFO("FPGA failed");
		return EXIT_FAILURE;
	}
	
	if(sysFpgaCheck(runCfg)== EXIT_FAILURE)
	{
		EXT_ERRORF("Check FPGA failed");
	}
#endif

	sysFpgaCfgSfpWhenBoot();


	EXT_INFOF(EXT_NEW_LINE"OK");
	return 0;
}

