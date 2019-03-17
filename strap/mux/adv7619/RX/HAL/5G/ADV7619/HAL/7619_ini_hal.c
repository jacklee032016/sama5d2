/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
/*============================================================================
 * This file contains all functions that are chip-specific
 *
 *===========================================================================*/
#include "atv_types.h"
#include "rx_lib.h"

#if (RX_DEVICE == 7619)

#include "rx_cfg.h"
#include "rx_isr.h"

/*===============================
 * External declarations
 *==============================*/

/*==========================================
 * Shared constants
 *=========================================*/
CONSTANT UCHAR RxPllInitTable[] = 
{
    /*===============================
     * PLL Recommendations
     *==============================*/  
    /*RX_I2C_HDMI_MAP_ADDR,      0x50, 0xC0,*/ /*PLL Setting Changed to 0x00 with RSD Dec 2013*/
    RX_I2C_HDMI_MAP_ADDR,      0x57, 0xA3, /*PLL Setting Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x58, 0x07, /*PLL Setting Synced with RSD Oct 2011*/


    0
};

CONSTANT UCHAR RxEqInitTable[] = 
{
    /*===============================
     * Equaliser Recommendations
     *==============================*/
    RX_I2C_HDMI_MAP_ADDR,      0x5A, 0x80,  /*ADI Recommended Eq Setting Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x84, 0x03,  /*As per ADV7619_Required_Settings_Manual_1.6_2014-12-19 */  
    RX_I2C_HDMI_MAP_ADDR,      0x85, 0x10,  /*ADI Recommended Eq Setting Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x86, 0x9B,  /*ADI Recommended Eq Setting Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x89, 0x03,  /*As per ADV7619_Required_Settings_Manual_1.6_2014-12-19 */
    RX_I2C_HDMI_MAP_ADDR,      0x9B, 0x03,  /*ADI Recommended Eq Setting Synced with RSD Oct 2011*/
        
    0
};

CONSTANT UCHAR RxHdmiAdiRecSetTable[] = 
{
    /*===============================
     * HDMI ADI Recommended Settings
     *==============================*/     
    RX_I2C_HDMI_MAP_ADDR,      0x3D, 0x10,  /*ADI Recommended HDMI Setting Synced with RSD Dec 2013 */
    RX_I2C_HDMI_MAP_ADDR,      0x3E, 0x69,  /*ADI Recommended HDMI Setting Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x3F, 0x46,  /*ADI Recommended HDMI Setting Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x4E, 0xFE,  /*ADI Recommended HDMI Setting *Updated RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x4F, 0x08,  /*ADI Recommended HDMI Setting Updated RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x93, 0x03,  /*ADI Recommended HDMI Setting: PRC Updated RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0xC0, 0x03,  /*ADI Recommended HDMI Setting: Power Setting  Synced with RSD Oct 2011*/
    /*RX_I2C_HDMI_MAP_ADDR,    0x83, 0xFC,*/  /*Set clock Termination: Removed. Clock Termination is handled separately by software.Updated RSD July 2011*/
    /*RX_I2C_HDMI_MAP_ADDR,    0xBA, 0xB2,*/  /*Set clock Termination: Removed. Clock Termination is handled separately by software Updated RSD July 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x03, 0x98,  /*As per ADV7619_Required_Settings_Manual_1.6_2014-12-19 */
    RX_I2C_HDMI_MAP_ADDR,      0x10, 0xA5,  /* Turn ON HDCP mute flag bit 7  Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x45, 0x04,  /* Set Burst Err Filter, prevent PS3 audio noise Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x4C, 0x44,  /* Vertical Filter selection: Added 18 October, due to Vsync lock instability resulting in 2 resolution detections*/
    RX_I2C_HDMI_MAP_ADDR,      0x97, 0xC0,  /* ADI Recommended HDMI Setting Synced with RSD Dec 2013*/
    RX_I2C_HDMI_MAP_ADDR,      0x6F, 0x08,  /* ADI ADI Recommended HDMI Setting Synced with RSD Dec 2013*/
    RX_I2C_IO_MAP_ADDR,        0x19, 0x83,  /* DLL setting- can be reset in application layer by end user as per hw */ 
    RX_I2C_IO_MAP_ADDR,        0x33, 0x40,  /* DLL setting- can be reset in application layer by end user as per hw */ 
    RX_I2C_CP_MAP_ADDR,        0xC8, 0x08,  /* Set digital fine clamp setting for HDMI Mode*/  
    RX_I2C_CP_MAP_ADDR,        0x6C, 0x00,  /* Disable Auto Clamping-Synced with RSD Oct 2011*/
    RX_I2C_HDMI_MAP_ADDR,      0x50, 0x00,  /* Adjust Gear Wait Time to Minimum HDCP @ 4k2k Interconnectivity*/
    0
};


/*===========================================================================
 * Initialize the part into a specific mode.
 *
 * Entry:   OperatingMode = Mode the part mus be set into
 *
 * Return:  ATVERR_OK
 *
 * Notes:   For the ADV7840 a hardware reset should be done 
 *          before this function is called         
 *
 *==========================================================================*/
ATV_ERR HAL_RxInit (RX_OP_MODE OperatingMode)
{
    UCHAR i;
    
    /*===============================================
     * Software init
     *==============================================*/    
    CurrVideoDetected = FALSE;
    CurrBurstLock     = FALSE;
    CurrSspdLock = FALSE;
    CurrSspdLockCh1 = FALSE;
    CurrSspdLockCh2 = FALSE;
    CurrIntPin = 0xFF;
    RxPrevOperatingMode = RX_OPMODE_PWRDWN;
    RxOperatingMode     = RX_OPMODE_PWRDWN;
    
    for (i=0; i<RX_ENABLE_EVENTS_SIZE; i++)
    {
        RxEnabledEvents[i] = 0;
    }
    
    /*===============================================
     * Apply Full SW reset
     *==============================================*/
     VRX_set_MAIN_RESET(0x1);
     HAL_DelayMs(5);

    /*===============================================
     * Set I2C address of I2C programmabled maps
     *==============================================*/
    HAL_SetI2cMapAddress(RX_CP_MAP, VRX_CP_MAP_ADDR);
    HAL_SetI2cMapAddress(RX_INFOFRAME_MAP, VRX_INFOFRAME_MAP_ADDR);
    HAL_SetI2cMapAddress(RX_CEC_MAP, VRX_CEC_MAP_ADDR);
    HAL_SetI2cMapAddress(RX_DPLL_MAP, VRX_AFE_DPLL_MAP_ADDR);
    HAL_SetI2cMapAddress(RX_HDMI_MAP, VRX_HDMI_MAP_ADDR);
    HAL_SetI2cMapAddress(RX_REPEATER_MAP, VRX_REPEATER_MAP_ADDR);
    HAL_SetI2cMapAddress(RX_EDID_MAP, VRX_EDID_MAP_ADDR);


    /*===============================================
     * Configure the part into the requested
     * operating mode
     *==============================================*/    
    HAL_SetOperatingMode(OperatingMode, NULL);

#if RX_USER_INIT
    /*====================================
     * Set User-specified init values
     *===================================*/
    HAL_RxI2CWriteTable ((UINT8 *)UserRxRegInitTable, 0);
    HAL_RxI2CWriteFields ((UINT8 *)UserRxFieldInitTable, 0);
#endif
    return (ATVERR_OK);
}

/*==========================================================================
 * 
 *
 * Entry:   
 *
 * Return:  ATVERR_OK
 *          ATVERR_FAILED
 *          ATVERR_INV_PARM
 *
 * Notes:
 *
 *=========================================================================*/
ATV_ERR HAL_SetI2cMapAddress(RX_I2C_MAP  Map, UINT8 Address)
{
    ATV_ERR ErrVal = ATVERR_OK;

    Address >>= 1;
    switch (Map)
    {
        case RX_CP_MAP:
            VRX_set_CP_SLAVE_ADDR(Address);
            break;
        case RX_HDMI_MAP:
            VRX_set_HDMI_SLAVE_ADDR(Address);
            break;
        case RX_REPEATER_MAP:
            VRX_set_KSV_SLAVE_ADDR(Address); 
            break;
        case RX_EDID_MAP:
            VRX_set_EDID_SLAVE_ADDR(Address);
            break;
        case RX_INFOFRAME_MAP:
            VRX_set_INFOFRAME_SLAVE_ADDR(Address);
            break;
        case RX_CEC_MAP:
            VRX_set_CEC_SLAVE_ADDR(Address);
            break;
        case RX_DPLL_MAP:
            VRX_set_DPLL_SLAVE_ADDR(Address);
            break;
        default:
            ErrVal = ATVERR_INV_PARM;
            break;
    }
    return (ErrVal);
}

/*==========================================================================
 * 
 *
 * Entry:   
 *
 * Return:  ATVERR_OK
 *          ATVERR_FAILED
 *          ATVERR_INV_PARM
 *
 * Notes:   Ideally the part should be reset before calling this function
 *
 *=========================================================================*/
ATV_ERR HAL_SetOperatingMode (RX_OP_MODE OperatingMode, RX_OP_MODE_CFG *OpModeCfg)
{
    BOOL DisPowerMgt;
    BOOL DisTriStateMgt;
    RX_OP_MODE TmpOperatingMode;

    if (OpModeCfg == NULL)
    {
       DisPowerMgt = FALSE;
       DisTriStateMgt = FALSE;
    }
    else
    {
        DisPowerMgt = OpModeCfg->DisPowerMgt;
        DisTriStateMgt = OpModeCfg->DisTriStateMgt;
    }

    switch(OperatingMode)
    {
        case RX_OPMODE_PWRDWN:
            if (!DisPowerMgt)
            {
                VRX_set_POWER_DOWN(0x1);
            }
            break;
        case RX_OPMODE_PWRUP:
            if(RxOperatingMode != RX_OPMODE_PWRDWN)
            {
                return (ATVERR_FAILED);
            }
            VRX_set_POWER_DOWN(0);
            TmpOperatingMode = RxPrevOperatingMode;
            RxPrevOperatingMode = RxOperatingMode; 
            RxOperatingMode = TmpOperatingMode;
            return (ATVERR_OK);
        case RX_OPMODE_HDMI:
            if(!DisPowerMgt)
            {
                VRX_set_POWER_DOWN(0x0);                
            }
            VRX_set_PRIM_MODE(0x6);
            VRX_set_VID_STD(0x2);
            if(!DisTriStateMgt)
            {
                VRX_set_TRI_PIX(0);
                VRX_set_TRI_LLC(0);
                VRX_set_TRI_SYNCS(0);
                VRX_set_TRI_AUDIO(0);
            }    
            HAL_RxI2CWriteTable((UCHAR *)RxPllInitTable,0);
            HAL_RxI2CWriteTable((UCHAR *)RxEqInitTable,0);
            HAL_RxI2CWriteTable((UCHAR *)RxHdmiAdiRecSetTable,0);
            break;
        default:
            return (ATVERR_INV_PARM);
    }
    if(RxOperatingMode != OperatingMode)
    {  
        RxPrevOperatingMode = RxOperatingMode; 
        RxOperatingMode = OperatingMode;
    }
    return (ATVERR_OK);
}

#endif
