/* 
* Interface to access PTP from external
*/
#ifndef	__MUX_PTP_H__
#define	__MUX_PTP_H__

#define		PTPC_NOISE_DEBUG			EXT_DBG_ON


#define		PTPC_ERROR_NO_CONNECTION		500
#define		PTPC_ERROR_NO_REPLY				501

typedef	struct _MUX_PTP_ID
{
	unsigned char	id[8];
}MUX_PTP_ID;


typedef	struct _MuxPtpRuntime
{
	/* configuration */
	unsigned char		isEnable;
	unsigned char		domainCfg;
	
	MUX_PTP_ID		clockId;
	MUX_PTP_ID		portId;

	int 				portState;
	char				portStateName[256];


	/************ default dataset ******************/
	unsigned char		isTwoStep;
	unsigned char		isSlaveOnly;

	/* order of BMC decision */
	unsigned char		priority1;
	unsigned char		clockClass;
	unsigned char		clockAccuracy;
	unsigned short	offsetScaledLogVariance;

	unsigned char		priority2;
	unsigned short	domain;

	unsigned char		portCount;
	

	MUX_PTP_ID		masterId;
	unsigned char		masterPresent;
	MUX_PTP_ID		sourceId;	/* source port */
	unsigned short	sourceIndex;

	/* dataset of parent */
	unsigned char		gmPriority1;
	unsigned char		gmClockClass;
	unsigned char		gmClockAccuracy;
	unsigned short	gmOffsetScaledLogVariance;

	unsigned char		gmPriority2;


	int				errCode;
	char				msg[256];

	void				*pmc;
}MuxPtpRuntime;

char *muxPtpId2Str(MUX_PTP_ID *id);

void *muxPtpInit(void *_muxPtp, unsigned char domainNumber);
void muxPtpDestory(void *_muxPtp);

int muxPtpRetrieve(void *_muxPtp);
int muxPtpDebug(void *_muxPtp);

int muxPtpDefaultConfig(void *_muxPtp);


#endif

