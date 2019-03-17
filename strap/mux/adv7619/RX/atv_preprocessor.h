/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/

#ifndef _ATV_PREPROCESSOR_H_
#define _ATV_PREPROCESSOR_H_

/* This File defines the configuration options for the Repeater (REP) Driver Application*/
/* The values of these definitions are currently set as per ADI evaluation platforms and the ADI example application*/
/* If the ADI example application is ported and customized it is important to review and set these options accordingly*/
/* Please refer to release build options file and DVP documentation for further information*/

#define RX_DEVICE           7619  /*Set the Target Rx  device. Rep Driver supports the following devices:   7612, 7611, 7614, 7622/3, 7619, 7630 */
#define RX_USER_CONFIG         1  /*Always set to 1 when using ADI REP Middleware and ADI REP Application. */
#define RX_USER_INIT           0  /*Always set to 0. Set to 1 for customer initialization customization only. */
#define RX_ES_ID               0  /*For unreleased products set to 1,2 for ES1 and ES2 legacy Engineering samples only. Set to 0 for released products*/

#define TX_DEVICE           7511  /*Set the Target Tx Device. Rep  Driver supports the following Tx devices: 7623/2, 7511, 7510, 7520 */
#define TX_USER_CONFIG         1  /*Always set to 1 when using ADI REP Middleware and ADI REP Application. */
#define TX_USER_INIT           0  /*Always set to 0. Set to 1 for customer initialization customization only. */

#define REP_USER_CONFIG        0  /*Always set to 0 unless re-defining input ports priority */

#define MUX_DEVICE             0  /*Set to 3014 for ADV3014 HDMI MUX applications only. */
#define MUX_USER_CONFIG        0
#define MUX_USER_INIT          0
#define MUX_ES_ID              3  /*Set to 3 for released MUX part */

#define UART_DEBUG             1  /*Set to 1 to enable Debug message Printouts. Set to 0 to disable.  */
#define MUTE_DEBUG             1  /*Set to 1 to enable Mute System Debug message Printouts. Set to 0 to disable.  */

#define IGNORE_INT_LINES       1  /*Set to 1 to ignore hw interrupt pin status to determine if interrupt is pending. Software method used only*/
#define RX_HW_INT_ID           1  /*Set the Receiver interrupt line/pin to which interrupt pending status will be mapped.*/
#define STRICT_TIMING          0
#define RX_INCLUDE_VBI         0
#define MULTI_RX_NUM           0  /*Set to Zero except for Multi RX Support*/





#endif /* _ATV_PREPROCESSOR_H_ */

