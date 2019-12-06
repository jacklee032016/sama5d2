/*
* Retrieve status  of runtime from PTP daemon
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "muxPtp.h"
#include "libCmn.h"

#define CMN_MODULE_MUX_PMC			"ptpc"
int handle_term_signals(void);

static void usage(char* base)
{
	printf("%s: \n\tPTP Client: Command line interface retrieving PTP status.\n"\
		"\t%s -d DOMAIN_NUMBER \n", 
		  base, base);
}

int main(int argc, char *argv[])
{
	MuxPtpRuntime	_muxPtp;
	MuxPtpRuntime	*muxPtp = &_muxPtp;

	int opt;
	int domain = 0;

	memset(muxPtp, 0, sizeof(MuxPtpRuntime));
	while ((opt = getopt (argc, argv, "d:")) != -1)
	{
		switch (opt)
		{
			case 'd':/* domain number */
				domain = atoi(optarg);
				printf("Domain Number: \t%d", domain );
				break;

			default:
				printf( "opt '%c'", opt);
				usage(argv[0]);
				exit(1);
		}

	}

//	printf(CMN_MODULE_MUX_PMC "\n"  );
	usage(argv[0] );
	printf("PTP domain number:%d\n", domain);

//	handle_term_signals();
//	muxPtp->domainCfg = domain;
	if(muxPtpInit(muxPtp, (unsigned char)domain) == NULL )
	{
		MUX_ERROR("PTP Client initialization failed");
		goto out;
	}

	if(muxPtpRetrieve(muxPtp) == EXIT_FAILURE)
	{
		MUX_ERROR("Retrive status from PTP daemon failed");
		goto out;
	}

	muxPtpDebug(muxPtp);
	
out:
	muxPtpDestory(muxPtp);

	return 0;
}

