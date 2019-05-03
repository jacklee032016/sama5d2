
#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"

#include "_cmnMux.h"

/* get ipaddress */
int cmnJsonGetIpAddress(cJSON* json, const char * key, uint32_t *ipAddress)
{
	char *value;
	uint32_t ip = INVALIDATE_VALUE_U32;

	value = cmnGetStrFromJsonObject(json, key);
	if(! IS_STRING_NULL_OR_ZERO(value) )
	{
		ip = cmnSystemNetIp(value);
		if(ip == INVALIDATE_VALUE_U32)
		{
			return EXIT_FAILURE;
		}
	}

	*ipAddress = ip;
	
	return EXIT_SUCCESS;
}

int cmnJsonGetInteger(cJSON* json, const char * key, uint32_t *ipAddress)
{
	char *value;
	uint32_t ip = INVALIDATE_VALUE_U32;

	value = cmnGetStrFromJsonObject(json, key);
	if(! IS_STRING_NULL_OR_ZERO(value) )
	{
		ip = cmnSystemNetIp(value);
		if(ip == INVALIDATE_VALUE_U32)
		{
			return EXIT_FAILURE;
		}
	}

	*ipAddress = ip;
	
	return EXIT_SUCCESS;
}


/* string with length of 0 is validate string for rest API/IP command */
int cmnJsonGetStrIntoString(cJSON* json, const char * key, char *value, int size)
{
	char *_value = cmnGetStrFromJsonObject(json, key);

//	MUX_DEBUG("_value of %s is %s", key, (_value==NULL)?"NULL":_value);
	if(IS_STRING_NULL(_value) )
	{
		return EXIT_FAILURE;
	}

	snprintf(value, size, "%s", _value);
	return EXIT_SUCCESS;
}


char* cmnGetStrFromJsonObject(cJSON* json, const char * key)
{
	cJSON * obj = cJSON_GetObjectItem(json, key);

//	MUX_DEBUG("obj of %s is %s", key, (obj==NULL)?"NULL":obj->string );
	if(cJSON_IsString(obj) )
	{
		return obj->valuestring;
	}
	else
	{
//		return "";
		return NULL;
	}
}


int cmnGetIntegerFromJsonObject(cJSON* json, const char * key)
{
	cJSON * obj = cJSON_GetObjectItem(json, key);
	
	if(cJSON_IsNumber(obj) )
		return obj->valueint;
	else
		return INVALIDATE_VALUE_U32;
}


cJSON *cmnMuxJsonLoadConfiguration(char *cfgFileName)
{
	cJSON *cfgHandlers = NULL;

	char *jsonStr = cmn_read_file(cfgFileName);
	if ((jsonStr == NULL) )
	{
		MUX_ERROR("IP Command configuration file '%s' reading failed", cfgFileName);
		return NULL;
	}
	
	cfgHandlers = cJSON_Parse(jsonStr);
	if (cfgHandlers== NULL)
	{
		cmn_free(jsonStr);
		MUX_ERROR("IP Command configuration file '%s' Parsing failed", cfgFileName);
		return NULL;
	}

	cmn_free(jsonStr);

	return cfgHandlers;
}


cJSON *cmnMuxSystemJSon2Flat(cJSON *systemJson)
{
	int count, i;
	cJSON *flatObj = NULL;

//	flatObj =cJSON_CreateArray();
	flatObj =cJSON_CreateObject();
	
	count = cJSON_GetArraySize(systemJson);
	MUX_DEBUG("Total %d items", count );
	
	for(i=0; i < count; i++ )
	{
		cJSON *item = cJSON_GetArrayItem(systemJson, i);
		cJSON *obj = cJSON_GetArrayItem(item, 0);
		MUX_DEBUG("#%d: %s", i, obj->string );
		if(obj)
		{
			cJSON *child = obj->child;
			while(child)
			{
				MUX_DEBUG("Add '%s':'%s'", child->string, (child->type == cJSON_String)? child->valuestring:"Othertype");

				cJSON_AddItemToArray(flatObj, cJSON_Duplicate(child, 1));
				
				child = child->next;
			}

		}
	}

	MUX_DEBUG_JSON_OBJ(flatObj);

	return flatObj;
}


