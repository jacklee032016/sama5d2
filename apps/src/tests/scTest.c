/*
* test for Security Chip: write page and secret into SC chip
*/

#include "libCmnSys.h"

#include "muxDrv.h"


/* Security Chip */
int testSC(void )
{
	unsigned char page[SC_PAGE_SIZE];
	unsigned char key[SC_PAGE_SIZE];
	unsigned char challenge[SC_PAGE_SIZE];
	
	SecurityFiles *scf = cmnSysScInit();
	if(!scf)
		return EXIT_FAILURE;

//	if(SC_CRC_DISABLE(scf) )
//		return EXIT_FAILURE;

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
	CMN_HEX_DUMP(scf->sc.secret, sizeof(scf->sc.secret), "Write key:");
	if(SC_KEY_WRITE(scf, scf->sc.secret))
		return EXIT_FAILURE;

	EXT_INFOF(EXT_NEW_LINE"\tRead Personality...");
	SC_KEY_READ(scf, key);
	CMN_HEX_DUMP(key, sizeof(key), "Personality");

#if 0
	/* test read and write of scratchpad in driver only */
	if(memcmp(scf->sc.secret, key, sizeof(scf->sc.secret)))
	{
		EXT_ERRORF(EXT_NEW_LINE"\tKEY is wrong"EXT_NEW_LINE);
	}
	else
	{
		EXT_INFOF(EXT_NEW_LINE"\tKEY is OK"EXT_NEW_LINE);
	}
#endif

	scf->sc.manId[0] = key[2];
	scf->sc.manId[1] = key[3];

	cmnSysScChallegeInit(&scf->sc);

	EXT_INFOF(EXT_NEW_LINE"Challenge"EXT_NEW_LINE"\tWrite ...");
	CMN_HEX_DUMP(scf->sc.challenge, sizeof(scf->sc.challenge), "Write Challenge:");
	if(SC_MAC_WRITE(scf, scf->sc.challenge))
		return EXIT_FAILURE;


	EXT_INFOF(EXT_NEW_LINE"\tRead ...");
	if(SC_MAC_READ(scf, challenge))
		return EXIT_FAILURE;
	CMN_HEX_DUMP(challenge, sizeof(challenge), "Read Challenge:");
	memcpy(scf->sc.readMac, challenge, sizeof(challenge));

	if(cmnSysScComputeMAC(&scf->sc) == EXIT_FAILURE)
	{
		EXT_ERRORF(EXT_NEW_LINE"\tMAC Validating result: Failed"EXT_NEW_LINE);
	}
	else
	{
		EXT_INFOF(EXT_NEW_LINE"\tMAC Validating result: Success"EXT_NEW_LINE);
	}

	EXT_INFOF(EXT_NEW_LINE"\tEnable CRC ...");
	SC_CRC_ENABLE(scf);

	EXT_INFOF(EXT_NEW_LINE"\tDisable CRC ...");
	SC_CRC_DISABLE(scf);
	
	EXT_INFOF(EXT_NEW_LINE"Test OK!!!");
	return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
	int opt;
	char		cmd[CMN_NAME_LENGTH];

	return 0;

}

