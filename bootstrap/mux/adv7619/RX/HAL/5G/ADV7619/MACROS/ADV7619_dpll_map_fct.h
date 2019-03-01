/**********************************************************************************************
*																						      *
* Copyright (c) 2008 - 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
/*******************************************************************************
*                                                                              *
*                                                                              *
* This software is intended for use with the ADV7619 part only.                *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*                                                                              *
* FILE AUTOMATICALLY GENERATED.                                                *
* DATE: 22 Jul 2010 16:4:26                                                    *
*                                                                              *
*******************************************************************************/


#ifndef ADV7619_DPLL_MAP_FCT_H
#define ADV7619_DPLL_MAP_FCT_H

#include "ADV7619_cfg.h"

#define VRX_get_MCLK_FS_N(pval)                             ATV_I2CGetField8(VRX_DPLL_MAP_ADDR, 0xB5, 0x7, 0, pval)
#define VRX_ret_MCLK_FS_N()                                 ATV_I2CReadField8(VRX_DPLL_MAP_ADDR, 0xB5, 0x7, 0)
#define VRX_set_MCLK_FS_N(val)                              ATV_I2CWriteField8(VRX_DPLL_MAP_ADDR, 0xB5, 0x7, 0, val)

#define VRX_set_VIDEO_CORE_ENABLE_MAN_EN(val)               ATV_I2CWriteField8(VRX_DPLL_MAP_ADDR, 0xC3, 0x80, 0x7, val)
#define VRX_set_VIDEO_FREQ_LIMIT_DISABLE(val)               ATV_I2CWriteField8(VRX_DPLL_MAP_ADDR, 0xCF, 0x02, 0x1, val)
#define VRX_set_AUDIO_FREQ_LIMIT_DISABLE(val)               ATV_I2CWriteField8(VRX_DPLL_MAP_ADDR, 0xCF, 0x01, 0x0, val)

#define VRX_set_DPLL_REG_DB(val)                            ATV_I2CWriteField8(VRX_DPLL_MAP_ADDR, 0xDB, 0x80, 0x7, val)

#endif
