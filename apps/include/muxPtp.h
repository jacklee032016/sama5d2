/* 
* Interface to access PTP from external
*/
#ifndef	__MUX_PTP_H__
#define	__MUX_PTP_H__

#include <stdint.h>

#define		PTPC_NOISE_DEBUG			EXT_DBG_OFF

#define		PTPC_ERROR_OK						200		/* everything is OK */

#define		PTPC_ERROR_UN_INITIALIZED			300		/* data uninitalized */

#define		PTPC_ERROR_NO_CONNECTION		400
#define		PTPC_ERROR_NO_REPLY				401

/* system error */
#define		PTPC_ERROR_SYS_NO_CFG			500
#define		PTPC_ERROR_SYS_NET_CFG			501
#define		PTPC_ERROR_SYS_CREATE_CLIENT		502


#define	MUX_PTP_DEFAULT_PRIORITY_1					254
#define	MUX_PTP_DEFAULT_PRIORITY_2					254

#define	MUX_PTP_PRIORITY_MAX							MUX_PTP_DEFAULT_PRIORITY_1

#define	MUX_PTP_DEFAULT_CLOCK_CLASS					248 		/* p.55, table 5, default */
#define	MUX_PTP_DEFAULT_CLOCK_ACCURACY				0xFE	/* p.56, table 6, unknown */
#define	MUX_PTP_DEFAULT_OFFSET_VARIANCE				0xFFFF

typedef	struct _MUX_PTP_ID
{
	unsigned char	id[8];
}MUX_PTP_ID;


typedef	struct _MuxPtpConfig
{
	/* configuration */
	unsigned char		isEnable;
	unsigned char		domain;
	
	/************ default dataset ******************/
	unsigned char		isTwoStep;
	unsigned char		isSlaveOnly;

	/* order of BMC decision */
	unsigned char		priority1;
	unsigned char		clockClass;
	unsigned char		clockAccuracy;
	unsigned short	offsetScaledLogVariance;

	unsigned char		priority2;

}__attribute__((packed)) MuxPtpConfig;


typedef	struct _MuxPtpRuntime
{
	/* data read dynamically */

	unsigned char		domain;


	MUX_PTP_ID		clockId;
	MUX_PTP_ID		portId;

	int 				portState;
	char				portStateName[256];

	unsigned char		portCount;
	

	MUX_PTP_ID		masterId;
	unsigned char		masterPresent;
	MUX_PTP_ID		sourceId;	/* source port */
	unsigned short	sourceIndex;
	uint64_t			offset;		/* offset from master, nanosecond */

	/* dataset of parent */
	unsigned char		gmPriority1;
	unsigned char		gmClockClass;
	unsigned char		gmClockAccuracy;
	unsigned short	gmOffsetScaledLogVariance;

	unsigned char		gmPriority2;


	int				errCode;
	char				msg[256];

	void				*pmc;
}__attribute__((packed)) MuxPtpRuntime;


char *muxPtpId2Str(MUX_PTP_ID *id);

void *muxPtpInit(void *_muxPtp, unsigned char domain);
void muxPtpDestory(void *_muxPtp);

int muxPtpRetrieve(void *_muxPtp, unsigned char domain);
int muxPtpPoll(void *_muxPtp, unsigned char domain);

int muxPtpDebug(void *_muxPtp);

int muxPtpDefaultConfig(void *_muxPtp);

int muxPtpInitData(void *_muxPtp);

#endif

