/*
* Write SC key when IP command client is not available
*/

#include "libCmnSys.h"

#define CMN_MODULE_REG_READ			"regRead"

int main(int argc, char *argv[])
{
	EXT_RUNTIME_CFG		_runCfg;
	EXT_RUNTIME_CFG		*runCfg = &_runCfg;

	printf(CMN_VERSION_INFO(CMN_MODULE_REG_READ) EXT_NEW_LINE );

	memset(runCfg, 0, sizeof(EXT_RUNTIME_CFG));

	if(cmnSysCfgRead(runCfg, EXT_CFG_MAIN) == EXIT_FAILURE)
	{
		cmnSysCfgFromFactory(runCfg);
	}

	if(sysFpgaInit(runCfg)== EXIT_FAILURE)
	{
		MUX_ERROR("FPGA failed");
	}

//	sysFpgaRefresh();

	EXT_INFOF(EXT_NEW_LINE"OK");
	return 0;
}

