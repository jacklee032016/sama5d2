/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#ifndef _ATV_TYPES_H_
#define _ATV_TYPES_H_

#include "atv_tal.h"
#include "atv_osal.h"
#include "atv_preprocessor.h"

/*=======================================
 * Data types
 *======================================*/
typedef unsigned char   UCHAR;          /* unsigned 8-bit */
typedef unsigned short  UINT16;         /* unsigned 16-bit */
typedef unsigned long   UINT32;         /* unsigned 32-bit */
typedef short int       INT16;
typedef long int        INT32;
typedef char            CHAR;

#define UINT8           UCHAR

#ifndef BOOL
#define BOOL            UCHAR
#endif

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifndef NULL
#define NULL            ((void *)0)
#endif


#endif
