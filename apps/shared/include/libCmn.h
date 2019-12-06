/*
 * $Id$
*/

#ifndef __LIB_COMMON_H__
#define __LIB_COMMON_H__

#ifdef __cplusplus
	extern "C"
	{
#endif

#ifdef ARM
//#warning		"build for ARM platform"
#elif  X86
//#warning		"build for X86 platform"
#else
#error	"unknowm platform, build stopped"
#endif


/* build options for all modules */
#define	CMN_SHARED_DEBUG						1

#define	CMN_TIMER_DEBUG						0

/* timer thread start up after daemonized */
#define	CMN_TIMER_START_DELAY				1

#define	CMN_THREAD_DEBUG						0

#define	DEBUG_CONFIG_FILE						0


#define	MUX_OPTIONS_DEBUG_IP_COMMAND		1

#define	MUX_OPTIONS_DEBUG_IPC_CLIENT		0



/* playlist which is added is only saved in testing. In release version, no any playlist is saved in configuration file */
#define	MUX_SAVE_PLAYLIST						0

/* Macros for constants */

#define	CMN_NAME_LENGTH						256


#define	MUX_INVALIDATE_CHAR					0xFF
#define	MUX_INVALIDATE_SHORT					0xFFFF
#define	MUX_INVALIDATE_INTEGER				0xFFFFFFFF


/* board type, so libMux and libCmnSys can be customized */
#define	EXT_BOARD_TYPE			MUX_BOARD_TYPE_774

#if 1
#define	MUX_ATMEL_XPLAINED		1
#define	MUX_BOARD_768				2
#define	MUX_BOARD_774				3
#define	MUX_BOARD_767				4

//#define	MUX_BOARD				MUX_BOARD_774
//#define	MUX_BOARD				MUX_ATMEL_XPLAINED
#endif



// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define	ENV_64
#else
#define	ENV_32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define	ENV_64
#define	POINTRT_TO_INTEGER	long int
#else
#define	ENV_32
#define	POINTRT_TO_INTEGER	int
#endif
#endif

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pthread.h"


#define	DEBUG_RAW_RW						1

#define CMN_OK 								0
#define CMN_FAIL 							-1

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#ifndef	false
#define	false								0
#endif

#ifndef	true
#define	true									(!false)
#endif

#ifndef	OK
#define	OK									0
#endif

#ifndef	FAIL
#define	FAIL								-1
#endif


#define EXIT_FAILURE 			1
#define EXIT_SUCCESS 			0


#define KB(x)						((x) / 1024)
#define PERCENTAGE(x,total)		(((x) * 100) / (total))

#define	UNIT_OF_KILO					1024
#define	UNIT_OF_MEGA					(UNIT_OF_KILO*UNIT_OF_KILO)
#define	UNIT_OF_GIGA					(UNIT_OF_KILO*UNIT_OF_MEGA)

#define	UNIT_K_HZ						1000
#define	UNIT_M_HZ						((UNIT_K_HZ)*(UNIT_K_HZ))


#define	CMNLIB_OPS_DEBUG						0x01
#define	CMNLIB_OPS_OTHER						0x02



/* define board, libMedia uses it, so defined here */
typedef	enum _MUX_BOARD_TYPE
{
	MUX_BOARD_TYPE_RX769 = 0,
	MUX_BOARD_TYPE_RX762,
	MUX_BOARD_TYPE_RX,

	/* */	
	MUX_BOARD_TYPE_774,
	MUX_BOARD_TYPE_767,
	MUX_BOARD_TYPE_768,
	MUX_BOARD_TYPE_UNKNOWN
}MUX_BOARD_TYPE;


typedef	enum _MEDIA_DEVICE_TYPE
{
	MEDIA_DEVICE_USBDISK = 0, 		/* USB disk is default device */
	MEDIA_DEVICE_SDCARD,
	MEDIA_DEVICE_FTP,
	MEDIA_DEVICE_UNKNOWN
}MEDIA_DEVICE_T;

/* also used as the stream index of capture media and capture feed s*/
typedef	enum _MUX_MEDIA_TYPE
{
	MUX_MEDIA_TYPE_VIDEO = 0,
	MUX_MEDIA_TYPE_AUDIO,
	MUX_MEDIA_TYPE_SUBTITLE,
	MUX_MEDIA_TYPE_ERROR,
}MUX_MEDIA_TYPE;


/*
* ESC (27, 0x1b) charactor is '\e' or '\x1b'
*/



#define ANSI_COLOR_RED				"\x1b[31m"	/* ESC[31m : red */
#define ANSI_COLOR_GREEN			"\x1b[32m"
#define ANSI_COLOR_YELLOW			"\x1b[33m"
#define ANSI_COLOR_BLUE			"\x1b[34m"
#define ANSI_COLOR_MAGENTA		"\x1b[35m"
#define ANSI_COLOR_CYAN			"\x1b[36m"
#define ANSI_COLOR_RESET			"\x1b[0m"


#define	ERROR_TEXT_BEGIN			"\t\e[31m"
#define	ERROR_TEXT_END			"\e[0m"


#define	WARN_TEXT_BEGIN			""ANSI_COLOR_MAGENTA""

#define	INFO_TEXT_BEGIN			""ANSI_COLOR_YELLOW""



#define		STR_BOOL_VALUE(bValue)	\
			(((bValue)==0)?"NO":"YES")


#if	ARCH_ARM
#define	RUN_HOME_DIR						""
#define	RUN_HOME_DIR_TEMP				""
#else
//#define	RUN_HOME_DIR						ROOT_DIR"/releases"
#define	RUN_HOME_DIR						"./releases"
#define	RUN_HOME_DIR_TEMP				RUN_HOME_DIR"/x86"	/* this directory will not be packed into release version of ARM */
#endif

#define	CONFIG_FILE_HOME_PROJECT			RUN_HOME_DIR"/etc/mLab/"


#define	MUX_FACTORY_FLAGS_FILE			RUN_HOME_DIR"/var/factory.sh"

#ifndef EXT_CONFIG_FILE_PTPD
#define	EXT_CONFIG_FILE_PTPD				RUN_HOME_DIR"/etc/mLab/muxPtpd.cfg"
#endif

#define	PTP_EXE								"muxPtpd"

#define ADD_ELEMENT(header, element)	\
	if (header == NULL){					\
		header  = element;				\
		element->next   = NULL;			\
	}									\
	else	{								\
		element->next   = header;		\
		header = element;				\
	}


#define REMOVE_ELEMENT(header, element, type)	\
{type **cp, *c1; cp = &header; \
	while ((*cp) != NULL){  c1 = *cp; \
		if (c1 == element ) \
		{ *cp = element->next;} \
		else	{ cp = &c1->next;} \
	}; }


#define	GET_BIT(value, bit)				(((value)>>(bit))&0x01)
#define	SET_BIT(value, bit)				((value) << (bit))



#define	CMN_SET_BIT(flags, bitPosition)	\
		flags |= SET_BIT(0x01, (bitPosition) ) 

#define	CMN_CLEAR_BIT(flags, bitPosition)	\
		flags &= ~(SET_BIT(0x01, (bitPosition) ) )	

#define	CMN_CHECK_BIT(flags, bitPosition)	\
		( (flags&SET_BIT(0x01,(bitPosition) ) )!=0 )

#define	CMN_SET_FLAGS(flags, value)	\
		flags |= (value) 

#define	CMN_CLEAR_FLAGS(flags, value)	\
		flags &= ~((value) ) )	

#define	CMN_CHECK_FLAGS(flags, value)	\
		( (flags&(value) )!=0 )


#include "cmnLogUtils.h"

int cmn_log_init(log_stru_t *lobj);


#if CMN_SHARED_DEBUG
	#ifndef	TRACE
	#define	TRACE()	CMN_MSG_LOG(CMN_LOG_DEBUG,"" )
	#endif
#else
	#ifndef	TRACE
	#define	TRACE()	do{}while(0)
	#endif
#endif

//#define __FILENAME__ (strrchr(__FILENAME__, '/') ? strrchr(__FILENAME__, '/') + 1 : __FILENAME__)


#define	FILE_TO_STDOUT			0

#ifndef   __CMN_RELEASE__
#if FILE_TO_STDOUT
#define	CMN_MSG_LOG(level, format ,...)   \
	do{ char buf[4096]; snprintf(buf, sizeof(buf), ERROR_TEXT_BEGIN  "%s.[ERR, %s]:[%s-%u.%s()]: %s" ERROR_TEXT_END "\n", sysTimestamp(), cmnThreadGetName(), __FILENAME__, __LINE__, __FUNCTION__, format ) ;\
		printf(buf, ##__VA_ARGS__);			\
		}while(0)
#else
#define	CMN_MSG_LOG(level, format ,...)   \
	do{ 					\
		if ( level <= CMN_LOG_DEBUG && get_current_level() >= level) \
			log_information(level, __FILENAME__, __LINE__, format, ##__VA_ARGS__);	\
	}while(0)
#endif
#else
#define	CMN_MSG_LOG(level, format ,...) \
	do{ 					\
		if ( level <= CMN_LOG_DEBUG && get_current_level() >= level) \
			log_information(level, format, ##__VA_ARGS__);	\
	}while(0)
#endif

#ifndef   __CMN_RELEASE__
#if FILE_TO_STDOUT
#define	CMN_MSG_INFO(level, format ,...)   \
	do{ char buf[4096]; snprintf(buf, sizeof(buf), INFO_TEXT_BEGIN"%s.[INFO,%s]:[%s-%u.%s]: %s"ANSI_COLOR_RESET "\n", sysTimestamp(), cmnThreadGetName(),  __FILENAME__, __LINE__, __FUNCTION__, format ) ;\
		printf(buf, ##__VA_ARGS__);			\
		}while(0)
#else
#define	CMN_MSG_INFO(level, format ,...)   \
	do{ 					\
		if ( level <= CMN_LOG_DEBUG && get_current_level() >= level) \
			log_information(level, __FILENAME__, __LINE__, format, ##__VA_ARGS__);	\
	}while(0)
#endif
#else
#define	CMN_MSG_INFO(level, format ,...) \
	do{ 					\
		if ( level <= CMN_LOG_DEBUG && get_current_level() >= level) \
			log_information(level, format, ##__VA_ARGS__);	\
	}while(0)
#endif


#define	CMN_ABORT(format ,...)		\
			{CMN_MSG_LOG(CMN_LOG_ERR, format, ##__VA_ARGS__); CMN_MSG_LOG(CMN_LOG_ERR,"Program exit now!!!");exit(1);/*abort();*/}

#ifndef   __CMN_RELEASE__
#if FILE_TO_STDOUT
#define	CMN_MSG_DEBUG(level, format ,...) 	 \
	do{ char buf[4096]; snprintf(buf, sizeof(buf), "%s.[DBUG,%s]:[%s-%u.%s]: %s\n", sysTimestamp(), cmnThreadGetName(),  __FILENAME__, __LINE__, __FUNCTION__, format ) ;\
		printf(buf, ##__VA_ARGS__);			\
		}while(0)

	//printf(format, ##__VA_ARGS__)
#else
#define	CMN_MSG_DEBUG(level, format ,...) 	\
	do{ 					\
		if ( level <= CMN_LOG_DEBUG && get_current_level() >= level) \
			log_information(level, __FILENAME__, __LINE__, format, ##__VA_ARGS__);	\
	}while(0)
#endif

#else
#define	CMN_MSG_DEBUG(level, format ,...) 	{}
#endif


#define	IS_STRING_NULL( str)	\
	 ((str)==NULL) 


#define	IS_STRING_NULL_OR_ZERO( str)	\
	(((str)==NULL)||(!strlen((str))) )


#define	IS_STRING_EQUAL( str1, str2)	\
	( !strcasecmp((str1), (str2)))


int safe_open (const char *pathname,int flags);
void safe_rewind (int fd, int offset, const char *filename);
void safe_read (int fd,const char *filename,void *buf, size_t count, int verbose);


/* time for warm-reset, when both set to 0, it set to maxium: about 64 seconds */
int cmn_watchdog_enable(int seconds, int millseconds);
int cmn_watchdog_disable();
int cmn_watchdog_update();
void cmn_beep_long(int times);
void cmn_beep_short(int times);
int cmn_get_button_states(void);
int cmn_ver_log( char *modulename, char *info);
int cmn_ver_opt (int argc, char **argv, char *info);
int cmn_ver_debug( char *verInfo);

int cmn_write_serial_no (unsigned char *serNO);
int cmn_read_serial_no(unsigned char *read_serial_no, int length);

#define FLASH_SERIAL_OFFSET 			131072L		/* offset is 128 K bytes */
#define FLASH_SERIAL_DEVICE			"/dev/mtd0"	

/* cmd-line flags */
#define FLAG_NONE			0x00
#define FLAG_VERBOSE		0x01
#define FLAG_HELP			0x02
#define FLAG_FILENAME		0x04
#define FLAG_DEVICE			0x08

/* 6 blocks and 32K per block, total 192 K for bootloader partition in FLASH mtd 
* First 3 blocks are occupied by bootloader
*/
#define LOCK_START 			0
#define LOCK_SECTORS 		3


/* size of read/write buffer */
#define BUFSIZE 				(10 * 1024)
#define SERIAL_BUFSIZE 		20


#define	CMN_FLASH_MAGIC_START			0xAB
#define	CMN_FLASH_MAGIC_END			0xCD
#define	CMN_FLASH_MAGIC_LENGTH		2

#define	CMN_FLASH_ITEM_LENGTH			32
typedef	struct
{
	char		magicStart[CMN_FLASH_MAGIC_LENGTH];
	
	char		serialNo[CMN_FLASH_ITEM_LENGTH];
	char		userName[CMN_FLASH_ITEM_LENGTH];
	char		passwd[CMN_FLASH_ITEM_LENGTH];

	char		wanMac[CMN_FLASH_ITEM_LENGTH];
	char		wanIp[CMN_FLASH_ITEM_LENGTH];
	char		wanMask[CMN_FLASH_ITEM_LENGTH];

	char		lanMac[CMN_FLASH_ITEM_LENGTH];
	char		lanIp[CMN_FLASH_ITEM_LENGTH];
	char		lanMask[CMN_FLASH_ITEM_LENGTH];

	char		magicEnd[CMN_FLASH_MAGIC_LENGTH];
}CMN_FLASH_INFO_T;

extern	CMN_FLASH_INFO_T		factoryDefault;

int	cmn_default_write(CMN_FLASH_INFO_T *defaultInfo);
int	cmn_default_read(CMN_FLASH_INFO_T *defaultInfo);
void cmn_default_info_debug(CMN_FLASH_INFO_T *info);


#include "md5.h"
#include "base64.h"

#include <cmnOsPort.h>


/* return : 0 : success */
int	cmnLibInit(int	options);
void cmnLibDestroy(void);


#ifdef   __CMN_RELEASE__
#define	CMN_SHARED_INIT()	\
{ int options = 0;	\
	cmnLibInit(options);}
#else
#define	CMN_SHARED_INIT()	\
{ int options = 0;	\
	options |= CMNLIB_OPS_DEBUG;	\
	cmnLibInit(options);}
#endif	

uint64_t cmnGetTimeFromStartup(void);
uint64_t cmnGetTimeUs(void);

long cmnGetTimeMs(void);
char *cmnGetCtime1(char *buf2, size_t buf_size);

void cmnHexDump(const uint8_t *buf, int size);

#define	CMN_HEX_DUMP(buf, size, msg)		\
	do{MUX_INFO((msg)); cmnHexDump((buf), (size)); }while(0)
		

int	cmnParseGetHexIntValue(char *hexString);

unsigned int cmnMuxCRC32b(void *message, int len);

#include "cmnList.h"


#include "cJSON.h"

int cmnJsonGetIpAddress(cJSON* json, const char * key, uint32_t *ipAddress);
int cmnJsonGetStrIntoString(cJSON* json, const char * key, char *value, int size);
char* cmnGetStrFromJsonObject(cJSON* json, const char * key);
int cmnGetIntegerFromJsonObject(cJSON* json, const char * key);


void cmnForkCommand(const char *cmd);
int cmnKillProcess(char * processName);

/* parse configuration file */
void cmnParseGetArg(char *buf, int buf_size, const char **pp);
int cmnParseGetIntValue( const char **pp);
int cmnParseGetBoolValue( const char **pp);
int cmnParseGetIpAddress(struct in_addr *ipAddress, const char *p);


#define  MUX_DEBUG(...)		{CMN_MSG_DEBUG(CMN_LOG_DEBUG, __VA_ARGS__);}

#define  MUX_INFO(...)		{CMN_MSG_INFO(CMN_LOG_INFO, __VA_ARGS__);}

#define  MUX_WARN(...)		{CMN_MSG_INFO(CMN_LOG_WARNING, __VA_ARGS__);}

#define  MUX_ERROR(...)		{CMN_MSG_LOG(CMN_LOG_ERR, __VA_ARGS__);}


#define	MUX_MAIN_INFO(muxMain, format, message...) \
	if(MUX_MAIN_IS_DEBUG_MSG((muxMain)) ){MUX_INFO((format), ##message);}



extern volatile int recvSigalTerminal;


#define	SYSTEM_SIGNAL_EXIT() \
		{ recvSigalTerminal = TRUE; \
		}

#define	SYSTEM_IS_EXITING() \
		(recvSigalTerminal == TRUE)


#if MUX_OPTIONS_DEBUG_IP_COMMAND			
#define	MUX_DEBUG_JSON_OBJ(obj)	\
	{ char *printedStr =cJSON_Print((obj));  MUX_DEBUG("JSON object %s :\n'%s'", #obj, printedStr);	\
		free(printedStr);}

#else
#define	MUX_DEBUG_JSON_OBJ(obj)					{}

#endif


char *cmn_read_file(const char *filename, uint32_t *size);
int cmn_write_file(const char *filename, void *data, uint32_t size);
int cmn_count_file(const char *filename);


char *cmnTimestampStr(void);


/* Definitions from include/linux/timerfd.h */
//#define TFD_TIMER_ABSTIME	(1 << 0)

#define	EXT_CLOCK_ID						CLOCK_REALTIME

#define	EXT_CLOCK_ID_MONO				CLOCK_MONOTONIC

int timerfd_create(int clockid, int flags);
int timerfd_set_time(int fd, long mstime);


char *strnstr(const char *haystack, const char *needle, size_t len);

#define	CMN_THREAD_NAME_MAIN				"muxMain"
#define	CMN_THREAD_NAME_TIMER				"muxTimer"
#define	CMN_THREAD_NAME_COMM				"muxBroker"
#define	CMN_THREAD_NAME_MANAGER			"muxManager"
#define	CMN_THREAD_NAME_SDP_MANAGER		"muxSdpMngr"
#define	CMN_THREAD_NAME_SDP_RECEIVER		"muxSdpRecv"
#define	CMN_THREAD_NAME_BUTTON				"muxButton"
#define	CMN_THREAD_NAME_LED					"muxLed"
#define	CMN_THREAD_NAME_POLL				"muxPoll"


#define	WITH_ANCILLIARY_STREAM			0


#ifdef __cplusplus
};
#endif

#endif

