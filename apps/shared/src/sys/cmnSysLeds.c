/*
* LEDs, button and W1 bus
*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>	
#include <unistd.h>
#include <string.h>

#include "libCmnSys.h"
#include "mux7xx.h"


#define	LED_BRIGHTNESS			"brightness"
/* write  "default-on" to trigger */
#define	LED_TRIGGER			"trigger"

#define	LED_TRIGGER_NONE			"none"
#define	LED_TRIGGER_TIMER			"timer"
#define	LED_TRIGGER_HEARTBEAT		"heartbeat"


#define	LED_DELAY_ON			"delay_on"
#define	LED_DELAY_OFF			"delay_off"

#define	LED_POWER				LED_HOME"/""power"
#define	LED_VIDEO				LED_HOME"/""video"
#define	LED_ACT				LED_HOME"/""act"

#define	LED_LINK				LED_HOME"/""link"

/* 11.25, 2019, send PTP signal to FPGA to notify new timestamp is arriving */
#define	LED_SIGNAL				LED_HOME"/""signal"


struct _SysLedCtrl
{
	LED_TYPE_T		led;
	char				*name;
	char				*brightness;
	char				*trigger;
	char				*triggerCheck;	/* check trigger is ON/OFF */
};

#define	_LED_DEBUG		EXT_DBG_OFF

static struct _SysLedCtrl  _sysLeds[] =
{
	{
		.led = LED_TYPE_POWER,
		.name = "POWER",	
		.brightness = 	LED_POWER"/"LED_BRIGHTNESS,
		.trigger = 	LED_POWER"/"LED_TRIGGER,
		.triggerCheck = 	LED_POWER"/"LED_DELAY_ON
	},
	{
		.led = LED_TYPE_VIDEO,
		.name = "VIDEO",	
		.brightness = 	LED_VIDEO"/"LED_BRIGHTNESS,
		.trigger = 	LED_VIDEO"/"LED_TRIGGER,
		.triggerCheck = 	LED_VIDEO"/"LED_DELAY_ON
	},
	{
		.led = LED_TYPE_ACT,
		.name = "ACT",	
		.brightness = 	LED_ACT"/"LED_BRIGHTNESS,
		.trigger = 	LED_ACT"/"LED_TRIGGER,
		.triggerCheck = 	LED_ACT"/"LED_DELAY_ON
	},
	{
		.led = LED_TYPE_LINK,
		.name = "LINK",	
		.brightness = 	LED_LINK"/"LED_BRIGHTNESS,
		.trigger = 	LED_LINK"/"LED_TRIGGER,
		.triggerCheck = 	LED_LINK"/"LED_DELAY_ON
	},
	{
		.led = LED_TYPE_SIGNAL,
		.name = "SIGNAL",	
		.brightness = 	LED_SIGNAL"/"LED_BRIGHTNESS,
		.trigger = 	LED_SIGNAL"/"LED_TRIGGER,
		.triggerCheck = 	LED_SIGNAL"/"LED_DELAY_ON
	},
	{
		.led = LED_TYPE_UNKNOWN,
		.name = NULL,	
		.brightness = 	NULL,
		.trigger = 	NULL,
		.triggerCheck = 	NULL
	}
};

/* ON/OFF when trigger is disabled */
static int _ledCtrl(char *filename, int isOn)
{
	int fd;
	char _chValue = '0';
	
	fd = open(filename, O_WRONLY);
	if( fd < 0)
	{
		EXT_ERRORF("Open '%s' failed: %m", filename);
		return EXIT_FAILURE;
	}

	if(isOn)
	{
		_chValue = '1';
	}

	if(write(fd, &_chValue, sizeof(_chValue)) < 0)
	{
		EXT_ERRORF("write '%s' failed: %m", filename);
		close(fd);
		return EXT_FALSE;
	}
	
	close(fd);

	return EXIT_SUCCESS;
}


/* ON/OFF when trigger is disabled */
static int _ledCtrlCheck(char *filename)
{
	int fd;
	char _chValue = 0;
	
	fd = open(filename, O_RDWR);
	if( fd < 0)
	{
		EXT_ERRORF("Open '%s' failed: %m", filename);
		return EXT_FALSE;
	}

	if(read(fd, &_chValue, sizeof(_chValue)) < 0 )
	{
		EXT_ERRORF("read '%s' failed: %m", filename);
		close(fd);
		return EXT_FALSE;
	}

	EXT_DEBUGF(_LED_DEBUG, "read '%c(%d)' out from %s", _chValue, _chValue, filename );

	close(fd);

	return (_chValue=='0')?EXT_FALSE:EXT_TRUE;
}


static int _ledBlinkCtrl(char *filename, int isOn)
{
	int fd;
	int res = EXIT_SUCCESS;
	char *trigger = LED_TRIGGER_NONE;
	
	fd = open(filename, O_WRONLY);
	if( fd < 0)
	{
		EXT_ERRORF("Open '%s' failed: %m", filename);
		return EXIT_FAILURE;
	}

	if(isOn)
	{
		trigger = LED_TRIGGER_TIMER;
	}

	if(write(fd, trigger, strlen(trigger) )< 0)
	{
		EXT_ERRORF("Make '%s' to %s failed", filename, (isOn)?"On":"Off");
		res =  EXIT_FAILURE;
	}

	close(fd);
	return res;
}


static int _ledBlinkCheck(char *filename)
{
	int fd;
	fd = open(filename, O_WRONLY);
	if( fd < 0)
	{
		return EXT_FALSE;/* trigger is disabled */
	}

	close(fd);
	return EXT_TRUE;
}


#define	_LED_CHECK_BLINK(led)		\
	(_ledBlinkCheck( (led)->triggerCheck))


#define	_LED_BLINK(led, isOn)	\
	( _ledBlinkCtrl( (led)->trigger, (isOn)) )


#define	_LED_CTRL(led, isOn)	\
	( _ledCtrl( (led)->brightness, (isOn)) )

#define	_LED_CHECK(led)	\
	( _ledCtrlCheck( (led)->brightness) )


#if 0
static int _sysLedOnOff(struct _SysLedCtrl *_led, int isOn)
{
	if(_ledBlinkCheck(_led->triggerCheck) == EXT_TRUE && isOn == 0)
	{/* blink, first */
		EXT_INFOF("LED %s is blink now");
		_ledBlinkCtrl(_led->trigger, EXT_FALSE);
	}

	_ledCtrl(_led->brightness, isOn);

	return EXIT_SUCCESS;
}

static int _sysLedBlink(struct _SysLedCtrl *_led)
{
	_ledCtrl(_led->brightness, EXT_FALSE);

	if(_ledBlinkCheck(_led->triggerCheck) == EXT_TRUE )
	{/* blink, first */
		EXT_INFOF("LED %s is blink now");
	}

	_ledBlinkCtrl(_led->trigger, EXT_TRUE);

	return EXIT_SUCCESS;
}
#endif

static struct _SysLedCtrl *_findLed(LED_TYPE_T type)
{
	struct _SysLedCtrl *_led = _sysLeds;

	while(_led)
	{
		if(_led->led == type)
		{
			return _led;
		}
		_led++;
	}

	return NULL;
}


LED_MODE_T cmnSysLedCheck(LED_TYPE_T type)
{
	struct _SysLedCtrl *led= _findLed(type);
	if(led == NULL)
	{
		EXT_ERRORF("LED type %d is not correct", type);
		return EXIT_FAILURE;
	}

	if(_LED_CHECK_BLINK(led)== EXT_TRUE )
	{
		return LED_MODE_BLINK;
	}

	if(_LED_CHECK(led)== EXT_TRUE)
	{
		return LED_MODE_ON;
	}

	return LED_MODE_OFF;
}

char *cmnSysLedFindState(LED_MODE_T mode)
{
	switch (mode)
	{
		case LED_MODE_OFF:
			return "OFF";
		case LED_MODE_ON:
			return "ON";
		case LED_MODE_BLINK:
			return "BLINK";
		default:
			return "Unknown";
	}		

	return "Unknown";
}

#if _LED_DEBUG
#endif

int cmnSysLedCtrl(LED_TYPE_T type, LED_MODE_T mode)
{
	struct _SysLedCtrl *led= _findLed(type);
	LED_MODE_T currentMode;

	if(led == NULL)
	{
		EXT_ERRORF("LED type %d is not correct", type);
		return EXIT_FAILURE;
	}

	currentMode = cmnSysLedCheck(type);
	EXT_DEBUGF(_LED_DEBUG, "LED %s is on state of %s", led->name, cmnSysLedFindState(currentMode) );
	if(currentMode == mode)
	{
//		EXT_INFOF("LED %s has been in state of %s", led->name, cmnSysLedFindState(mode) );
//		return EXIT_FAILURE;
	}

	if(currentMode == LED_MODE_BLINK )
	{/* first stop blink */
		EXT_DEBUGF(_LED_DEBUG, "LED %s stop BLINK", led->name);
		_LED_BLINK(led, EXT_FALSE);
	}

	if(mode== LED_MODE_BLINK)
	{
		EXT_DEBUGF(_LED_DEBUG, "LED %s start BLINK", led->name);
		return _LED_BLINK(led, EXT_TRUE);
	}

	EXT_DEBUGF(_LED_DEBUG, "LED %s set %s", led->name, (mode== LED_MODE_ON)?"ON":"OFF" );
	return _LED_CTRL(led, (mode== LED_MODE_ON)?EXT_TRUE:EXT_FALSE);
}



#include <linux/input.h> /* struct input_event */
  
/* return BTN_EVENT_T. No event happen, it blocks */  
int cmnSysSW1CheckEvent(void)
{
	struct input_event _event;	
	int fd;
	int size = sizeof(_event);
	int event = BTN_EVENT_NONE;
    
    /* Read event0 */
	fd = open(INPUT_EVENT_SW1, O_RDONLY);
	if(fd < 0)
	{
		EXT_ERRORF("Open " INPUT_EVENT_SW1 " failed: %m");
		return BTN_EVENT_UNKNOWN;
	}
	
	/* Read and parse event, update global variable */
//	while (1)
	{
		if (read(fd, &_event, size) < size)
		{
			EXT_ERRORF("Reading from " INPUT_EVENT_SW1 " failed: %m");
			close(fd);
			return BTN_EVENT_UNKNOWN;
		}

		if (_event.code == KEY_DOWN && _event.type == EV_KEY )
		{
			EXT_INFOF("SW1 is %s", (_event.value == 0)?"released": "pressed");
			event = (_event.value == 0)?BTN_EVENT_RELEASED:BTN_EVENT_PRESSED;
//			isPressed = EXT_TRUE;
		}
		
//		usleep(1000);
	}

	close(fd);

	return event;
}


int cmnSysDipSwitchCheck(void)
{
	int fd;
	int r;

#define	_SUPPORT_MAX		0

#if _SUPPORT_MAX
#define	DIP_COUNT			SW_MAX
#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
	unsigned long state[NBITS(DIP_COUNT)];	/* every switch occupy one bit */
#else
#define	DIP_COUNT			4
	unsigned long state;	/* every switch occupy one bit, maximum of switch is 15, SW_MAX */
#endif	


	fd = open(INPUT_EVENT_DIP_SWITCH, O_RDONLY);
	if (fd < 0)
	{
		EXT_ERRORF("open DIP SWITCH '%s' failed: %m", INPUT_EVENT_DIP_SWITCH);
		return INVALIDATE_VALUE_U32;
	}
	
#if _SUPPORT_MAX
	memset(state, 0, sizeof(state));
	r = ioctl(fd, EVIOCGSW(DIP_COUNT), state);	/* query 4 bits */
#else
	memset(&state, 0, sizeof(state));
	r = ioctl(fd, EVIOCGSW(DIP_COUNT), &state);	/* query 4 bits */
#endif	
	close(fd);

	if (r == -1)
	{
		EXT_ERRORF("ioctl DIP SWITCH %s failed: %m", INPUT_EVENT_DIP_SWITCH);
		return INVALIDATE_VALUE_U32;
	}

#if _SUPPORT_MAX
	CMN_HEX_DUMP((void *)state, sizeof(state), "DIP Status");
#else
	CMN_HEX_DUMP((void *)&state, sizeof(state), "DIP Status");
#endif
	//EXT_DEBUGF(_LED_DEBUG, ("state is 0x%x", state));

	//if (test_bit(keycode, state))
	//	return 10; /* different from EXIT_FAILURE */
	//else
	return state;
}


