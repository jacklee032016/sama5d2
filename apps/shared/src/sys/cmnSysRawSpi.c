/*
* SPI flash access in user space based on /dev/spi0xxx
* 09.16, 2019, Jack Lee
*/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <math.h>

#include "libCmn.h"
#include "extSysParams.h"
#include "libCmnSys.h"
#include "mux7xx.h"


#define	_DEBUG_RAW_SPI		EXT_DBG_OFF

#define	_RAW_SPI_TITLE			"SPI_Flash: "

//#define SPI_DEVICE 	"/dev/spidev32766.0"
#define SPI_CMD_WRITE_STATUS_REGISTER				0x01
#define SPI_CMD_WRITE								0x02
#define SPI_CMD_READ								0x03
#define SPI_CMD_WRITE_DISABLE						0x04
#define SPI_CMD_READ_STATUS						0x05
#define SPI_CMD_WRITE_ENABLE						0x06
#define SPI_CMD_CHIP_ERASE							0xC7
#define SPI_CMD_CLEAR_AREA							0xD8
#define SPI_CMD_READ_DEVICE_ID						0x9F
#define SPI_CMD_ENTER_4BYTE_AD_MODE				0xB7
#define SPI_CMD_EXIT_4BYTE_AD_MODE 				0xE9


#define SPI_CMD_READ_4BYTE 						0x13
#define SPI_CMD_SECTOR_ERASE_4BYTE				0xDC	/* address in border of 64KB; 4-byte sector erase */
#define SPI_CMD_PAGE_PROGRAM_4BYTE				0x12

#define SPI_CMD_RESET 								0xF5	/* exit QUAD operation */

#define	SPI_PAGE_SIZE								256


#define _SPI_DEVICE								"/dev/spidev0.0"


#define	SPI_CLOCK_FREQUENCY					 	40*1000*1000

typedef	struct 
{
	unsigned char		manuId;
	unsigned char		memType;
	unsigned char		memCap;
	unsigned char		dataLength;
}FLASH_ID_T;

/* specs, p.43. */
static FLASH_ID_T _n25q512 =
{
	.manuId = 0x20,
	.memType = 0xBA,
	.memCap = 0x20,
	.dataLength = 0x10,
};

/* send command to SPI slave and receive reply from it
* cmdData and replyData have same length
*/
static int _rawSpiSendCommand(int fd, unsigned char spiCmd, unsigned char *cmdData, unsigned char *replyData, unsigned int size, char *cmdLabel )
{
	struct spi_ioc_transfer xfer[1];/* include/linux/spi/spidev.h */
	unsigned char  cmd[5], data[5];
	int	ret;

	if(size> 4)
	{
		MUX_ERROR(_RAW_SPI_TITLE"Payload of SPI command can be longer than 4 bytes: %d", size);
		goto error;
	}

	if(size> 0 && (cmdData== NULL || replyData == NULL) )
	{
		MUX_ERROR(_RAW_SPI_TITLE"Size is %d, but data is null", size);
		goto error;
	}
	
	memset(xfer, 0, sizeof(xfer) );
	memset(cmd, 0, sizeof(cmd) );
	memset(data, 0, sizeof(data) );
	
	cmd[0] = spiCmd;
	if( (size > 0) && (cmdData != NULL ) )
	{
		memcpy(cmd+1, cmdData, (size>4)?4:size );
	}
	xfer[0].tx_buf = (unsigned long)cmd;
	xfer[0].len = size+1; 	/* length for both tx_buf and rx_buf */
	xfer[0].rx_buf = (unsigned long)data;
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		MUX_ERROR(_RAW_SPI_TITLE"Send Flash command %s(0x%02x) fail:%m", cmdLabel, spiCmd );
		return EXIT_FAILURE;
	}

	if( (size >0 ) && (replyData != NULL) )
	{
		memcpy(replyData, data+1, size );
	}
	EXT_DEBUGF(_DEBUG_RAW_SPI, _RAW_SPI_TITLE"Flash command %s(0x%02x) is OK", cmdLabel, spiCmd);
	
	return EXIT_SUCCESS;

error:
	if(size > 0 && replyData != NULL )
	{
		memset(replyData, INVALIDATE_VALUE_U8,  size);
	}
	
	return EXIT_FAILURE;
}


#define	SYS_RAW_SPI_CMD_RESET(fd)	\
		_rawSpiSendCommand(fd, SPI_CMD_RESET, NULL, NULL, 0, "RESET")


#define	SYS_RAW_SPI_CMD_WRITE_ENABLE(fd)	\
		_rawSpiSendCommand(fd, SPI_CMD_WRITE_ENABLE, NULL, NULL, 0, "WRITE_ENABLE")

#define	SYS_RAW_SPI_CMD_WRITE_DISABLE(fd)	\
		_rawSpiSendCommand(fd, SPI_CMD_WRITE_DISABLE, NULL, NULL, 0, "WRITE_DISABLE")


int sysRawSpiCmdReadId(int fd, int *flashId)
{
	int	cmdData=0;
	return _rawSpiSendCommand(fd, SPI_CMD_READ_DEVICE_ID, (unsigned char *)&cmdData, (unsigned char *)flashId, 4, "READ_ID");
}


int sysRawSpiCmdReadStatus(int fd, unsigned char *status)
{
	unsigned char cmdData;
	return _rawSpiSendCommand(fd, SPI_CMD_READ_STATUS, (unsigned char *)&cmdData, (unsigned char *)status, 1, "READ_STATUS");
}


int sysRawSpiCmd4byteAddressMode(int fd, int isEnable)
{
	unsigned char  cmd;
	
	int address = 0, dummy = 0;

	if( !isEnable )
	{/* exit 4-byte address mode */
		cmd = SPI_CMD_EXIT_4BYTE_AD_MODE;	
	}
	else
	{
		cmd = SPI_CMD_ENTER_4BYTE_AD_MODE;	
	}
	
	//write enable before enter/exit 4 byte address mode.
//	SYS_RAW_SPI_CMD_WRITE_ENABLE(fd);
	
	return _rawSpiSendCommand(fd, cmd, (unsigned char *)&address, (unsigned char *)&dummy, 4, (cmd == SPI_CMD_EXIT_4BYTE_AD_MODE)?"Exit4Byte": "Enter4Byte");
}

static int _checkSpiOperationStatus(int fd, int seconds)
{
	int timeout = seconds*1000;
	unsigned char status;
	int count = 0;
	
	//check status register bit 0--write in progress
	sysRawSpiCmdReadStatus(fd, &status);
	EXT_DEBUGF(_DEBUG_RAW_SPI, "Status: 0x%02X when read %d times", status, ++count);
	while( (status&0x1) )
	{
		timeout--;
		usleep(100);
		//cmn_delay(100);
		sysRawSpiCmdReadStatus(fd, &status);
		EXT_DEBUGF(_DEBUG_RAW_SPI, "Status: 0x%02X when read %d times", status, ++count);
		status = status & 0x1;
		if(timeout == 0)
		{
			MUX_ERROR(_RAW_SPI_TITLE"Timeout after %d seconds when check SPI operation status", seconds );
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

/* sector: 64 K */
int sysRawSpiSectorErase4byte(int fd, int startAddr)
{
	unsigned char  addr[4], data[4];
	int timeoutSeconds = 3;/* 3 seconds */
	
	addr[0] = (startAddr >> 24) & 0xff;
	addr[1] = (startAddr >> 16) & 0xff;
	addr[2] = (startAddr >> 8) & 0xff;
	addr[3] = startAddr & 0xff;

	if(_rawSpiSendCommand(fd, SPI_CMD_SECTOR_ERASE_4BYTE, addr, data, 4, "ERASE_SECTOR") == EXIT_FAILURE)
	{
		MUX_ERROR(_RAW_SPI_TITLE"Erase sector at 0x%x failed", startAddr);
		return EXIT_FAILURE;
	}
	
	return _checkSpiOperationStatus(fd, timeoutSeconds);
	
}


/* write: one page, 256 bytes;
* read: 512 bytes 
*/
int _sysRawSpiRwDataIn4byte(int fd, int startAddr, unsigned char *data, int size, int isRead)
{
	struct spi_ioc_transfer xfer[1];
	
	unsigned char  cmdBuf[SPI_PAGE_SIZE*2+6], dataBuf[SPI_PAGE_SIZE*2+6];
	
	int	len;
	int timeoutSeconds = 3;

	memset(xfer, 0, sizeof xfer);
	memset(cmdBuf, 0, sizeof(cmdBuf));
	memset(dataBuf, 0, sizeof(dataBuf));
	
	if(isRead)
	{
		if(size > SPI_PAGE_SIZE*2 )
		{
			EXT_ERRORF("Cannot read %d bytes (>256*2) in one page", size);
			return -1;
		}
		len = (size>SPI_PAGE_SIZE*2)?SPI_PAGE_SIZE*2:size;

//		sysRawSpiCmd4byteAddressMode(fd, EXT_TRUE);
		
		cmdBuf[0] = SPI_CMD_READ_4BYTE;
	}
	else
	{
		if(size > SPI_PAGE_SIZE)
		{
			EXT_ERRORF("Cannot program %d bytes (>256) in one page", size);
			return -1;
		}
		len = (size>SPI_PAGE_SIZE)?SPI_PAGE_SIZE:size;

#if 1		
		//write enable before page program.
		if(SYS_RAW_SPI_CMD_WRITE_ENABLE(fd) == EXIT_FAILURE)
		{
			MUX_ERROR(_RAW_SPI_TITLE"Set Write Enable failed when program page at 0x%x", startAddr);
			return EXIT_FAILURE;
		}
#endif

		cmdBuf[0] = SPI_CMD_PAGE_PROGRAM_4BYTE;	
		
		memcpy(cmdBuf+5, data, len);
	}

	EXT_DEBUGF(_DEBUG_RAW_SPI, "%s %d bytes at address of 0x%08X", (isRead==0)?"Write":"Read", size, startAddr);
	cmdBuf[1] = (startAddr >> 24) & 0xff;
	cmdBuf[2] = (startAddr >> 16) & 0xff;
	cmdBuf[3] = (startAddr >> 8) & 0xff;
	cmdBuf[4] = startAddr & 0xff;
	
	
	xfer[0].tx_buf = (unsigned long)cmdBuf;
	xfer[0].len = size + 5; 
	xfer[0].rx_buf = (unsigned long)dataBuf;
	if (ioctl(fd, SPI_IOC_MESSAGE(1), xfer) < 0) 
	{
		MUX_ERROR("Page program SPI_IOC_MESSAGE fail at address 0x%x: %m", startAddr);
		return EXIT_FAILURE;
	}

	if(isRead)
	{
		memcpy(data, dataBuf+5, size);
		return EXIT_SUCCESS;
	}

	return _checkSpiOperationStatus(fd, timeoutSeconds);
}


/* write one page: 256 bytes */
#define	SYS_RAW_SPI_WRITE_DATA(fd, startAddr, data, size)	\
	_sysRawSpiRwDataIn4byte((fd), (startAddr), (data), (size), EXT_FALSE)
	

#define	SYS_RAW_SPI_READ_DATA(fd, startAddr, data, size)	\
	_sysRawSpiRwDataIn4byte((fd), (startAddr), (data), (size), EXT_TRUE)



int sysRawSpiInit(int fd, uint8_t mode, uint8_t bits, uint32_t speed)
{
	int ret;
	
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		EXT_ERRORF(_RAW_SPI_TITLE"can't set spi mode: %m");
		return EXIT_FAILURE;
	}
	
	// bits per word	
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		EXT_ERRORF(_RAW_SPI_TITLE"can't set bits per word: %m");
		return EXIT_FAILURE;
	}
	
	//max speed hz	
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{	
		EXT_ERRORF(_RAW_SPI_TITLE"can't set max speed hz: %m");
		return EXIT_FAILURE;
	}

	printf(_RAW_SPI_TITLE"mode: %d; BitPerWord:%d; MaxSpeed:%d MHz"EXT_NEW_LINE, mode, bits, speed/1000/1000 );
	
	return EXIT_SUCCESS;
}



int	cmnSysRawSpiFlashInit(void)
{
	//spi parameter
	uint8_t mode = 0; 
	uint8_t bits = 8; 
	uint32_t speed =  SPI_CLOCK_FREQUENCY;
	
	int fd;
	int flashId;
	
	fd = open(_SPI_DEVICE, O_RDWR);
	if(fd < 0)
	{
		EXT_ERRORF(_RAW_SPI_TITLE"can't open spi device: %s.", _SPI_DEVICE);
		return -EXIT_FAILURE;
	}

	SYS_RAW_SPI_CMD_RESET(fd);

	if(sysRawSpiInit(fd, mode, bits, speed) == EXIT_FAILURE)
	{
		EXT_ERRORF(_RAW_SPI_TITLE"Spi device init error.");
		return -EXIT_FAILURE;
	}
	
	if(sysRawSpiCmdReadId(fd, &flashId) == EXIT_FAILURE)
	{
		EXT_ERRORF(_RAW_SPI_TITLE"Spi device read id error.");
	}
	else
	{
		int *p = (int *)&_n25q512;
		int _id = (int )*p;
		printf(_RAW_SPI_TITLE"Flash_id = 0x%X(0x%X): %s"EXT_NEW_LINE, flashId, _id, (_id==flashId)? "OK":"Failed");
	}

#if 1
	if(sysRawSpiCmd4byteAddressMode(fd, EXT_TRUE) == EXIT_FAILURE)
	{
		EXT_ERRORF(_RAW_SPI_TITLE"Enter 4-Byte Address mode failed");
	}
#endif
	
	return fd;
}

void	cmnSysRawSpiFlashClose(int fd)
{
	if(sysRawSpiCmd4byteAddressMode(fd, EXT_FALSE) == EXIT_FAILURE)
	{
		EXT_ERRORF(_RAW_SPI_TITLE"Exit 4-Byte Address mode failed");
	}
	
	close(fd);
}

//uint32_t crc32_table[256];
//int make_crc32_table()
static int _makeCrc32Table(uint32_t *crc32_table)
{
	uint32_t c;
	int i = 0;
	int bit = 0;	
	for(i = 0; i < 256; i++)
	{
		c  = (uint32_t)i;		
		for(bit = 0; bit < 8; bit++)
		{
			if(c&1)
			{
				c = (c >> 1)^(0xEDB88320);
			}
			else
			{
				c =  c >> 1;
			}			
		}
		crc32_table[i] = c;			
	}
	return 1;
}

static uint32_t _makeCrc(uint32_t crc, uint8_t *string, uint32_t size, uint32_t *crc32_table) 
{
	while(size--)
		crc = (crc >> 8)^(crc32_table[(crc ^ *string++)&0xff]);
	return crc;
}



//start_addr must be based on 64KB size
int cmnSysRawSpiFlashWriteFile(int flashFd, char *filename, uint32_t startAddr)
{
	int ret = EXIT_FAILURE;

	uint8_t readData[SPI_PAGE_SIZE];	
	size_t  readLength = 0;

	int i;
	uint32_t crc32_table[256];
	uint32_t file_crc = INVALIDATE_VALUE_U32, read_crc = INVALIDATE_VALUE_U32;
	FILE *fp = NULL;
	int fileSize;
	int sectorCount, pageCount;
	
	long startTime, endTime;

	startTime = cmnGetTimeMs();
	
	//check start_addr
	if((startAddr%SPI_FLASH_SECTOR_SIZE) != 0)
	{
		MUX_ERROR("start_addr is NOT integer times of 64KB.\n");
		return -1;
	}
	
	_makeCrc32Table(crc32_table);
	
	memset(readData, 0, sizeof(readData));
	fileSize = cmn_count_file(filename);
	if(fileSize <= 0 )
	{
		MUX_ERROR("Size of file '%s' is not correct: %d", filename, fileSize);
		return EXIT_FAILURE;
	}
	
	sectorCount = fileSize/SPI_FLASH_SECTOR_SIZE;
	if((fileSize%SPI_FLASH_SECTOR_SIZE) != 0)
	{		
		sectorCount++;
	}

	pageCount = fileSize/SPI_PAGE_SIZE;
	if((fileSize%SPI_PAGE_SIZE) != 0)
	{
		pageCount++;
	}

	//write enable before sector erase command.
	if(SYS_RAW_SPI_CMD_WRITE_ENABLE(flashFd) == EXIT_FAILURE)
	{
		MUX_ERROR(_RAW_SPI_TITLE"Set Write Enable failed when rrase sector at 0x%x", startAddr);
		return EXIT_FAILURE;
	}

	printf(_RAW_SPI_TITLE"FPGA binary ('%s') size: %d bytes; Need %d sectors, %d pages of flash;"EXT_NEW_LINE, filename, fileSize, sectorCount, pageCount);

	printf(EXT_NEW_LINE _RAW_SPI_TITLE"Erase %d sectors from address 0x%x..."EXT_NEW_LINE, sectorCount, startAddr);
	for(i = 0; i < sectorCount; i++)
	{
		if(sysRawSpiSectorErase4byte(flashFd, startAddr + i*SPI_FLASH_SECTOR_SIZE)==EXIT_FAILURE)
		{
			MUX_ERROR("Failed at erase #%d page", i);
			return EXIT_FAILURE;
		}
		else
		{
			{
				printf(".");fflush(stdout);
			}
		}
	}
		
	fp = fopen(filename, "rb");	
	if(fp == NULL)
	{
		MUX_ERROR("Cannot open binary file '%s': %m", filename );
		goto error;
	}
	
	//read SPI_PAGE_SIZE 256 bytes from file, write to flash, calculate file CRC, until file end.
	printf(EXT_NEW_LINE EXT_NEW_LINE _RAW_SPI_TITLE"Program %d pages into address 0x%08X of flash..."EXT_NEW_LINE, pageCount, startAddr);
	for(i = 0; i < (pageCount - 1); i++)
	{		
		readLength = fread(readData, 1, SPI_PAGE_SIZE, fp);
		if(readLength != SPI_PAGE_SIZE)
		{
			MUX_ERROR("Read page #% from binary failed: %m", i);
			goto error;
		}
		
		//write to flash
		if(SYS_RAW_SPI_WRITE_DATA(flashFd, startAddr + i*SPI_PAGE_SIZE, readData, SPI_PAGE_SIZE)==EXIT_FAILURE)
		{
			MUX_ERROR("Error happened on write #%d page", i);
			goto error;
		}
		else
		{
			if( i%32== 0) /* 4 '.' for every sector */
			{
				printf(".");fflush(stdout);
			}
		}
			
		//make file crc
		file_crc = _makeCrc(file_crc, readData, SPI_PAGE_SIZE, crc32_table);		
	}
	
	//last page
	readLength = fileSize - i*SPI_PAGE_SIZE;
	if(readLength > 0)
	{
		memset(readData, 0xff, sizeof(readData));
		readLength = fread(readData, 1, readLength, fp);
		if(readLength != (fileSize - i*SPI_PAGE_SIZE))
		{
			MUX_ERROR("Read %d bytes of last page from binary failed: %d", fileSize - i*SPI_PAGE_SIZE, readLength);
			goto error;
		}
		
		if(SYS_RAW_SPI_WRITE_DATA(flashFd, startAddr + i*SPI_PAGE_SIZE, readData, readLength)==EXIT_FAILURE)
		{
			MUX_ERROR("Error happened on write %d bytes of last page", readLength);
			goto error;
		}
		else
		{
			printf(".");fflush(stdout);
		}
		
		//make file crc
		file_crc = _makeCrc(file_crc, readData, readLength, crc32_table);
	}
	printf(EXT_NEW_LINE);
	
	file_crc = ~file_crc;

	
	printf(EXT_NEW_LINE EXT_NEW_LINE _RAW_SPI_TITLE"Read %d pages from address 0x%08X to validate..."EXT_NEW_LINE, pageCount, startAddr);
	//read 256 bytes from flash, calculate read data CRC, until read all the written data(file_size).
	//printf("Read flash to check CRC.\n");		
	for(i = 0; i < (pageCount - 1); i++)
	{			
		SYS_RAW_SPI_READ_DATA(flashFd, startAddr + i*SPI_PAGE_SIZE, readData, SPI_PAGE_SIZE);
		read_crc = _makeCrc(read_crc, readData, SPI_PAGE_SIZE, crc32_table);
		if( i%32== 0) /* 4 '.' for every sector */
		{
			printf(".");fflush(stdout);
		}
	}

	
	readLength = fileSize - i*SPI_PAGE_SIZE;
	if(readLength > 0)
	{
		memset(readData, 0xff, sizeof(readData) );
		SYS_RAW_SPI_READ_DATA(flashFd, startAddr + i*SPI_PAGE_SIZE, readData, readLength );
		read_crc = _makeCrc(read_crc, readData, readLength, crc32_table);			
		printf(".");fflush(stdout);
	}
	printf(EXT_NEW_LINE);
	read_crc = ~read_crc;

	EXT_DEBUGF(_DEBUG_RAW_SPI, "source CRC: 0x%08x; read CRC: 0x%08x", file_crc, read_crc);
	if(file_crc != read_crc)
	{
		MUX_ERROR("flash program ERROR!");
		goto error;
	}
	endTime = cmnGetTimeMs();

	printf(EXT_NEW_LINE EXT_NEW_LINE  _RAW_SPI_TITLE"Flash program OK, %ld ms spent!"EXT_NEW_LINE, (endTime-startTime));

	ret = EXIT_SUCCESS;

error:
	if(fp)
	{
		fclose(fp);
	}

	return ret;
	
}


//start_addr must be based on 64KB size
int cmnSysRawSpiFlashRead(int flashFd, uint32_t startAddr, uint32_t pageCount)
{
	int i;
	uint8_t readData[SPI_PAGE_SIZE];
	char msg[128];
	uint32_t address = startAddr;

	
	MUX_INFO("Read flash: page count: %d; start address: 0x%08x ", pageCount, startAddr);
	
	for(i = 0; i < pageCount; i++)
	{			
		SYS_RAW_SPI_READ_DATA(flashFd, address, readData, SPI_PAGE_SIZE);
		
		snprintf(msg, sizeof(msg), "Page#%d, Address: 0x%08x", i,  address);
		CMN_HEX_DUMP((const uint8_t *)readData, SPI_PAGE_SIZE, msg);

		address += SPI_PAGE_SIZE;
	}

	return i;
}


