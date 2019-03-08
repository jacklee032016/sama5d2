
#ifndef	__MUX_COMPACT_H__
#define	__MUX_COMPACT_H__


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

