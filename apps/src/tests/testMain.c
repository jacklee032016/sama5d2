
#include "libCmn.h"
#include "libMux.h"
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


int testIgmp(void);
int testDipSwitch(void);
int testLedVideo(void);
int testLeds(void);
int testSw1(void);
int testSecureChipId(void);

static void _usage(char* base)
{
	printf("%s: \n\tTest command line utility.\n"\
		"\t%s -c command "
		"\t\t Current command:  " \
		"\n\t\t\t"TEST_CMD_IGMP", "TEST_CMD_DIP_SWITCH", "TEST_CMD_LEDS", "TEST_CMD_SW1", "TEST_CMD_SECURE_CHIP" \n", 
		  base, base);

	exit(-1);
}


int main(int argc, char *argv[])
{
	int opt;
	char		cmd[CMN_NAME_LENGTH];

	while ((opt = getopt (argc, argv, "c:")) != -1)
	{
		switch (opt)
		{

			case 'c': /* command */
				snprintf(cmd, sizeof(cmd), "%s", optarg);
				break;

			default:
				_usage(argv[0]);
		}

	}

//	res = cmnMuxPlayerParseConfig(MUX_PLAYER_CONFIG_FILE, &_cfg);

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
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_SECURE_CHIP))
	{
		testSecureChipId();
	}
	else
	{
		printf("Unknonw command '%s' is defined\n", cmd);
		_usage(argv[0]);
	}
		
	return 0;
}

