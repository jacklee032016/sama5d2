/*
* common constants for PTP library and all programs
*/

#ifndef	__EXT_PTP_H__
#define	__EXT_PTP_H__

#if 0//ARCH_X86
#define	PTP_RUN_HOME					"/tmp"
#else
#define	PTP_RUN_HOME					"/var/run"
#endif


#define	PHC_SYS_UNIX_SOCKET			PTP_RUN_HOME"/phc2sys"



#define	PHC_SYS_DEBUG					EXT_DBG_ON

#define	CONFIG_HOME					"/etc/mLab"

#if	ARCH_ARM
#define	CONFIG_FILE_HOME_DIR			CONFIG_HOME
#else
//#error	"root dir is " ROOT_DIR
#define	CONFIG_FILE_HOME_DIR			ROOT_DIR"/releases"CONFIG_HOME
#endif

#define	EXT_PTP_DEVICE_NAME			"eth0"

#define	EXT_CONFIG_FILE_PTPD			CONFIG_FILE_HOME_DIR"/muxPtpd.cfg"

#define	EXT_CONFIG_FILE_PHCD			CONFIG_FILE_HOME_DIR"/muxPhcd.cfg"	/* Ptp Hardware Clock controller */

#endif

