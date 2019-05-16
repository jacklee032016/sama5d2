

#include "libCmn.h"
#include "libMux.h"
#include "libCmnSys.h"

int main(int argc, char *argv[])
{
	int index = 0;
	char *ip[] = 
		{
			"239.122.1.2",
			"239.122.2.2",
			"239.122.3.2",
			"239.122.4.2",
			"239.122.5.2",
			NULL
		};
	
	CmnMultiGroup *group = cmnSysNetMGroupInit("eth0", "239.111.0.1");

	while(1)
	{
		char *newIP;

		cmn_delay(2000*10);
		newIP = ip[index];
		index++;
		if(newIP == NULL)
			break;

		group->changeGroup(group, newIP);
	}

	cmnSysNetMGroupDestory(group);

	return 0;
}

