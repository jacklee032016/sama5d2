#ifndef	__MUX7XX_COMPACT_H__
#define	__MUX7XX_COMPACT_H__


#define	MUX_BOARD_768			2
#define	MUX_BOARD_774			3
#define	MUX_BOARD_767			4

#define	MUX_BOARD				MUX_BOARD_774

#define	MUX_NAME				"Mux"

#if	(MUX_BOARD == MUX_BOARD_768)
	#define	EXT_SYSTEM		MUX_NAME"768"
#elif (MUX_BOARD == MUX_BOARD_774)
	#define	EXT_SYSTEM		MUX_NAME"774"
#elif (MUX_BOARD == MUX_BOARD_767)
	#define	EXT_SYSTEM		MUX_NAME"767"
#else
#error 	Not support board definition
#endif


/** Debug level: ALL messages*/
#define	EXT_DBG_LEVEL_ALL				0x00
#define	EXT_DBG_LEVEL_INFO			0x01
#define	EXT_DBG_LEVEL_WARN			0x02
#define	EXT_DBG_LEVEL_ERROR			0x03
#define	EXT_DBG_LEVEL_FATAL			0x04


#define	EXT_DBG_MASK_LEVEL			0x04
#define	EXT_DBG_LEVEL_OFF				EXT_DBG_LEVEL_ALL


#define	EXT_DBG_ON					0x80U
#define	EXT_DBG_OFF					0x00U


#define	EXT_DBG_TRACE					0x40U
#define	EXT_DBG_STATE					0x20U
#define	EXT_DBG_FRESH					0x10U
#define	EXT_DBG_HALT					0x08U


/* level used */
#define	EXT_DBG_TYPES_ON				EXT_DBG_ON
#define	EXT_DBG_MIN_LEVEL			EXT_DBG_LEVEL_ALL

/*
* ESC (27, 0x1b) charactor is '\e' or '\x1b'
*/

#define ANSI_COLOR_RED				"\x1b[31m"	/* ESC[31m : red */
#define ANSI_COLOR_GREEN			"\x1b[32m"
#define ANSI_COLOR_YELLOW			"\x1b[33m"
#define ANSI_COLOR_BLUE			"\x1b[34m"
#define ANSI_COLOR_MAGENTA		"\x1b[35m"
#define ANSI_COLOR_CYAN			"\x1b[36m"
#define ANSI_COLOR_RESET			"\x1b[0m"	/* for all colors, other than red, this must be used. April,15,2018. JL*/


#define	ERROR_TEXT_BEGIN			"\t\e[31m ERR:"
#define	ERROR_TEXT_END			"\e[0m"


#define	WARN_TEXT_BEGIN			""ANSI_COLOR_MAGENTA"WARN:"

#define	INFO_TEXT_BEGIN			""ANSI_COLOR_BLUE"INFO:"

#define	EXT_NEW_LINE							"\r\n"


#define	SYS_PRINT					dbg_printf

#ifndef	sysTaskName
#define	sysTaskName()				"TEST"
#endif

#ifndef __EXT_RELEASE__
	#define	EXT_PRINTF(x)						{SYS_PRINT x ;}
	
//	#define	EXT_DEBUGF(fmt, args...)	{printf("[%s-%u] DEBUG: " fmt EXT_NEW_LINE, __FILE__, __LINE__, ## args);}
	#define	EXT_DEBUGF(debug, message)		do { \
                               if ( \
                                   ((debug) & EXT_DBG_ON) && \
                                   ((debug) & EXT_DBG_TYPES_ON) && \
                                   ((short)((debug) & EXT_DBG_MASK_LEVEL) >= EXT_DBG_MIN_LEVEL)) { \
                                 _TRACE_OUT(message);SYS_PRINT(EXT_NEW_LINE); \
                                 if ((debug) & EXT_DBG_HALT) { \
                                   while(1); \
                                 } \
                               } \
                             } while(0)

                             
	#define	EXT_INFOF(message)		{SYS_PRINT(ANSI_COLOR_CYAN "%s:[%s-%u]:", sysTaskName(), __FILE__, __LINE__);EXT_PRINTF(message);SYS_PRINT((ANSI_COLOR_RESET EXT_NEW_LINE));}
	
	#define	EXT_ERRORF(message)		{SYS_PRINT(ERROR_TEXT_BEGIN "%s: ERROR:[%s-%u]:", sysTaskName(), __FILE__, __LINE__);EXT_PRINTF(message); SYS_PRINT((ERROR_TEXT_END  EXT_NEW_LINE));}

//	#define	EXT_ASSERT(x)				{printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__); while(1);}
	#define	EXT_ASSERT(msg, x)			{if((x)==0) {SYS_PRINT(ERROR_TEXT_BEGIN"%s: ASSERT: [%s-%u]:",  sysTaskName(), __FILE__, __LINE__ );SYS_PRINT msg ;SYS_PRINT((ERROR_TEXT_END EXT_NEW_LINE)); while(0){};}}
	#define	EXT_ABORT(fmt, args... )		SYS_PRINT("%s: ABORT in [" __FILE__ "-%u]:" fmt EXT_NEW_LINE, sysTaskName(), __LINE__, ##args );while(1){}
#else
	#define	EXT_PRINTF(x)						{;}

	#define	EXT_DEBUGF(debug, message)		{}

	#define	EXT_INFOF(message)				{SYS_PRINT message ;SYS_PRINT(EXT_NEW_LINE);}

	#define	EXT_ERRORF(message)				{SYS_PRINT message ; SYS_PRINT(EXT_NEW_LINE);}
	
//	#define	EXT_ASSERT(x)				{while (1);}
	#define	EXT_ASSERT(msg, x)				{}
	#define	EXT_ABORT(fmt, args... )		{}
#endif

#define	_TRACE_OUT(message)	\
			{EXT_PRINTF(("%s: [%s-%u.%s()]: ",  sysTaskName(), __FILE__, __LINE__, __FUNCTION__) );EXT_PRINTF(message); }

#define	TRACE()						_TRACE_OUT((EXT_NEW_LINE) )



#define	EXT_NEW_LINE							"\r\n"


#define	EXT_VERSION_DOT(a, b, c)				a ##.## b ##.## c

#define	EXT_VERSION(a, b, c)						EXT_VERSION_DOT(a, b, c)

#define	EXT_STRINGIFY(s)         					EXT_TOSTRING(s)
#define	EXT_TOSTRING(s)							#s


#define	EXT_GLUE(a, b)							a ## b
#define	EXT_JOIN(a, b)							EXT_GLUE(a, b)

#define	EXT_VERSION_MAJOR						1
#define	EXT_VERSION_MINOR						1
#define	EXT_VERSION_REVISION					1

#define	BL_VERSION_MAJOR						0
#define	BL_VERSION_MINOR						1
#define	BL_VERSION_REVISION					1


#define	EXT_VERSION_INFO()					((EXT_VERSION_MAJOR<<16)|(EXT_VERSION_MINOR<<8)|(EXT_VERSION_REVISION))


#define	EXT_VERSION_INTEGER()					((EXT_VERSION_MAJOR)<<16 | (EXT_VERSION_MINOR)<<8 | (EXT_VERSION_REVISION))


#define	BL_VERSION_TOKEN						EXT_VERSION(BL_VERSION_MAJOR, BL_VERSION_MINOR, BL_VERSION_REVISION)
#define	EXT_VERSION_TOKEN					EXT_VERSION(EXT_VERSION_MAJOR, EXT_VERSION_MINOR, EXT_VERSION_REVISION)


#define	BUILD_DATE_TIME			__DATE__" "__TIME__

#define	EXT_SYSTEM_STRING(sysName, verInfo) 		\
		EXT_NEW_LINE"" sysName" (" \
		"Version: " verInfo "; " \
		"Built: " BUILD_DATE_TIME ")"EXT_NEW_LINE


#define	EXT_VERSION_STRING		\
			EXT_STRINGIFY(EXT_VERSION_TOKEN)	

#define	EXT_OS_NAME		EXT_SYSTEM_STRING(EXT_SYSTEM, EXT_VERSION_STRING)

#endif

