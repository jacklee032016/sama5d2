/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#ifndef _RX_CONFIG_H_
#define _RX_CONFIG_H_

#include "rep_config.h"

#define RX_CALLBACK_FUNCTION            ADIAPI_RepRxNotification

#if MUX_DEVICE
#include "mux_config.h"
#endif

                            /* This defines the function that will be called */
                            /* by the RX driver to notify the application of */
                            /* changes in RX operating conditions. Comment-  */
                            /* out if not required                           */

#endif
