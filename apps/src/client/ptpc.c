/*
* Retrieve status  of runtime from PTP daemon
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "muxPtp.h"

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

	printf(CMN_MODULE_MUX_PMC "\n"  );
	printf("PTP domain number:%d\n", domain);

//	handle_term_signals();
	if(muxPtpInit(muxPtp, (unsigned char)domain ) == NULL )
	{
		printf("PTP Client initialization failed\n");
		goto out;
	}

	if(muxPtpRetrieve(muxPtp) == EXIT_FAILURE)
	{
		printf("Retrive status from PTP daemon failed\n");
		goto out;
	}

	
out:
	muxPtpDebug(muxPtp);
	muxPtpDestory(muxPtp);

	return 0;
}

