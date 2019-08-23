

#include "libCmnSys.h"

int testIgmp(void)
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



int testDipSwitch(void)
{
#if 0
	int fd;
	unsigned int status;

	fd =open(MUX_DRV_DIP_SWITCH_DEV_NAME, O_RDWR);
	if(fd < 0 )
	{
		printf("\ncan not open %s!\n", MUX_DRV_DIP_SWITCH_DEV_NAME);
		return -1;
	}
	
	while(1)
	{
		if(ioctl(fd, MUX_DRV_DIP_SWITCH_READ, &status) == -1)
		{	
			printf("\n ioctl read switch error!");
		}
		else
		{
			printf("\nreturn status is : 0x%x\n", status);		
		}
		
		sleep(1);
	}
	
	close(fd);
#else	
	while(1)
	{
		int dipStatus = cmnSysDipSwitchCheck();
		if( dipStatus == INVALIDATE_VALUE_U32)
		{
			break;
		}

		EXT_INFOF("DIP State: '0x%x'", dipStatus);
		sleep(2);
	}
	
#endif	
	return 0;		
}

#if 0
int testLedVideo(void)
{
	int fd;
	int i;
	
	fd = open(MUX_DRV_LED_VIDEO_DEV_NAME, O_RDWR);
	if(fd < 0 )
	{
		printf("can not open '%s'!\n", MUX_DRV_LED_VIDEO_DEV_NAME);
		return -1;
	}
	
	for( i = 0; i <  20; i++)
	{
		usleep(200000);
		ioctl(fd, MUX_DRV_LED_VIDEO_OP_ON, 0);  
		usleep(200000);
		ioctl(fd, MUX_DRV_LED_VIDEO_OP_OFF, 0);  	
	}

	close(fd);
	return 0;		
}
#endif

static int _testOneLed(LED_TYPE_T type)
{
	LED_MODE_T mode = cmnSysLedCheck(type);

	/* first case : ON --> OFF */
	EXT_INFOF("\tset LED as ON ...");
	if(cmnSysLedCtrl(type, LED_MODE_ON) )
	{
		EXT_ERRORF("set LED as ON failed");
	}

	sleep(5);
	EXT_INFOF("\tset LED as OFF ...");
	if(cmnSysLedCtrl(type, LED_MODE_OFF) )
	{
		EXT_ERRORF("set LED as OFF failed");
	}

	sleep(5);
	/* second case : OFF --> ON*/
	EXT_INFOF("\tset LED as ON ...");
	if(cmnSysLedCtrl(type, LED_MODE_ON) )
	{
		EXT_ERRORF("set LED as ON failed");
	}

	sleep(5);
	/* case 3: ON--> BLINK */
	EXT_INFOF("\tset LED as BLINK ...");
	if(cmnSysLedCtrl(type, LED_MODE_BLINK) )
	{
		EXT_ERRORF("set LED as BLINK failed");
	}

	sleep(16);
	/* blink --> off */
	EXT_INFOF("\tset LED as OFF ...");
	if(cmnSysLedCtrl(type, LED_MODE_OFF) )
	{
		EXT_ERRORF("set LED as OFF failed");
	}

	sleep(5);
	/* off --> blink */
	EXT_INFOF("\tset LED as BLINK ...");
	if(cmnSysLedCtrl(type, LED_MODE_BLINK) )
	{
		EXT_ERRORF("set LED as BLINK failed");
	}

	/* blink --> ON */
	sleep(16);
	EXT_INFOF("\tset LED as ON ...");
	if(cmnSysLedCtrl(type, LED_MODE_ON) )
	{
		EXT_ERRORF("set LED as ON failed");
	}

	sleep(5);
	/* restore initial state */
	EXT_INFOF("\trestore LED as %s ...", cmnSysLedFindState(mode) );
	if(cmnSysLedCtrl(type, mode) )
	{
		EXT_ERRORF("set LED as ON failed");
	}

	return EXIT_SUCCESS;
}

int testLeds(void)
{
	EXT_INFOF("Test LED Power...");
	_testOneLed(LED_TYPE_POWER);

	EXT_INFOF(EXT_NEW_LINE"Test LED Video...");
	_testOneLed(LED_TYPE_VIDEO);

	EXT_INFOF(EXT_NEW_LINE"Test LED ACT...");
	_testOneLed(LED_TYPE_ACT);
	return EXIT_SUCCESS;
}

int testSw1(void)
{
	EXT_INFOF("Please press SW1 button...");
	return cmnSysSW1CheckEvent();
}


int testI2c(void)
{
	EXT_INFOF("Test I2C...");

	cmnSysI2cTest();

	return EXIT_SUCCESS;
}

