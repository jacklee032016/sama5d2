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


int32_t	extSystemInit(MuxMain		*muxMain)
{
//	uint32_t ip, mask;
	char *ifName ="eth0";
	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;

	extCfgFromFactory(runCfg);
	
	{
		runCfg->local.ip = cmnSysNetGetIp(ifName);
		runCfg->ipMask = cmnSysNetGetMask(ifName);
		runCfg->ipGateway = cmnSysNetGetDefaultGw(ifName);

		if(cmnSysNetGetMacAddress(ifName, &muxMain->runCfg.local.mac) == EXIT_FAILURE)
		{
			return EXIT_FAILURE;
		}
		
		MUX_DEBUG("IP is %s, 0x%x", cmnSysNetAddress(runCfg->local.ip), runCfg->local.ip);
		MUX_DEBUG("MASK is %s, 0x%x", cmnSysNetAddress(runCfg->ipMask), runCfg->ipMask);
		MUX_DEBUG("Gateway is %s, 0x%x", cmnSysNetAddress(runCfg->ipGateway), runCfg->ipGateway);
		MUX_DEBUG("Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X" , runCfg->local.mac.address[0], runCfg->local.mac.address[1], runCfg->local.mac.address[2], 
			runCfg->local.mac.address[3], runCfg->local.mac.address[4], runCfg->local.mac.address[5]);

	}

	{
		cJSON				*muxSystemJson = NULL;
		int count = 0, i;

		muxSystemJson = cmnMuxJsonLoadConfiguration(MUX_SYSTEM_CONFIG_FILE);
		if (muxSystemJson== NULL)
		{
			MUX_ERROR("IP Command configuration file '%s' Parsing failed", MUX_SYSTEM_CONFIG_FILE);
			return EXIT_FAILURE;
		}
		muxMain->systemJson = muxSystemJson;

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
	}
	

	return EXIT_SUCCESS;
}

cJSON *extSystemFindObject(MuxMain *muxMain, const char*objName )
{
	cJSON *obj =	cJSON_GetObjectItem(muxMain->systemJson, objName);
	if (obj== NULL)
	{
		MUX_ERROR("No data for '%s' is found", objName);
	}
	return obj;
}

