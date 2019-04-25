
#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"

#include "_cmnMux.h"

char* cmnGetStrFromJsonObject(cJSON* json, const char * key)
{
	cJSON * obj = cJSON_GetObjectItem(json, key);

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
		return -1;
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


