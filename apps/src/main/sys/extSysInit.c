#include "mux.h"

int extSystemJsonInit(const cJSON *array)
{
	cJSON *child = array->child;
	size_t i = 0;


	while(child)
	{
//		MUX_DEBUG("JSON %s, %s", child->string, (child->type == cJSON_String)? child->valuestring:"Othertype");
		
		i++;
		child = child->next;
	}


	return (int)i;
}



cJSON *extSystemGetKey(cJSON *sysObj, char *item, int index, char *key)
{
	cJSON *itemObj, *destObj;

	itemObj = cmnJsobSystemGetSubItem(sysObj, item, index);
	if(item == NULL)
	{
		MUX_ERROR("No data for '%s' is found", item);
		return NULL;
	}

	destObj = cJSON_GetObjectItem(itemObj, key);
	if(item == NULL)
	{
		MUX_ERROR("No data with key '%s' in SubItem '%s' is found", key, item);
		return NULL;
	}

	return destObj;
}

static int	_extSysNetInit(MuxMain		*muxMain)
{
	char *ifName = MUX_ETH_DEVICE;
	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;
	
	runCfg->local.ip = cmnSysNetGetIp(ifName);
	runCfg->ipMask = cmnSysNetGetMask(ifName);
	runCfg->ipGateway = cmnSysNetGetDefaultGw(ifName);

	if(cmnSysNetGetMacAddress(ifName, &muxMain->runCfg.local.mac) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
		
	EXT_DEBUGF(EXT_DEBUG_INIT, "IP is %s, 0x%x", cmnSysNetAddress(runCfg->local.ip), runCfg->local.ip);
	EXT_DEBUGF(EXT_DEBUG_INIT, "MASK is %s, 0x%x", cmnSysNetAddress(runCfg->ipMask), runCfg->ipMask);
	EXT_DEBUGF(EXT_DEBUG_INIT, "Gateway is %s, 0x%x", cmnSysNetAddress(runCfg->ipGateway), runCfg->ipGateway) ;
	EXT_DEBUGF(EXT_DEBUG_INIT, "Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X" , runCfg->local.mac.address[0], runCfg->local.mac.address[1], runCfg->local.mac.address[2], 
		runCfg->local.mac.address[3], runCfg->local.mac.address[4], runCfg->local.mac.address[5] );

	return EXIT_SUCCESS;
}


static int	_extSysJsonInit(MuxMain		*muxMain)
{
	muxMain->systemJson = cmnMuxJsonLoadConfiguration(MUX_SYSTEM_JSON_CONFIG);
	if (muxMain->systemJson == NULL)
	{
		MUX_ERROR("IP Command configuration file '%s' Parsing failed", MUX_SYSTEM_JSON_CONFIG);
		return EXIT_FAILURE;
	}

#if 0//EXT_DEBUG_INIT
	int count = 0, i;
	count = cJSON_GetArraySize(muxSystemJson);
	MUX_DEBUG("Total %d items", count );
	for(i=0; i < count; i++ )
	{
		cJSON *item = cJSON_GetArrayItem(muxSystemJson, i);
		cJSON *obj = cJSON_GetArrayItem(item, 0);
		MUX_DEBUG("Item#%d:%s, No#1:%s", i, item->string, obj->string );
		
		if(obj)
		{
//				MUX_DEBUG("\t#%d: %s:%s", i, type->string, type->valuestring);
			int _count = extSystemJsonInit(item);
			MUX_DEBUG("\tTotal %d==%d sub functions", _count, cJSON_GetArraySize(item) );
		}
	}
#endif

	return cmnSysJsonUpdate(muxMain);
}


int32_t	extSystemInit(MuxMain		*muxMain)
{
	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;
	unsigned char isTx = EXT_FALSE;
	int ret;

	/* read FPGA info to determine whether this is TX or RX before get other configuration */
	if(sysFpgaCheck(runCfg)== EXIT_FAILURE)
	{
		EXT_ERRORF("Check FPGA failed");
	}

	isTx = runCfg->isTx;
	ret = cmnSysCfgRead(runCfg, EXT_CFG_MAIN);
	runCfg->isTx = isTx;

	if(ret == EXIT_FAILURE)
	{
		MUX_ERROR("Read configuration data failed, using factory default data");
		cmnSysCfgFromFactory(runCfg);
	}

	if(_extSysNetInit(muxMain)== EXIT_FAILURE)
	{
		MUX_ERROR("Network initialization failed");
		return EXIT_FAILURE;
	}

	if(_extSysJsonInit(muxMain)== EXIT_FAILURE)
	{
		MUX_ERROR("JSON initialization failed");
		return EXIT_FAILURE;
	}

	/* config FPGA and join multicast group */
	if(sysFpgaInit(runCfg)== EXIT_FAILURE)
	{
		MUX_ERROR("FPGA failed");
	}
	
	return EXIT_SUCCESS;
}


