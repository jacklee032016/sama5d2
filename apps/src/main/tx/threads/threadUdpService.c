
#include "controler.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "multithread.h"

extern void json_value_init(cJSON* root_msg);

volatile int paramInitDone = 0;

void * tx_server_routine()
{	
	cJSON *root_msg; 
	int sockCtl;
	sockCtl = udp_config();
	if (sockCtl <= 0)
		printf("tx_server_routine socket create error!\n");

	root_msg = parameter_list_json_create();
	char * tmp = cJSON_Print(root_msg);	
	//printf("DEBUG roog_msg is : %s \n", tmp);
	
	if (root_msg != NULL) 
	{
		json_value_init(root_msg);
		int isSysReady = 1;	
		char * tmp = cJSON_Print(root_msg);
		if (tmp) 
		{	
			saveParamToDrive(CONFIG_INI, tmp, strlen(tmp));
			free(tmp);
		}		
		pthread_mutex_lock(&i2c_bus_lock);
		fpga_ip_addr_inf(root_msg);		
		pthread_mutex_unlock(&i2c_bus_lock);
		paramInitDone = 1;
		udp_json_parse(sockCtl, root_msg);
		cJSON_Delete(root_msg);
	}
	close(sockCtl);
	return 0;
}
