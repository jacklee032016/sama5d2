
#include "libCmnSys.h"

#define CMN_MODULE_TEST_NAME			"testsMux" //module name

#define	TEST_CMD_IGMP					"igmp"
#define	TEST_CMD_DIP_SWITCH			"dip"

#define	TEST_CMD_LED_VIDEO			"ledV"
#define	TEST_CMD_LED_POWER			"ledP"
#define	TEST_CMD_LED_ACT				"ledA"

#define	TEST_CMD_LEDS					"led"
#define	TEST_CMD_SW1					"sw1"
#define	TEST_CMD_SECURE_CHIP			"sc"

#define	TEST_CMD_I2C					"i2c"
#define	TEST_CMD_SPI					"spi"

#define	TEST_CMD_RS232				"rs"

#define	TEST_CMD_CLOCK				"clk"

#define	TEST_CMD_TIMESTAMP			"ts"

#define	TEST_CMD_TIMERS				"timers"


int testIgmp(void);
int testDipSwitch(void);
int testLedVideo(void);
int testLeds(void);
int testSw1(void);
int testSecureChipId(void);

int testDs2815(void );
int testSC(void );
int testI2c(void);
int testRs232(void);

int testTimers(void);


int	cmnSysSpiTest(void);;

static void _usage(char* base)
{
	printf("%s: \n\tTest command line utility.\n"\
		"\t%s -c command "
		"\t\t Current command:  " \
		"\n\t\t\t"TEST_CMD_IGMP", "TEST_CMD_DIP_SWITCH", "TEST_CMD_LEDS", "TEST_CMD_SW1", "TEST_CMD_SECURE_CHIP
		", "TEST_CMD_I2C ", "TEST_CMD_SPI ", "TEST_CMD_CLOCK ", "TEST_CMD_TIMESTAMP ", "TEST_CMD_TIMERS" \n", 
		  base, base);

	exit(-1);
}

unsigned int testFreq = 20;

int testClock(void)
{
	EXT_INFOF("Test Clock of %d MHz...", testFreq);
	
	if(testFreq <= 0 )
	{
		EXT_ERRORF("Test frequency must be defined in MHz and larger than 1");
		return EXIT_FAILURE;
	}

	if(cmnSysI2cSi5351VcxoControl((unsigned long)testFreq*UNIT_M_HZ )  == EXIT_SUCCESS)
	{
		EXT_INFOF("Frequency of CLK6 has been changed into %d MHz, please check with oscilloscope", testFreq);
	}

	return EXIT_SUCCESS;
}

int testTimestamp2(void)
{
	struct timeval tv;

	gettimeofday(&tv,NULL);
//	return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;

	uint64_t tsUs = cmnGetTimeUs();
	uint32_t regVideo = FPGA_GET_PTP_TIMESTAMP_VIDEO();
	uint32_t regAudio = FPGA_GET_PTP_TIMESTAMP_AUDIO();

//	printf("Current Timestamp: %llu us, video reg: %u; audio reg: %u; %lu ms\n", tsUs, regVideo, regAudio, cmnGetTimeMs());
	printf("Current Timestamp: %llu us, video reg: %u; audio reg: %u;\n", tsUs, regVideo, regAudio );
	printf("%lu second, %lu us\n", tv.tv_sec, tv.tv_usec);

	return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
	int opt;
	char		cmd[CMN_NAME_LENGTH];

	while ((opt = getopt (argc, argv, "c:f:")) != -1)
	{
		switch (opt)
		{

			case 'c': /* command */
				snprintf(cmd, sizeof(cmd), "%s", optarg);
				break;

			case 'f': /* MHz Frequency of CLOCK */
				testFreq = atoi(optarg);
				break;

			default:
				_usage(argv[0]);
		}

	}

	printf(CMN_VERSION_INFO(CMN_MODULE_TEST_NAME) EXT_NEW_LINE );

	if( IS_STRING_NULL_OR_ZERO(cmd) )
	{
		printf("Command is not defined\n");
		_usage(argv[0]);
	}
	
	if(IS_STRING_EQUAL(cmd, TEST_CMD_IGMP) )
	{
		testIgmp();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_DIP_SWITCH))
	{
		testDipSwitch();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_LEDS))
	{
		testLeds();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_SW1))
	{
		testSw1();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_SPI))
	{
		int fd = cmnSysRawSpiFlashInit();
		cmnSysRawSpiFlashClose(fd);
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_SECURE_CHIP))
	{
		testSC();
//		testDs2815();
//		testSecureChipId();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_I2C))
	{
		testI2c();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_RS232))
	{
		testRs232();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_CLOCK))
	{	
		testClock();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_TIMESTAMP))
	{	
		testTimestamp2();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_TIMERS))
	{	
		testTimers();
	}
	else
	{
		printf("Unknonw command '%s' is defined\n", cmd);
		_usage(argv[0]);
	}
		
	return 0;
}

