/*
* Write SC key when IP command client is not available
*/

#include "libCmnSys.h"

#define CMN_MODULE_SC_TEST			"scKey"

int main(int argc, char *argv[])
{

	printf(CMN_VERSION_INFO(CMN_MODULE_SC_TEST) EXT_NEW_LINE );

	SecurityFiles *scf = cmnSysScInit();
	if(!scf)
		return EXIT_FAILURE;


	EXT_INFOF(EXT_NEW_LINE"KEY"EXT_NEW_LINE"\tWrite ...");
	CMN_HEX_DUMP(scf->sc.secret, sizeof(scf->sc.secret), "Write key:");
	if(SC_KEY_WRITE(scf, scf->sc.secret))
		return EXIT_FAILURE;

	EXT_INFOF(EXT_NEW_LINE"OK");
	return 0;
}

