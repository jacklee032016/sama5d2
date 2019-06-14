
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include "libCmn.h"
#include "libCmnSys.h"

#include "mux7xx.h"

#define	_W1_SLAVE_ROOT_PATH			"/sys/devices/w1_bus_master1/"


static void _bspScCreateSecrect(SC_CTRL *sc)
{
	sc->secret[0] = (sc->romId[0] + 0x2A + 0x2) & 0xFF;
	sc->secret[1] = (sc->romId[1] + 0x4D + 0x2) & 0xFF;
	sc->secret[2] = (sc->romId[2] + 0x41 + 0x2) & 0xFF;
	sc->secret[3] = (sc->romId[3] + 0x52 + 0x2) & 0xFF;

	sc->secret[4] = (sc->romId[4] + 0x42 + 0x2) & 0xFF;
	sc->secret[5] = (sc->romId[5] + 0x4F + 0x2) & 0xFF;
	sc->secret[6] = (sc->romId[6] + 0x48 + 0x2) & 0xFF;
	sc->secret[7] = (sc->romId[7] + 0x4A + 0x2) & 0xFF;

	sc->secret[8]  = (sc->romId[0] + 0x4F + 0x4) & 0xFF;
	sc->secret[9]  = (sc->romId[1] + 0x43 + 0x4) & 0xFF;
	sc->secret[10] = (sc->romId[2] + 0x41 + 0x4) & 0xFF;
	sc->secret[11] = (sc->romId[3] + 0x55 + 0x4) & 0xFF;

	sc->secret[12] = (sc->romId[4] + 0x42 + 0x4) & 0xFF;
	sc->secret[13] = (sc->romId[5] + 0x50 + 0x4) & 0xFF;
	sc->secret[14] = (sc->romId[6] + 0x45 + 0x4) & 0xFF;
	sc->secret[15] = (sc->romId[7] + 0x54 + 0x4) & 0xFF;

	sc->secret[16] = (sc->romId[0] + 0x4A + 0x6) & 0xFF;	
	sc->secret[17] = (sc->romId[1] + 0x49 + 0x6) & 0xFF;
	sc->secret[18] = (sc->romId[2] + 0x41 + 0x6) & 0xFF;
	sc->secret[19] = (sc->romId[3] + 0x52 + 0x6) & 0xFF;

	sc->secret[20] = (sc->romId[4] + 0x4F + 0x6) & 0xFF;
	sc->secret[21] = (sc->romId[5] + 0x44 + 0x6) & 0xFF;
	sc->secret[22] = (sc->romId[6] + 0x46 + 0x6) & 0xFF;
	sc->secret[23] = (sc->romId[7] + 0x41 + 0x6) & 0xFF;

	sc->secret[24] = (sc->romId[0] + 0x4E + 0x8) & 0xFF;	
	sc->secret[25] = (sc->romId[1] + 0x53 + 0x8) & 0xFF;
	sc->secret[26] = (sc->romId[2] + 0x41 + 0x8) & 0xFF;
	sc->secret[27] = (sc->romId[3] + 0x4E + 0x8) & 0xFF;

	sc->secret[28] = (sc->romId[4] + 0x50 + 0x8) & 0xFF;
	sc->secret[29] = (sc->romId[5] + 0x41 + 0x8) & 0xFF;
	sc->secret[30] = (sc->romId[6] + 0x54 + 0x8) & 0xFF;
	sc->secret[31] = (sc->romId[7] + 0x2A + 0x8) & 0xFF;


}


static void _bspScInitPage(SC_CTRL *sc)
{
	/* page structure */
	memset(sc->pageData, 0, sizeof(sc->pageData) );
	
	memcpy(sc->pageData, sc->romId, 8);
	memcpy(sc->pageData + 8, EXT_MODEL, 6);
	memcpy(sc->pageData + 14, EXT_PRODUCT_NAME, 13);
	
	sc->pageData[27] = EXT_MAGIC_VALUE_A;
	sc->pageData[28] = EXT_MAGIC_VALUE_B;
	sc->pageData[29] = EXT_MAGIC_VALUE_A;
	sc->pageData[30] = EXT_MAGIC_VALUE_B;
	sc->pageData[31] = EXT_MAGIC_VALUE_A;
}


/* initialize 32 byte challenge from random */
int cmnSysScChallegeInit(SC_CTRL *sc)
{
	int ranVal;
	int i;
	
	srand((unsigned) time(NULL));
	
	for (i = 0; i < 8; i++)
	{
		ranVal= rand();
		memcpy( &sc->challenge[i*4], &ranVal, 4);
	}

	return EXIT_SUCCESS;
}

char cmnSysScComputeMAC(SC_CTRL *sc)
{
	int i,j;
	uint8_t calc_mac[32];
	uint8_t tmp[4]; 
	uint8_t MT[128];
	
	// create buffer to compute and verify mac
	memset(MT,0,128);

	// insert page data
	memcpy(&MT[0], sc->pageData, 32);

	// insert challenge
	memcpy(&MT[32], sc->challenge,32);
   
	// insert secret
	memcpy(&MT[64], sc->secret, 32);
	
	// insert ROM number or FF
	if (sc->isAnon )
	{
		memset(&MT[96], 0xFF, 8);
	}
	else
	{
		memcpy(&MT[96], sc->romId, 8);
	}

	MT[106] = sc->pageNum;
	MT[105] = sc->manId[0];
	MT[104] = sc->manId[1];

	// change to little endian for A1 devices    
	for (i = 0; i < 108; i+=4)
	{
		for (j = 0; j < 4; j++)
		{
			tmp[3 - j] = MT[i + j];
		}

		for (j = 0; j < 4; j++)
		{
			MT[i + j] = tmp[j];
		}
	}

	ComputeSHA256(MT, 119, EXT_TRUE, EXT_TRUE, calc_mac);
#if 1//BSP_SC_DEBUG
	CMN_HEX_DUMP(calc_mac, 32, "Calculated MAC:");
#endif

	// Compare calculated mac with one read from device
	for (i = 0; i < 32; i++)
	{
		if (sc->readMac[i] != calc_mac[i])
		{
			return EXIT_FAILURE;
		}
	}


	return EXIT_SUCCESS; 
}


#if 0
/* Write the 256 bit(32 byte) data to scratchpad with CRC16 verification for DS28E15 */
static char __bspWriteScratchPad(unsigned char *data)
{
	int i;
	unsigned short read_crc;
	unsigned short cmd = SC_CMD_READ_WRITE_SCRATCHPAD;

	CRC16 = 0;
	//CMD code
	docrc16(cmd);
	////PB--parameter byte bitmap, BIT3-0:0000 write, 1111 read
	cmd = 0x0;
	docrc16(cmd);
	
	_bspScReset();
	
	//SKIP_ROM
	_bspScWriteByte(SC_CMD_ROM_SKIP);	
	_bspScWriteByte(SC_CMD_READ_WRITE_SCRATCHPAD);		
	_bspScWriteByte(0x0);	
	
	//first crc	
	SC_READ_16BIT(read_crc);
	if( CRC16 != read_crc )
	{
#if BSP_SC_DEBUG
		EXT_ERRORF("SC: writeScratch CMD CRC error: %04x != %04x", CRC16,  read_crc );
#endif
		return EXIT_FAILURE;
	}
	EXT_DEBUGF(BSP_SC_DEBUG, "SC: WriteScratch CMD CRC: %04x:%04x", CRC16, read_crc);
		
	//data			
	for(i = 0; i < 32; i++ )
	{
		_bspScWriteByte(data[i]);	
	}
	
	SC_READ_16BIT(read_crc);
	
	CRC16 = 0;		
	for(i = 0; i < 32; i++ )
	{
		docrc16(data[i]);		
	}
	
	if( CRC16 != read_crc )	
	{
#if BSP_SC_DEBUG
		EXT_ERRORF("SC: writeScratch Data CRC error : %04x !=%04x !", CRC16, read_crc );
#endif
		return EXIT_FAILURE;
	}
	EXT_DEBUGF(BSP_SC_DEBUG, "SC: WriteScratch Data CRC: %04x:%04x", CRC16, read_crc);
	
	return EXIT_SUCCESS;
}


/*
* load and lock the 256 bit(32 byte) secret to chip
* 'secret'- secret to load
* 'lock_flag' - 1: lock , 0: no lock
* 'magic - 0x5A, protect lock_flag
*/
static char _writeKeyIntoScratchAndLock(unsigned char *secret, unsigned char lock_flag, unsigned char lock_magic)
{
	unsigned short read_crc;	
	unsigned short cmd = SC_CMD_LOAD_LOCK_SECRET;	
	unsigned char PB;
	
	if(__bspWriteScratchPad(secret) == EXIT_FAILURE)
	{
		EXT_ERRORF("SC: LoadSecret error!");
		return EXIT_FAILURE;
	}	
		
	if((lock_flag == 1)&&(lock_magic == 0x5A))
	{
		PB = 0xE0;
	}
	else
	{
		PB = 0x00;
	}

	CRC16 = 0;
	//CMD code
	docrc16(cmd);
	////PB--parameter byte bitmap, BIT3-0:0000 write, 1111 read
	cmd = PB;
	docrc16(cmd);
	
	_bspScReset();
	//SKIP_ROM
	_bspScWriteByte(SC_CMD_ROM_SKIP);	
	_bspScWriteByte(SC_CMD_LOAD_LOCK_SECRET);		
	_bspScWriteByte(PB);
	
	//first crc	
	SC_READ_16BIT(read_crc);
	if( CRC16 != read_crc)
	{
		EXT_ERRORF("SC: LoadSecret CMD CRC error %04x != %04x!", CRC16, read_crc );
		return EXIT_FAILURE;
	}		
	EXT_DEBUGF(BSP_SC_DEBUG, "SC: LoadSecret CMD CRC: %04x:%04x", CRC16, read_crc);
	
	//send release
	_bspScWriteByte(0xAA);
	//strong pull up
//	enable_ds_pull_up();
	SC_STRONG_PULL_UP_ENABLE();
	//delay tprs
	delay_ms(SECRET_PROGRAM_DELAY);
	SC_STRONG_PULL_UP_DISABLE();
	
//	disable_ds_pull_up();
	
	read_crc = _bspScReadByte();	
	if(read_crc == 0xAA)
	{
		return EXIT_SUCCESS;
	}
	else if(read_crc == 0x55)
	{
		EXT_ERRORF("SC: LoadSecret: secret was locked before.");
		return EXIT_FAILURE;
	}
	else
	{
		EXT_ERRORF("SC: LoadSecret fail.");
		return EXIT_FAILURE;
	}			
}




/*  Do Compute Page MAC command and return MAC. Optionally do anonymous mode (anon != 0). 
* page_num - page number to read 0 - 16
* challange - 32 byte buffer containing the challenge
* mac - 32 byte buffer for page data read
* anon - Flag to indicate Anonymous mode if (anon != 0)
*/
static char _writeChallengeIntoScratchAndReadMAC(SC_CTRL *sc)
{
	int i;
	unsigned short read_crc;
	unsigned short cmd = SC_CMD_COMPUTE_READ_PAGE_MAC;
	unsigned char PB;	
	
	if(sc->isAnon == 1)	//anonymous 
	{
		PB = 0xE0;
	}
	else
	{
		PB = 0x00;	//use ROM ID
	}
	
	PB |= (sc->pageNum & 0x1);
	
	// write the challenge 32 bytes to the scratchpad
	if(__bspWriteScratchPad(sc->challenge) == EXIT_FAILURE)
	{
#if BSP_SC_DEBUG
		EXT_ERRORF("SC: WriteScratchPad fail.");
#endif
		return EXIT_FAILURE;
	}
	
	CRC16 = 0;
	//CMD code
	docrc16(cmd);
	////PB--parameter byte bitmap, BIT3-0:0000 write, 1111 read
	cmd = PB;
	docrc16(cmd);
	
	_bspScReset();
	//SKIP_ROM
	_bspScWriteByte(SC_CMD_ROM_SKIP);	
	_bspScWriteByte(SC_CMD_COMPUTE_READ_PAGE_MAC);		
	_bspScWriteByte(PB);	
	
	//first crc	
	SC_READ_16BIT(read_crc);
		
	if( CRC16 !=  read_crc )
	{
		EXT_ERRORF("SC: ReadMAC CMD CRC error %04x != %04x!", CRC16, read_crc );
		return EXIT_FAILURE;
	}		
	EXT_DEBUGF(BSP_SC_DEBUG, "SC: ReadMAC CMD CRC: %04x:%04x", CRC16, read_crc);

	//wait for 2*tcsha
	delay_ms(2*SHA_COMPUTATION_DELAY);
	
	//read cs
	read_crc = _bspScReadByte();
	if(read_crc != 0xAA)
	{
		EXT_ERRORF("SC: ReadDMA read cs error: 0x%x", read_crc);
		return EXIT_FAILURE;
	}
	
	//read 32 byte MAC 
	for(i = 0; i < 32; i++ )
	{
		sc->readMac[i] = _bspScReadByte();
	}
	
	SC_READ_16BIT(read_crc);
	
#if BSP_SC_DEBUG
	CONSOLE_DEBUG_MEM(sc->readMac, 32, 0, "Read MAC");
#endif

	CRC16 = 0;		
	for(i = 0; i < 32; i++ )
	{
		PB = sc->readMac[i];
		docrc16(PB);
	}

	if(CRC16 != read_crc)
	{
#if BSP_SC_DEBUG
		EXT_ERRORF("SC: ReadMAC Data CRC error: %04x != %04x!", CRC16, read_crc );
		CONSOLE_DEBUG_MEM(sc->readMac, 32, 0, "Read error MAC");
#endif
		return EXIT_FAILURE;
	}

#if BSP_SC_DEBUG
	CONSOLE_DEBUG_MEM(sc->readMac, 32, 0, "Read MAC");
	EXT_DEBUGF(BSP_SC_DEBUG, "SC: ReadMAC Data CRC: %04x:%04x", CRC16, read_crc);
#endif
	
	return EXIT_SUCCESS;	
}
#endif

/***  public interfaces of security chip ****/

#define	ROM_ID_PRINT(buf, romId)	\
	sprintf((buf), "%02x-%02x%02x%02x%02x%02x%02x", *romId, *(romId+6), *(romId+5), *(romId+4), *(romId+3), *(romId+2), *(romId+1) )

static SecurityFiles  _scFiles;
/*
* read ROM ID, initialize pageData and create secrect locally;
* read personality from chip
*/
SecurityFiles *cmnSysScInit(void)
{
	SecurityFiles *scf = &_scFiles;
	/* read ROM ID */
	unsigned char romId[SC_ROM_ID_SIZE];
	char romStr[256];

	if(cmnSysW1GetRomId(romId) )
	{
		EXT_ERRORF("No ROM ID of security chip is found" );
		return NULL;
	}
	
	memset(scf, 0, sizeof(SecurityFiles));
	
	CMN_HEX_DUMP(romId, SC_ROM_ID_SIZE, "ROM ID");
	ROM_ID_PRINT(romStr, romId);

	snprintf(scf->pageFile, sizeof(scf->pageFile), _W1_SLAVE_ROOT_PATH"%s/page0", romStr);
	
	snprintf(scf->keyFile, sizeof(scf->keyFile), _W1_SLAVE_ROOT_PATH"%s/key", romStr);
	
	snprintf(scf->macFile, sizeof(scf->macFile), _W1_SLAVE_ROOT_PATH"%s/MAC", romStr);
	
	snprintf(scf->crcFile, sizeof(scf->crcFile), _W1_SLAVE_ROOT_PATH"%s/crc", romStr);

	memcpy(scf->sc.romId, romId, sizeof(scf->sc.romId));
	_bspScInitPage(&scf->sc);
	_bspScCreateSecrect(&scf->sc);

	/* read personality */
	return scf; 
}

int	cmnSysScRWPage(SecurityFiles *scf, unsigned char *page, int isRead)	/* key : 32 bytes*/
{
	int len = 0;

	int fd = open(scf->pageFile, O_RDWR);
	if( fd < 0)
	{
		EXT_ERRORF("Open %s failed:%m", scf->pageFile);
		return EXIT_FAILURE;
	}
	
	if(isRead)
	{
	TRACE();
		len = read(fd, page, SC_PAGE_SIZE);
	}
	else
	{
	TRACE();
		len = write(fd, page, SC_PAGE_SIZE);
	}

	close(fd);
	if(len != SC_PAGE_SIZE)
	{
		EXT_ERRORF("SC: %s Page failed, %d:%m", (isRead)?"Read":"Write", len);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


int	cmnSysScRWKey(SecurityFiles *scf, unsigned char *page, int isRead)	/* key : 32 bytes*/
{
	int len = 0;
	
	int fd = open(scf->keyFile, O_RDWR);
	if( fd < 0)
	{
		EXT_ERRORF("Open %s failed:%m", scf->keyFile);
		return EXIT_FAILURE;
	}

	if(isRead)
	{
		len = read(fd, page, SC_PAGE_SIZE);
	}
	else
	{
		len = write(fd, page, SC_PAGE_SIZE);
	}
	close(fd);
	
	if(len != SC_PAGE_SIZE)
	{
		EXT_ERRORF("SC: %s Key failed, %d:%m", (isRead)?"Read":"Write", len);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


int	cmnSysScRWMac(SecurityFiles *scf, unsigned char *page, int isRead)	/* key : 32 bytes*/
{
	int len = 0;
	int fd = open(scf->macFile, O_RDWR);
	if( fd < 0)
	{
		EXT_ERRORF("Open %s failed:%m", scf->macFile);
		return EXIT_FAILURE;
	}

	if(isRead)
	{
		len = read(fd, page, SC_PAGE_SIZE);
	}
	else
	{
		len = write(fd, page, SC_PAGE_SIZE);
	}
	close(fd);
	
	if(len != SC_PAGE_SIZE)
	{
		EXT_ERRORF("SC: %s MAC failed, %d: %m", (isRead)?"Read":"Write", len);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


int	cmnSysScCrc(SecurityFiles *scf, int isEnable)
{
	int len = 0;
	unsigned char _val = '0';
	int fd;

	if(isEnable)
		_val = '1';

	fd = open(scf->crcFile, O_RDWR);
	if( fd < 0)
	{
		EXT_ERRORF("Open %s failed:%m", scf->crcFile);
		return EXIT_FAILURE;
	}
	
	len = write(fd, &_val, sizeof(_val));
//	len = write(fd, '1', 1);
	close(fd);
	
	if(len < 0 )
	{
		EXT_ERRORF("SC: %s CRC failed, %d:%m", (isEnable)?"Enable":"Disable", len);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


#if 0
/*
* Write key into chip, key from IP command or test program
*/
int	cmnSysScWriteKey(SC_CTRL *sc, unsigned char *key)	/* key : 32 bytes*/
{

	_bspScWritePage(sc->pageNum, sc->pageData);

	//load and lock secret		
	_writeKeyIntoScratchAndLock(key, lock_flag, lock_magic);
	
	return EXIT_SUCCESS;
}


/*
* Write challenge into chip, and read back MAC; then compare with MAC caculated locally
*/
int	cmnSysScCheck(SC_CTRL *sc)
{
	_writeChallengeIntoScratchAndReadMAC(sc);

	_bspScComputeMAC(sc);
	return EXIT_SUCCESS;
}
#endif


