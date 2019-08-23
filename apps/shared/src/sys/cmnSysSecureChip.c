
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include "libCmnSys.h"

#include "mux7xx.h"

#ifndef uchar
   typedef unsigned char uchar;
#endif
int ComputeSHA256(uchar* message, short length, ushort skipconst, ushort reverse, uchar* digest);

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
int cmnSysScChallengeInit(SC_CTRL *sc)
{
	int ranVal;
	int i;
	
	srand((unsigned) time(NULL));
	
	for (i = 0; i < 8; i++)
	{
#if 1
		if(i==0)
			ranVal= 1234;
		else if(i==1)
			ranVal= 5678;
		else if(i==2)
			ranVal= 9012;
		else if(i==3)
			ranVal= 1290;
		else
			ranVal= 3456;
#else
		ranVal= rand();
#endif
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
	if (0)//sc->isAnon )
	{
		memset(&MT[96], 0xFF, 8);
	}
	else
	{
		memcpy(&MT[96], sc->romId, 8);
	}

	MT[106] = 0;//sc->pageNum;
	MT[105] = sc->manId[0];
	MT[104] = sc->manId[1];

	CMN_HEX_DUMP(MT, 119, "raw MT data:");
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


/***  public interfaces of security chip ****/

#define	ROM_ID_PRINT(buf, romId)	\
	sprintf((buf), "%02x-%02x%02x%02x%02x%02x%02x", *romId, *(romId+6), *(romId+5), *(romId+4), *(romId+3), *(romId+2), *(romId+1) )

static SecurityFiles  _scFiles;
/*
* read ROM ID, initialize pageData and create secrect locally;
* read personality from chip
* Used to find device. If device is found, validation failed, then it is used to input key 
*/
SecurityFiles *cmnSysScInit(void)
{
	SecurityFiles *scf = &_scFiles;
	/* read ROM ID */
	unsigned char romId[SC_ROM_ID_SIZE];
	char romStr[256];
	unsigned char	personality_byte[SC_PAGE_SIZE];

#if ARCH_ARM
	if(cmnSysW1GetRomId(romId) )
	{
		EXT_ERRORF("No ROM ID of security chip is found" );
		return NULL;
	}
#else
	/*17.000000882884 */
	memset(romId, 0, sizeof(romId));
	romId[0]= 0x17;
	
	romId[5]= 0x88;
	romId[6]= 0x28;
	romId[7]= 0x84;
#endif

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
	EXT_INFOF(EXT_NEW_LINE"\tRead Personality...");
#if ARCH_ARM
	SC_KEY_READ(scf, personality_byte);
#else
	int	ranVal= rand();
	memcpy( personality_byte, &ranVal, 4);
#endif
	CMN_HEX_DUMP(personality_byte, sizeof(personality_byte), "Personality");

#if 1
	scf->sc.manId[0] = personality_byte[2];
	scf->sc.manId[1] = personality_byte[3];
#else	
	scf->sc.manId[0] = personality_byte[1];
	scf->sc.manId[1] = personality_byte[0];
#endif

	MUX_INFO("ManuID: 0x%02x:0x%02x", scf->sc.manId[0], scf->sc.manId[1]);
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
		EXT_ERRORF("SC: %s MAC %s failed, %d: %m", (isRead)?"Read":"Write", scf->macFile, len);
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

/* after page and secret have been writen by IP command client */
int cmnSysScValidate(SecurityFiles *scf)
{
	if(!scf)
	{
		MUX_ERROR("MAC verification failed!\n");
		return EXIT_FAILURE;
	}

	cmnSysScChallengeInit(&scf->sc);
	EXT_INFOF(EXT_NEW_LINE"Challenge"EXT_NEW_LINE"\tWrite ...");
	CMN_HEX_DUMP(scf->sc.challenge, sizeof(scf->sc.challenge), "Write Challenge:");
	if(SC_MAC_WRITE(scf, scf->sc.challenge))
		return EXIT_FAILURE;


	EXT_INFOF(EXT_NEW_LINE"\tRead ...");
	if(SC_MAC_READ(scf, scf->sc.readMac))
		return EXIT_FAILURE;
	CMN_HEX_DUMP(scf->sc.readMac, sizeof(scf->sc.readMac), "Read Challenge:");
//	memcpy(scf->sc.readMac, challenge, sizeof(challenge));

#if ARCH_ARM
	if(cmnSysScComputeMAC(&scf->sc) == EXIT_FAILURE)
	{
		MUX_ERROR("Securety Validating result: Failed");
		return EXIT_FAILURE;
	}
#endif

	MUX_INFO("Securety Validating result: Success");
	
	return EXIT_SUCCESS;
}


/* write key and page with requirement from IP command client 
* key is secret from client, size must be SC_PAGE_SIZE
*/
int cmnSysScWriteKey(SecurityFiles *scf, unsigned char  *secret, int size)
{
	unsigned char page[SC_PAGE_SIZE];
	
	if(!scf)
		return EXIT_FAILURE;
	
	CMN_HEX_DUMP(secret, size, "Write SECRET");
	
	if(size != SC_PAGE_SIZE)
	{
		EXT_ERRORF("\tSize of secret key must be %d: %d", SC_PAGE_SIZE, size );
		return EXIT_FAILURE;
	}

	EXT_INFOF(EXT_NEW_LINE"PAGE"EXT_NEW_LINE"\tWrite ...");
	CMN_HEX_DUMP(scf->sc.pageData, sizeof(scf->sc.pageData), "Write page:");
	if(SC_PAGE_WRITE(scf, scf->sc.pageData))
		return EXIT_FAILURE;

	EXT_INFOF(EXT_NEW_LINE"\tRead ...");
	SC_PAGE_READ(scf, page);
	CMN_HEX_DUMP(page, sizeof(page), "Read Page");

	if(memcmp(scf->sc.pageData, page, sizeof(scf->sc.pageData)))
	{
		EXT_ERRORF(EXT_NEW_LINE"\tPage is wrong"EXT_NEW_LINE);
	}
	else
	{
		EXT_INFOF(EXT_NEW_LINE"\tPage is OK"EXT_NEW_LINE);
	}


	EXT_INFOF(EXT_NEW_LINE"KEY"EXT_NEW_LINE"\tWrite ...");
	CMN_HEX_DUMP(secret, SC_PAGE_SIZE, "Write Secret:");
	if(SC_KEY_WRITE(scf, secret))
		return EXIT_FAILURE;
	
	return EXIT_SUCCESS;
}

