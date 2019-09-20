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

	itemObj = cmnJsonSystemGetSubItem(sysObj, item, index);
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
	EXT_MAC_ADDRESS _macAddr;
	
	runCfg->local.ip = cmnSysNetGetIp(ifName);
	runCfg->ipMask = cmnSysNetGetMask(ifName);
	runCfg->ipGateway = cmnSysNetGetDefaultGw(ifName);

	if(cmnSysNetGetMacAddress(ifName, &_macAddr) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
		
	EXT_DEBUGF(EXT_DEBUG_INIT, "IP: %s, 0x%x", cmnSysNetAddress(runCfg->local.ip), runCfg->local.ip);
	EXT_DEBUGF(EXT_DEBUG_INIT, "MASK: %s, 0x%x", cmnSysNetAddress(runCfg->ipMask), runCfg->ipMask);
	EXT_DEBUGF(EXT_DEBUG_INIT, "Gateway: %s, 0x%x", cmnSysNetAddress(runCfg->ipGateway), runCfg->ipGateway) ;

	if(runCfg->isMacConfiged == EXT_TRUE)
	{/* MAC has been set by client, so use local address directly */
		if(! MAC_ADDR_IS_EQUAL(&runCfg->local.mac, &_macAddr) )
		{
			EXT_ERRORF("MAC address is not correct: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x(configured)!=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x(local)", 
				runCfg->local.mac.address[0], runCfg->local.mac.address[1], runCfg->local.mac.address[2], 
				runCfg->local.mac.address[3], runCfg->local.mac.address[4], runCfg->local.mac.address[5], 
				_macAddr.address[0], _macAddr.address[1], _macAddr.address[2],
				_macAddr.address[3], _macAddr.address[4], _macAddr.address[5]);

			memcpy(&runCfg->local.mac, &_macAddr, EXT_MAC_ADDRESS_LENGTH);
		}
	}
	else
	{/* MAC is still not been set by client, so use local address directly */
		memcpy(&runCfg->local.mac, &_macAddr, EXT_MAC_ADDRESS_LENGTH);
	}

	EXT_DEBUGF(EXT_DEBUG_INIT, "Mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X" , runCfg->local.mac.address[0], runCfg->local.mac.address[1], runCfg->local.mac.address[2], 
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
	
	if((runCfg->runtime.revision == 0) && (runCfg->runtime.revision== 0))
	{
		do
		{
			MUX_ERROR("No FPGA is found, service stop");
		}while(1);
	}

	isTx = runCfg->isTx;
	ret = cmnSysCfgRead(runCfg, EXT_CFG_MAIN);
	if(ret == EXIT_FAILURE)
	{/* TX/RX must be get before default condfiguration*/
		cmnSysCfgFromFactory(runCfg);
		
		if(cmnSysCheckFirstRunFlags() )
		{
			unsigned char *address;
			MUX_INFO("This is first startup after this %s board has been programmed. System will initialized based on hardware. ",  (EXT_IS_TX(runCfg))?"TX":"RX");
			address = cmnSysRandomMacAddress();
			MUX_INFO("New random MAC: %02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);
			memcpy(runCfg->local.mac.address, address, EXT_MAC_ADDRESS_LENGTH);
			cmn_free(address);

			cmnSysSetFirstRunFlags();

			cmnSysEthernetConfig(runCfg);
			cmnSysCfgSave(runCfg, EXT_CFG_MAIN);

			MUX_INFO("Please reboot to make it effective after it finished"EXT_NEW_LINE );
			return EXIT_FAILURE;	/* quit from running */
		}

		
		cmnSysCfgSave(runCfg, EXT_CFG_MAIN);
//		MUX_ERROR("No "MUX_SYSTEM_CONFIG_DATA" is found, default %s factory configuration is used", (EXT_IS_TX(runCfg))?"TX":"RX");
	}
	
	runCfg->runtime.reset = 0;
	runCfg->runtime.reboot = 0;
	runCfg->runtime.blink = 0;
	runCfg->isTx = isTx;
	
	memcpy(&runCfg->version, &muxMain->version, sizeof(muxMain->version));

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

	if(cmnSysRs232Init(runCfg)== EXIT_FAILURE)
	{
		MUX_ERROR("Initialization of RS232 failed");
	}
	
	return EXIT_SUCCESS;
}


