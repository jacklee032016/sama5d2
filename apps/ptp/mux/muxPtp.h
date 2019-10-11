/* 
* Interface to access PTP from external
*/
#ifndef	__MUX_PTP_H__
#define	__MUX_PTP_H__



typedef	struct _MUX_PTP_ID
{
	unsigned char	id[8];
}MUX_PTP_ID;


typedef	struct _MuxPtpRuntime
{
	MUX_PTP_ID		clockId;
	MUX_PTP_ID		portId;

	unsigned short	domain;

	int 				state;
	char				stateName[256];

	void				*pmc;
}MuxPtpRuntime;



#endif

