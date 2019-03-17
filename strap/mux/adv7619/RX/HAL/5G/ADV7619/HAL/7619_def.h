/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#ifndef _7619_DEF_H_
#define _7619_DEF_H_

/*============================================
 * General defines
 *===========================================*/
#define RX_NOF_PORTS                2
#define RX_MAX_NOF_KSVS             24
#define RX_FAST_SWITCHING_DEVICE    1
#define RX_DEV_NAME                 "ADV7619"
#define FAST_SWITCH_DEVICE          1
#define RX_NOF_SLCRS                0
/*============================================
 * Engineering Samples
 *===========================================*/
#define RX_DEV_REV_ES1              0
#define RX_DEV_REV_ES2              1
#define RX_DEV_REV_ES3              2
#define RX_DEV_REV_ES4              3

#if (RX_DEVICE == 7619)
#if (RX_ES_ID <= 1)
#define ES1_7619                    1
#endif
#endif

/*===========================================
 * HDMI Port Macros
 *===========================================*/
#define RX_VALID_PORT(a)            ((UCHAR)a<RX_NOF_PORTS)
#define RX_PORT_INDEX(a)            ((UCHAR)a)
#define RX_PORT_ID(a)               ((UCHAR)a)
#define RX_PORT_NAME(a)             ((RX_HDMI_PORT)(a))

#endif
