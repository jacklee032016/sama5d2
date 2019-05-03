
#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"

#include "_cmnMux.h"

unsigned char rs232SendHexStr(char *str )
{
	int len = strlen(str);
	int i;
	unsigned char value;


#if EXT_RS232_DEBUG
	printf("Sent to RS232:"EXT_NEW_LINE);
#endif
	for(i=0; i< len/2; i++)
	{
		if(extSysAtoInt8(str+i*2, &value)== EXIT_FAILURE)
		{
			EXT_ERRORF(("'%.*s' is not an integer", 2, str+i*2));
			return EXIT_FAILURE;
		}
		
#if EXT_RS232_DEBUG
		printf("%2X ", value);
#endif

#ifdef	ARM
		extRs232Write(&value, 1);
#endif
		
	}
#if EXT_RS232_DEBUG
	printf(""EXT_NEW_LINE);
#endif


	return EXIT_SUCCESS;
}


int extIpCmdSendRsData(struct DATA_CONN *dataConn)
{
	char ret;
	int index = 0;
	char *data = NULL;
	int size = 0;
	MuxMain *muxMain = SYS_MAIN(dataConn);

	if(cmnJsonGetStrIntoString(dataConn->dataObj, EXT_IPCMD_RS232_DATA_HEX, muxMain->setupData.hexData, sizeof(muxMain->setupData.hexData)) == EXIT_FAILURE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No '%s' is found or it is invalidate value", EXT_IPCMD_RS232_DATA_HEX);
		return EXIT_FAILURE;
	}
	
	muxMain->setupData.waitMs = cmnGetIntegerFromJsonObject(dataConn->dataObj, EXT_IPCMD_RS232_WAIT_TIME);
	if(muxMain->setupData.waitMs == INVALIDATE_VALUE_U16 )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No '%s' is found or it is invalidate value", EXT_IPCMD_RS232_WAIT_TIME);
		return EXIT_FAILURE;
	}

	muxMain->setupData.isFeedBack = cmnGetIntegerFromJsonObject(dataConn->dataObj, EXT_IPCMD_RS232_FEEDBACK);
	if(muxMain->setupData.isFeedBack == INVALIDATE_VALUE_U8 )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No '%s' is found or it is invalidate value", EXT_IPCMD_RS232_FEEDBACK);
		return EXIT_FAILURE;
	}
	
	if(EXT_DEBUG_UDP_CMD_IS_ENABLE())
	{
		printf("RS232 Data: hexdata:'%s'; feedback:%d; waitMs:%d"EXT_NEW_LINE, muxMain->setupData.hexData, muxMain->setupData.isFeedBack, muxMain->setupData.waitMs);
	}

//	EXT_DEBUGF(EXT_IPCMD_DEBUG, ("RS Data: hexdata:'%s'; feedback:%d; waitMs:%d",parser->setupData.hexData, parser->setupData.isFeedBack, parser->setupData.waitMs));

	rs232StartRx();
	
	if(rs232SendHexStr(muxMain->setupData.hexData) == EXIT_FAILURE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "'%s' is not validate when sent to RS232", muxMain->setupData.hexData );
		return EXIT_FAILURE;
	}

	if(!muxMain->setupData.isFeedBack )
	{
		muxMain->status = JSON_STATUS_OK;
		rs232StopRx();
		return extIpCmdResponseReply(parser);
	}

	/* read feedback */
	memset(muxMain->setupData.hexData, 0, HEX_DATA_MAX_LENGTH);
	size = rs232StartRead(muxMain->setupData.waitMs, muxMain->setupData.hexData);
	if(size <= 0)
	{
		muxMain->status = JSON_STATUS_CMD_EXEC_ERROR;
		rs232StopRx();
		return extIpCmdResponseReply(parser);
	}
	
	extIpCmdResponseHeaderPrint( parser);
	data = muxMain->outBuffer+IPCMD_HEADER_LENGTH + muxMain->outIndex;
	size = muxMain->outSize - IPCMD_HEADER_LENGTH - muxMain->outIndex;
	
	if(EXT_DEBUG_UDP_CMD_IS_ENABLE())
	{
		printf(EXT_IPCMD_RS232_DATA_FEEDBACK": '%s'"EXT_NEW_LINE, parser->setupData.hexData);
	}
	
	index += snprintf(data+index, size-index, ",\""EXT_IPCMD_DATA_ARRAY"\":[{");
	index += snprintf(data+index, size-index, "\""EXT_IPCMD_RS232_DATA_FEEDBACK"\":\"%s\"", muxMain->setupData.hexData);
	index += snprintf(data+index, size-index, "}]" );

	muxMain->outIndex += index;
	
	muxMain->status = JSON_STATUS_OK;
	extIpCmdResponseTailCalculate(muxMain, EXT_FALSE);

	rs232StopRx();
	
	return EXIT_SUCCESS;

}

