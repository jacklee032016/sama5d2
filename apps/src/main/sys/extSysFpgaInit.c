


#define	_PORT_BYTE_ORDER_CHANGE		0



typedef	struct _fpgaIntTable
{
	unsigned char		key;
	unsigned char		value;
}FpgaIntTable;


static const FpgaIntTable _frameRateTable[]=
{
	{
		2,
		EXT_V_FRAMERATE_T_23	
	},
	{
		3,
		EXT_V_FRAMERATE_T_24	
	},
	{
		5,
		EXT_V_FRAMERATE_T_25	
	},
	{
		6,
		EXT_V_FRAMERATE_T_29	
	},
	{
		7,
		EXT_V_FRAMERATE_T_30	
	},
	{
		9,
		EXT_V_FRAMERATE_T_50	
	},
	{
		0x0a,
		EXT_V_FRAMERATE_T_59	
	},
	{
		0x0b,
		EXT_V_FRAMERATE_T_60	
	}
};

static const FpgaIntTable _vDepthTable[]=
{
	{
		0,
		EXT_V_DEPTH_8	
	},
	{
		1,
		EXT_V_DEPTH_10	
	},
	{
		2,
		EXT_V_DEPTH_12	
	},
	{
		3,
		EXT_V_DEPTH_16	
	}
};

#if 0
static unsigned char _translateFrameRate(unsigned char rate, unsigned char isFromFpga)
{
	unsigned int i;
	for(i=0; i< EXT_ARRAYSIZE(_frameRateTable); i++)
	{
		if(isFromFpga )
		{
			if(_frameRateTable[i].key == rate )
				return _frameRateTable[i].value;
		}
		else
		{
			if(_frameRateTable[i].value == rate )
				return _frameRateTable[i].key;
		}
	}

	EXT_ERRORF("FrameRate %d is invalidate value for %s", rate, (isFromFpga)?"MCU":"FPGA" );
	
	if(isFromFpga)
		return EXT_V_DEPTH_8;
	return 0; /* depth 0 for FPGA */
}

static unsigned char _translateDepth(unsigned char depth, unsigned char isFromFpga)
{
	unsigned int i;
	for(i=0; i< EXT_ARRAYSIZE(_vDepthTable); i++)
	{
		if(isFromFpga )
		{
			if(_vDepthTable[i].key == depth )
				return _vDepthTable[i].value;
		}
		else
		{
			if(_vDepthTable[i].value == depth )
				return _vDepthTable[i].key;
		}
	}

	EXT_ERRORF("vDepth %d is invalidate value for %s", depth, (isFromFpga)?"MCU":"FPGA" );
	
	if(isFromFpga)
		return EXT_V_FRAMERATE_T_23;
	return 2; /* 23.9 for FPGA */
}
#endif

void extFpgaTimerJob(MuxRunTimeParam  *mediaParams)
{
	unsigned char value;


	_extFpgaReadByte(EXT_FPGA_REG_SDI_STATUS, &value);
	if(value == EXT_FPGA_FLAGS_SDI_CONNECTTED)
	{
		mediaParams->isConnect = EXT_TRUE;
	}
	else
	{
		mediaParams->isConnect = EXT_FALSE;
	}

	_extFpgaReadByte(EXT_FPGA_REG_PARAM_STATUS, &value);
	if(value != EXT_FPGA_FLAGS_PARAM_USABLE)
		return;

	EXT_DEBUGF(MUX_DEBUG_FPGA, "New Media Params is available now!");
	
//	extFpgaReadParams(mediaParams);
	/* clear register */
	_extFpgaWriteByte(EXT_FPGA_REG_PARAM_STATUS, &value);
	
	return;
}

void extI2cReset(void);

void	extFpgaEnable(char	isEnable)
{
	unsigned char		data = 0xFF;	/* enable all bits for different streams */
	if(isEnable == 0)
	{
		data = 0;
	}

	_fpgaRegisterWrite(EXT_FPGA_REG_ENABLE, &data, 1);
}


/* Reload FPGA from SPI memory */
char  extBspFpgaReload(void)
{
#define	_TIMEOUT_VALUE		20
	char		isOK = EXT_FALSE;
	unsigned char data;
	int timeoutSecond = _TIMEOUT_VALUE;	/* when compare with negative number, it must be int, not char */

	bspHwClockInit();

	 /* active-LOW reset to configure FPGA */
//	gpio_set_pin_high(EXT_PIN_FPGA_PROGRAM);
//	EXT_DELAY_MS(50);
	if(extRun.isConfigFpga )
	{
		EXT_INFOF(("Reconfiguring FPGA....."EXT_NEW_LINE));
		gpio_set_pin_low(EXT_PIN_FPGA_PROGRAM);
		
#if 1	
		EXT_DELAY_MS(10);
#else
		isOK = bspFpgaWaitDone(20);
		EXT_DELAY_MS(5000);
#endif	
	}
	else
	{
		EXT_INFOF(("Test version: FPGA is not loaded by MCU"EXT_NEW_LINE));
	}


	gpio_set_pin_high(EXT_PIN_FPGA_PROGRAM);
	
	EXT_DELAY_MS(50);

	/* wait for FPGA done pin
	* up to 20 second is the secondary image is loaded 
	*/
	isOK = bspFpgaWaitDone(20);
	
	printf("FPGA load image %s"EXT_NEW_LINE, (isOK== EXT_FALSE)?"failed":"sucessed");

	printf("Waiting FPGA ....."EXT_NEW_LINE);
	EXT_DELAY_US(1000*500);
	do
	{
		isOK = FPGA_I2C_READ(EXT_FPGA_REG_ETHERNET_RESET, &data, 1);
		timeoutSecond--;
		
//		EXT_ERRORF("Timeout %d seconds when waiting FPGA status10"EXT_NEW_LINE, timeoutSecond);
		if(timeoutSecond<0)
		{
//			EXT_ERRORF("Timeout %d seconds when waiting FPGA status0"EXT_NEW_LINE, _TIMEOUT_VALUE);
			break;
		}
		EXT_DELAY_US(1000*100);
	}while(isOK!= EXIT_SUCCESS);

	if(isOK == EXIT_SUCCESS)
	{
		printf("FPGA Ethernet Reset register %d : %2x"EXT_NEW_LINE, EXT_FPGA_REG_ETHERNET_RESET, data);

		_extFpgaReadByte(EXT_FPGA_REG_ETHERNET_RESET, &data);
		data = data||0x01;
		_extFpgaWriteByte(EXT_FPGA_REG_ETHERNET_RESET, &data);

#if 0
	/* test I2C when LMH1983 is not working. J.L. April 10, 2018 */
	bspBistI2cSensor(extBspCLIGetOutputBuffer(), EXT_COMMAND_BUFFER_SIZE);
	bspBistI2cEeprom(extBspCLIGetOutputBuffer(), EXT_COMMAND_BUFFER_SIZE);
#endif
	}
	else
	{
		EXT_ERRORF("Timeout %d seconds when waiting FPGA status"EXT_NEW_LINE, _TIMEOUT_VALUE);
	}
	
	return isOK;
}

#if 0
void extFpgaInit(void)
{
	EXT_FPGA_VER ver;
//	int timeout = 20;
	int i;
	unsigned char data;
	unsigned char val;
	
//	bspHwClockInit();
	for(i=0; i< 8; i++)
	{
		val =(unsigned char)i;
		_extFpgaWriteByte(i, &val);
		_extFpgaReadByte(i, &data);
		printf("FPGA register %d : %s (%d)"EXT_NEW_LINE, i, (val==data)?"OK":"Failed", data);
	}

	for(i=8; i< 16; i++)
	{
		_extFpgaReadByte(i, &data, 1);
		printf("FPGA register %d : 0x%2x"EXT_NEW_LINE, i, data);
	}
	
#if 0	
	while(FPGA_I2C_READ(FPGA_VERSION, (unsigned char *)&ver, 3 ) != EXIT_SUCCESS)
	{
//		wdt_enable(&watchdog_opt);
		EXT_DELAY_MS(1000);
		timeout--;
		if (timeout == 0)
		{
			EXT_ERRORF("FPGA read version timeout");
			break;	// too long , exit loop
		}
	}
#endif

	printf("FPGA %s, version:%d, revision:%d, device:%d"EXT_NEW_LINE, (ver.rx_tx== EXT_FALSE)?"RX":"TX", ver.version, ver.revision, ver.fpgaDevice);

}
#endif



