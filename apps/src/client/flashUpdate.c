/*
* Update FPGA binary into second flash 
*/

#include "libCmnSys.h"

#define CMN_MODULE_FW_UPDATE			"fwUpdate"

static void usage(char* base)
{
	printf("%s: \n\tCommand line interface for flash operation.\n"\
		"\t%s -a ADDRESS -f BINARY_FILE -p PAGE_COUNT -w\n"\
		"\t\t w: write BINARY_FILE into ADDRESS; default: read 2 pages data from 0x0000,0000\n"\
		"\t\t ADDRESS, hexa, default 0x0000,0000; PAGE_COUNT: default 2\n", 
		  base, base);
}

int main(int argc, char *argv[])
{
	int fd;
	uint32_t startAddress = 0;
	uint32_t pageCount = 2;
	char	fileName[256];
	int isWrite = EXT_FALSE;
	int opt;
	

	memset(fileName, 0, sizeof(fileName));

	if (argc < 2)
	{
		usage(argv[0]);
	}	

	while ((opt = getopt (argc, argv, "wa:f:p:")) != -1)
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
				
			case 'w': /* write */
				isWrite = EXT_TRUE;
				EXT_DEBUGF(EXT_DBG_OFF, "Write Flash...");
				break;

			default:
				EXT_DEBUGF(EXT_DBG_OFF, "opt '%c'", opt);
				usage(argv[0]);
				exit(1);
		}

	}

	
	printf(CMN_VERSION_INFO(CMN_MODULE_FW_UPDATE) EXT_NEW_LINE );


	if( isWrite && (cmn_count_file(fileName)< 0) )
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

	if(isWrite )
	{
		cmnSysRawSpiFlashWriteFile(fd, fileName, startAddress);
	}
	else
	{
		cmnSysRawSpiFlashRead(fd, startAddress, pageCount);	
	}

	cmnSysRawSpiFlashClose(fd);
	
	EXT_INFOF(EXT_NEW_LINE"OK");
	return 0;
}

