/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#ifndef _7619_HAL_H_ 
#define _7619_HAL_H_

#if (RX_DEVICE==7619)

#include "ADV7619_cfg.h"
#include "ADV7619_lib.h"
#include "rx_5g_hal.h"

#define VRX_is_DIS_I2S_ZERO_COMPR_true()        ATV_I2CIsField8(VRX_HDMI_MAP_ADDR, 0x03, 0x80, 0x7)
#define VRX_get_DIS_I2S_ZERO_COMPR(pval)        ATV_I2CGetField8(VRX_HDMI_MAP_ADDR, 0x03, 0x80, 0x7, pval)
#define VRX_ret_DIS_I2S_ZERO_COMPR()            ATV_I2CReadField8(VRX_HDMI_MAP_ADDR, 0x03, 0x80, 0x7)
#define VRX_set_DIS_I2S_ZERO_COMPR(val)         ATV_I2CWriteField8(VRX_HDMI_MAP_ADDR, 0x03, 0x80, 0x7, val)

#define VRX_is_LLC_DLL_DOUBLE_true()            ATV_I2CIsField8(VRX_IO_MAP_ADDR, 0x19, 0x40, 0x6)
#define VRX_get_LLC_DLL_DOUBLE(pval)            ATV_I2CGetField8(VRX_IO_MAP_ADDR, 0x19, 0x40, 0x6, pval)
#define VRX_ret_LLC_DLL_DOUBLE()                ATV_I2CReadField8(VRX_IO_MAP_ADDR, 0x19, 0x40, 0x6)
#define VRX_set_LLC_DLL_DOUBLE(val)             ATV_I2CWriteField8(VRX_IO_MAP_ADDR, 0x19, 0x40, 0x6, val)

#define VRX_get_MAN_OP_CLK_SEL(pval)            ATV_I2CGetField8(VRX_IO_MAP_ADDR, 0xDD, 0x70, 4, pval)
#define VRX_ret_MAN_OP_CLK_SEL()                ATV_I2CReadField8(VRX_IO_MAP_ADDR, 0xDD, 0x70, 4)
#define VRX_set_MAN_OP_CLK_SEL(val)             ATV_I2CWriteField8(VRX_IO_MAP_ADDR, 0xDD, 0x70, 4, val)


#define VRX_get_MAN_OP_CLK_SEL_EN(pval)         ATV_I2CGetField8(VRX_IO_MAP_ADDR, 0xDD, 0x80, 7, pval)
#define VRX_ret_MAN_OP_CLK_SEL_EN()             ATV_I2CReadField8(VRX_IO_MAP_ADDR, 0xDD, 0x80, 7)
#define VRX_set_MAN_OP_CLK_SEL_EN(val)          ATV_I2CWriteField8(VRX_IO_MAP_ADDR, 0xDD, 0x80, 7, val)

#define VRX_get_LLC_DELAY(pval)                 ATV_I2CGetField8(VRX_IO_MAP_ADDR, 0x33, 0x0F, 0x0, pval)
#define VRX_ret_LLC_DELAY()                     ATV_I2CReadField8(VRX_IO_MAP_ADDR, 0x33, 0x0F, 0x0)
#define VRX_set_LLC_DELAY(val)                  ATV_I2CWriteField8(VRX_IO_MAP_ADDR, 0x33, 0x0F, 0x0, val)

/*==========================================
 *Adding Manual Equaliser Controls
 *=========================================*/

#define VRX_set_EQ_MAN_VALUE_PORT(val)         ATV_I2CWriteField8(VRX_HDMI_MAP_ADDR, 0x97, 0x7, 0x0, val)
#define VRX_set_EQ_LOAD_MAN_VALUE(val)         ATV_I2CWriteField8(VRX_HDMI_MAP_ADDR, 0x5A, 0x07, 0x0, val)
#define VRX_set_EQ_MAN_PORT(val)               ATV_I2CWriteField8(VRX_HDMI_MAP_ADDR, 0x97, 0x80, 0x7, val)
#define VRX_set_EQ_AGC_MODE_PORT(val)          ATV_I2CWriteField8(VRX_HDMI_MAP_ADDR, 0x91, 0x70, 0x4, val)
#define VRX_set_EQ_AGC_MODE(val)               ATV_I2CWriteField8(VRX_HDMI_MAP_ADDR, 0x85, 0x01, 0x0, val)
#define VRX_set_EQ_MAN_REACQUIRE_EN(val)       ATV_I2CWriteField8(VRX_HDMI_MAP_ADDR, 0x9C, 0x80, 0x7, val)
#define VRX_set_EQ_MAN_REACQUIRE(val)          ATV_I2CWriteField8(VRX_HDMI_MAP_ADDR, 0x9C, 0x40, 0x6, val)


#ifdef __cplusplus
extern "C" {
#endif





#ifdef __cplusplus
}
#endif

#endif

#endif
