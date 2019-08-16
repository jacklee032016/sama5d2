
#include "libCmnSys.h"
#include "_cmnMux.h"

#include "mux7xx.h"

struct DATA_BUFFER 	_readBuf;


int _rs232SendHexStr(char *str )
{
	int len = strlen(str);
	int i;
	unsigned char value;


#if EXT_RS232_DEBUG
	EXT_DEBUGF(EXT_RS232_DEBUG, "Sent to RS232:");
#endif
	for(i=0; i< len/2; i++)
	{
		if(extSysAtoInt8(str+i*2, &value)== EXIT_FAILURE)
		{
			EXT_ERRORF("'%.*s' is not an integer", 2, str+i*2);
			return EXIT_FAILURE;
		}
		
#if EXT_RS232_DEBUG
		printf("%2X ", value);
#endif

#ifdef	ARM
		cmnSysRs232Write(&value, 1);
#endif
		
	}
#if EXT_RS232_DEBUG
	printf(""EXT_NEW_LINE);
#endif


	return EXIT_SUCCESS;
}

/* return buffer length */
static int _rs232StartRead(int timeout, char *buf)
{/* init IRQ, index of buffer */
	int ret = 0;
	int	i;
	struct DATA_BUFFER *readBuf = &_readBuf;

//	u32_t timeElapes = 0;
#ifdef	ARM
	int	timeElapes = sys_arch_sem_wait(&rs232Dev->rxSema, (timeout<= 0)?rs232Dev->timeoutMs:timeout);
	if(timeElapes == SYS_ARCH_TIMEOUT)
#else
	memset(readBuf, 0 , sizeof(struct DATA_BUFFER));
	readBuf->size = sizeof(readBuf->buffer);
	readBuf->writeIndex = 10;
	readBuf->buffer[0] = 0x09;
	readBuf->buffer[1] = 0x87;
	readBuf->buffer[2] = 0x65;
	readBuf->buffer[3] = 0x43;
	readBuf->buffer[4] = 0x21;
	readBuf->buffer[5] = 0xFE;
	readBuf->buffer[6] = 0xDC;
	readBuf->buffer[7] = 0xBA;
	readBuf->buffer[8] = 0x23;
	readBuf->buffer[9] = 0x32;
	if(1)
#endif
	{
#if EXT_RS232_DEBUG
		EXT_DEBUGF(EXT_RS232_DEBUG, "Timeout RS232, RX: Total %d rxed", readBuf->writeIndex );
#endif
		for(i=0; i< readBuf->writeIndex; i++)
		{
			extBcd2Ascii(readBuf->buffer[i], buf+i*2);
		}	

		ret = readBuf->writeIndex*2;
	}
	else
	{
		EXT_ERRORF("Wakeup RS232 RX" );
		ret = EXIT_FAILURE;
	}

	/* Disable interrupt  */

	return ret;
}

/* used by RS232 and IR */
int	cmnMuxObjectParseHexaData(struct DATA_CONN *dataConn, cJSON *dataObj, char *cmdName)
{
	int	intValue;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;
	
	if(cmnJsonGetStrIntoString(dataObj, FIELD_RS232_DATA, rxCfg->setupData.hexData, sizeof(rxCfg->setupData.hexData)) == EXIT_FAILURE)
	{/* no data, so no process is needed */
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is not found", FIELD_RS232_DATA, cmdName );
		return EXIT_FAILURE;
	}

	if(strlen(rxCfg->setupData.hexData)%2 != 0)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "'%s' error: data size '%d' is not even number", FIELD_RS232_DATA, strlen(rxCfg->setupData.hexData) );
		return EXIT_FAILURE;
	}

	
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_IS_FEEDBACK);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->setupData.isFeedBack = (intValue)?1:0;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_WAIT_TIME);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if(intValue < 0)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is wrong", FIELD_RS232_WAIT_TIME, intValue, cmdName);
			return EXIT_FAILURE;
		}
		rxCfg->setupData.waitMs = intValue;
	}

	return EXIT_SUCCESS;
}


/* send and receive data from RS232 port */
int	cmnMuxSendRsData(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	int size = 0;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;
	
	if(MUX_MAIN_IS_DEBUG_MSG(muxMain))
	{
		MUX_INFO("RS232 Data: hexdata:'%s'; feedback:%d; waitMs:%d", rxCfg->setupData.hexData, rxCfg->setupData.isFeedBack, rxCfg->setupData.waitMs);
	}

//	EXT_DEBUGF(EXT_IPCMD_DEBUG, ("RS Data: hexdata:'%s'; feedback:%d; waitMs:%d",parser->setupData.hexData, parser->setupData.isFeedBack, parser->setupData.waitMs));


	rs232StartRx();
	
	if(_rs232SendHexStr(rxCfg->setupData.hexData) == EXIT_FAILURE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR,"'%s' is not validate when sent to RS232", rxCfg->setupData.hexData);
		rs232StopRx();
		return EXIT_FAILURE;
	}

	if(!rxCfg->setupData.isFeedBack )
	{/* default dataConn->errCode ==200 */
		rs232StopRx();
		return EXIT_SUCCESS;
	}

	memset(rxCfg->setupData.hexData, 0, HEX_DATA_MAX_LENGTH);
	size = _rs232StartRead(rxCfg->setupData.waitMs, rxCfg->setupData.hexData);
	rs232StopRx();
	if(size <= 0)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR,"No data is replied from RS232");
		return EXIT_FAILURE;
	}
	
	
	if(MUX_MAIN_IS_DEBUG_MSG(muxMain))
	{
		MUX_INFO("RS232: " EXT_IPCMD_RS232_DATA_FEEDBACK": '%s'", rxCfg->setupData.hexData);
	}

	JEVENT_ADD_STRING(dataConn->dataObj, EXT_IPCMD_RS232_DATA_FEEDBACK, rxCfg->setupData.hexData);
	
	return EXIT_SUCCESS;
}


static int _irSendHexStr(char *str )
{
	int len = strlen(str);
	int i;
	unsigned char value;


#if EXT_RS232_DEBUG
	EXT_DEBUGF(EXT_RS232_DEBUG, "Sent to IR:");
#endif
	for(i=0; i< len/2; i++)
	{
		if(extSysAtoInt8(str+i*2, &value)== EXIT_FAILURE)
		{
			EXT_ERRORF("'%.*s' is not an integer", 2, str+i*2);
			return EXIT_FAILURE;
		}
		
#if EXT_RS232_DEBUG
		printf("%2X ", value);
#endif

#ifdef	ARM
		cmnSysRs232Write(&value, 1);
#endif
		
	}
#if EXT_RS232_DEBUG
	printf(""EXT_NEW_LINE);
#endif


	return EXIT_SUCCESS;
}


/* return buffer length */
static int _irStartRead(int timeout, char *buf)
{/* init IRQ, index of buffer */
	int ret = 0;
	int i;

	struct DATA_BUFFER *readBuf = &_readBuf;
	
#ifdef	ARM
	int	timeElapes = sys_arch_sem_wait(&rs232Dev->rxSema, (timeout<= 0)?rs232Dev->timeoutMs:timeout);
	if(timeElapes == SYS_ARCH_TIMEOUT)
#else
	memset(readBuf, 0 , sizeof(struct DATA_BUFFER));
	readBuf->size = sizeof(readBuf->buffer);
	readBuf->writeIndex = 10;
	readBuf->buffer[0] = 0x09;
	readBuf->buffer[1] = 0x87;
	readBuf->buffer[2] = 0x65;
	readBuf->buffer[3] = 0x43;
	readBuf->buffer[4] = 0x21;
	readBuf->buffer[5] = 0xFE;
	readBuf->buffer[6] = 0xDC;
	readBuf->buffer[7] = 0xBA;
	readBuf->buffer[8] = 0x12;
	readBuf->buffer[9] = 0x21;
	if(1)
#endif
	{
#if EXT_RS232_DEBUG
		EXT_DEBUGF(EXT_RS232_DEBUG, "Timeout IR, RX: Total %d rxed", readBuf->writeIndex );
#endif
		for(i=0; i< readBuf->writeIndex; i++)
		{
			extBcd2Ascii(readBuf->buffer[i], buf+i*2);
		}	

		ret = readBuf->writeIndex*2;
	}
	else
	{
		EXT_ERRORF("Wakeup IR RX");
		ret = EXIT_FAILURE;
	}


	return ret;
}


/* send and receive data from IR port */
int	cmnMuxSendIRData(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;
	
	if(MUX_MAIN_IS_DEBUG_MSG(muxMain))
	{
		MUX_INFO("IR Data: hexdata:'%s'; feedback:%d; waitMs:%d", rxCfg->setupData.hexData, rxCfg->setupData.isFeedBack, rxCfg->setupData.waitMs);
	}

//	EXT_DEBUGF(EXT_IPCMD_DEBUG, ("RS Data: hexdata:'%s'; feedback:%d; waitMs:%d",parser->setupData.hexData, parser->setupData.isFeedBack, parser->setupData.waitMs));

	if(_irSendHexStr(rxCfg->setupData.hexData) == EXIT_FAILURE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR,"'%s' is not validate when sent to IR", rxCfg->setupData.hexData);
		return EXIT_FAILURE;
	}

	if(!rxCfg->setupData.isFeedBack )
	{/* default dataConn->errCode ==200 */
		return EXIT_SUCCESS;
	}

	memset(rxCfg->setupData.hexData, 0, HEX_DATA_MAX_LENGTH);
	if(_irStartRead(rxCfg->setupData.waitMs, rxCfg->setupData.hexData) <= 0)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR,"No data is replied from IR");
		return EXIT_FAILURE;
	}
	
	
	if(MUX_MAIN_IS_DEBUG_MSG(muxMain))
	{
		MUX_INFO("IR:" EXT_IPCMD_RS232_DATA_FEEDBACK": '%s'", rxCfg->setupData.hexData);
	}

	JEVENT_ADD_STRING(dataConn->dataObj, EXT_IPCMD_RS232_DATA_FEEDBACK, rxCfg->setupData.hexData);
	
	return EXIT_SUCCESS;
}


