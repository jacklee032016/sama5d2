
#ifndef	__CMN_MUX_API_H__
#define	__CMN_MUX_API_H__

#ifdef __cplusplus
	extern "C"
	{
#endif

int cmnMuxClientInit(int port, CTRL_LINK_TYPE type, char *serverAddress, int timeoutSecond);


#define	CLIENT_INIT_IP_CMD(port, address)	\
	cmnMuxClientInit((port), CTRL_LINK_UDP, (address), 5)


#define	CLIENT_INIT_REST()	\
	cmnMuxClientInit(0, CTRL_LINK_UNIX, NULL, 3)


void cmnMuxClientDestroy(void);

/* PlayMedia command and its actions */


#define	_muxApiMediaGenericCmdNoParams(ipcmdName) \
		cmnMuxClientRequest((ipcmdName))
			
#define	_muxApiPlayMediaGenericCmdNoParams( action) \
		cmnMuxClientRequest((action))


/* APIs for WEB */
#define	muxApiWebInfos()	\
	_muxApiMediaGenericCmdNoParams(IPCMD_NAME_MEDIA_WEB)
	

/* APIs for Sys Admin */
#define	muxApiSysAdminThreads()	\
	_muxApiMediaGenericCmdNoParams(IPCMD_NAME_SYS_ADMIN)
	
#define	muxApiSysAdminVerInfo()	\
	_muxApiMediaGenericCmdNoParams(IPCMD_NAME_SYS_ADMIN)


#define	muxApiPlayMediaQuit()	\
	_muxApiPlayMediaGenericCmdNoParams("quit")


#ifdef __cplusplus
	};
#endif

#endif

