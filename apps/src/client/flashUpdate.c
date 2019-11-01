/*
* Update FPGA binary into second flash 
*/

#include "libCmnSys.h"

#define CMN_MODULE_FW_UPDATE			"fwUpdate"

static void usage(char* base)
{
	printf("%s: \n\tCommand line interface for flash operation.\n"\
		"\t%s -a ADDRESS -f BINARY_FILE -p PAGE_COUNT(read)|SECTOR_COUNT(erase) -c r(read, default)/e(rase)/w(rite)\n"\
		"\t\t w: write BINARY_FILE into ADDRESS; default: read 2 pages data from 0x0000,0000\n"\
		"\t\t ADDRESS, hexa, default 0x0000,0000; PAGE_COUNT|SECTOR_COUNT: default 2\n", 
		  base, base);
}

int main(int argc, char *argv[])
{
#define	_CMD_READ			0
#define	_CMD_ERASE		1
#define	_CMD_WRITE		2

	int fd;
	uint32_t startAddress = 0;
	uint32_t pageCount = 2;
	char	fileName[256];
	char	cmd[256];
	int isWrite = EXT_FALSE;
	int opt;
	int cmdType = _CMD_READ;
	

	memset(fileName, 0, sizeof(fileName));
	memset(cmd, 0, sizeof(cmd));

	if (argc < 2)
	{
		usage(argv[0]);
	}	

	while ((opt = getopt (argc, argv, "c:a:f:p:")) != -1)
	{
		switch (opt)
		{
			case 'a':/*start address*/
				startAddress =  cmnParseGetHexIntValue(optarg);// atoi(optarg);
				EXT_DEBUGF(EXT_DBG_OFF, "Raw address: \t0x%08X", startAddress);
				break;

			case 'f': /* firmware file name */
				snprintf(fileName, sizeof(fileName), "%s",optarg);
				EXT_DEBUGF(EXT_DBG_OFF, "Write file: \t'%s'", fileName);
				break;

			case 'p':/* Page Count */
				pageCount = atoi(optarg);
				EXT_DEBUGF(EXT_DBG_OFF, "Page count: \t%d", pageCount );
				break;
				
			case 'c': /* write */
				snprintf(cmd, sizeof(cmd), "%s",optarg);
				if(strstr(cmd, "w") )
				{
					cmdType = _CMD_WRITE;
				}
				else if(strstr(cmd, "e") )
				{
					cmdType = _CMD_ERASE;
				}
				
				break;

			default:
				EXT_DEBUGF(EXT_DBG_OFF, "opt '%c'", opt);
				usage(argv[0]);
				exit(1);
		}

	}

	
	printf(CMN_VERSION_INFO(CMN_MODULE_FW_UPDATE) EXT_NEW_LINE );
	EXT_DEBUGF(EXT_DBG_OFF, "\tCommamd:\t%s", (cmdType == _CMD_READ)?"READ":(cmdType==_CMD_WRITE)?"WRITE":"ERASE" );


	if( strlen(fileName)> 0 && (cmn_count_file(fileName)< 0) )
	{
		MUX_ERROR("FPGA firmware '%s' can not be open for write flash");
		return -1;
	}

	if((startAddress %SPI_FLASH_SECTOR_SIZE) != 0)
	{
		uint32_t tmp = (startAddress/SPI_FLASH_SECTOR_SIZE);
		MUX_INFO("Start address: 0x%08x must aligned at sector border to 0x%08x", startAddress, tmp*SPI_FLASH_SECTOR_SIZE );
		startAddress = tmp*SPI_FLASH_SECTOR_SIZE;
	}

	fd = cmnSysRawSpiFlashInit();
	if( fd < 0)
	{
		MUX_ERROR("Flash Initialization failed");
		return -1;
	}

	if(cmdType == _CMD_WRITE)
	{
		cmnSysRawSpiFlashWriteFile(fd, fileName, startAddress);
	}
	else if(cmdType == _CMD_ERASE )
	{
		cmnSysRawSpiFlashEraseSectors(fd, pageCount, startAddress);
	}
	else
	{
		cmnSysRawSpiFlashRead(fd, startAddress, pageCount);	
	}

	cmnSysRawSpiFlashClose(fd);
	
	EXT_INFOF(EXT_NEW_LINE"Finished");
	return 0;
}

