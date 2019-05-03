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


cJSON *extSystemFindObject(MuxMain *muxMain, const char*objName )
{
	cJSON *obj =	cJSON_GetObjectItem(muxMain->systemJson, objName);
	if (obj== NULL)
	{
		MUX_ERROR("No data for '%s' is found", objName);
	}
	return obj;
}


/* only called by process of IP command */
cJSON *extSystemGetSubItem(cJSON *sysObj, char *item, int index)
{
	cJSON *itemObj;

	itemObj = cJSON_GetObjectItem(sysObj, item);
	if (itemObj== NULL)
	{
		MUX_ERROR("No data for '%s' is found", item);
		return NULL;
	}
	
	if(index != INVALIDATE_VALUE_U32)
	{
		cJSON *obj = cJSON_GetArrayItem(itemObj, index);
		if(obj == NULL)
		{
			MUX_ERROR("No index#%d in data item '%s' is found", index, item);
		}

		return obj;
	}


	MUX_DEBUG_JSON_OBJ(itemObj);
	
	return itemObj;
}

cJSON *extSystemGetKey(cJSON *sysObj, char *item, int index, char *key)
{
	cJSON *itemObj, *destObj;

	itemObj = extSystemGetSubItem(sysObj, item, index);
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


/* refresh params into JSON object */
int extSystemUpdateJsonObject(MuxMain *muxMain)
{
	cJSON *newObj = NULL;
	cJSON *itemObj = NULL;
	char macAddress[128];
	

	itemObj = extSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_SYSTEM, INVALIDATE_VALUE_U32);
	
	newObj = cJSON_CreateString(cmnSysNetAddress(muxMain->runCfg.local.ip) );
	cJSON_ReplaceItemInObject(itemObj, FIELD_SYS_CFG_ADDRESS, newObj);

	newObj = cJSON_CreateString(cmnSysNetAddress(muxMain->runCfg.ipMask) );
	cJSON_ReplaceItemInObject(itemObj, FIELD_SYS_CFG_NETMASK, newObj);

	newObj = cJSON_CreateString(cmnSysNetAddress(muxMain->runCfg.ipGateway) );
	cJSON_ReplaceItemInObject(itemObj, FIELD_SYS_CFG_GATEWAY, newObj);

	snprintf(macAddress, sizeof(macAddress), "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X" , 
		muxMain->runCfg.local.mac.address[0], muxMain->runCfg.local.mac.address[1], muxMain->runCfg.local.mac.address[2], 
		muxMain->runCfg.local.mac.address[3], muxMain->runCfg.local.mac.address[4], muxMain->runCfg.local.mac.address[5]);

	newObj = cJSON_CreateString(macAddress);
	cJSON_ReplaceItemInObject(itemObj, FIELD_SYS_CFG_MAC, newObj);
	

	return EXIT_SUCCESS;
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


	return extSystemUpdateJsonObject(muxMain);
}


