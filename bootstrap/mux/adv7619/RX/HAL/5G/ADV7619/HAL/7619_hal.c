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





/*============================================================================
 *
 * Entry:   None
 *  
 * Return:  void
 *
 * Notes:   None
 *
 *===========================================================================*/
ATV_ERR HAL_SetOpPixFormat (RX_PIX_BUS_INFO* PixBusInfo,  RX_OP_MODE* OpMode)
{
   

    ATV_ERR ErrVal = ATVERR_OK;
    UINT8 TmpBusWidth=0;
    UINT8 TmpFormat=0;
    UINT8 TmpMode=0;
    UINT8 OpFormatSel; 
    ErrVal = ATVERR_OK;
    
    switch(PixBusInfo->BitWidth)
    {
        case RX_BUS_BITS_8:
            TmpBusWidth = 0x0;
            break;
        case RX_BUS_BITS_10:
            TmpBusWidth = 0x1;
            break;
        case RX_BUS_BITS_12:
            TmpBusWidth = 0x2;
            break;
        default:
            ErrVal=  ATVERR_INV_PARM;   
            break;
    }

    switch(PixBusInfo->Mode)
    {
        case RX_BUS_MODE0:
            TmpMode = 0x0;
            break;
        case RX_BUS_MODE1:
            TmpMode = 0x1;
            break;
        case RX_BUS_MODE2:
            TmpMode = 0x2;
            break;
        case RX_BUS_MODE4:
            TmpMode = 0x4;
            break;
        case RX_BUS_MODE5:  /*48 Bit Pixel Mode*/
            TmpMode = 0x5;
            break;
        default:
            ErrVal=  ATVERR_INV_PARM;   
            break;
    }
    
    switch(PixBusInfo->Format)
    {
        case RX_BUS_OP_MODE_422PSDR:
            TmpFormat= 0x00;
            break;
        case RX_BUS_OP_MODE_422DDR:
            TmpFormat= 0x01;
            break;
        case RX_BUS_OP_MODE_444SDR:
            TmpFormat= 0x02;
            break;
        case RX_BUS_OP_MODE_444DDR:
            TmpFormat= 0x03;
            break;
        case RX_BUS_OP_MODE_422SDR:
            TmpFormat= 0x04;
            break;
        default:
            ErrVal=  ATVERR_INV_PARM;   
            break;
    }

    OpFormatSel= ( TmpFormat << 5) | (TmpMode <<2)| (TmpBusWidth);
    VRX_set_OP_FORMAT_SEL(OpFormatSel);
      
    switch(PixBusInfo->ChOrder)
    {
        case RX_BUS_OP_CH_GBR:

             VRX_set_OP_CH_SEL(0x0);
             break;     
        case RX_BUS_OP_CH_GRB:
             VRX_set_OP_CH_SEL(0x1);
             break;
        case RX_BUS_OP_CH_BGR:
             VRX_set_OP_CH_SEL(0x2);
             break;
        case RX_BUS_OP_CH_RGB:
             VRX_set_OP_CH_SEL(0x3);
             break;
        case RX_BUS_OP_CH_BRG:
             VRX_set_OP_CH_SEL(0x4);
             break;
        case RX_BUS_OP_CH_RBG:
             VRX_set_OP_CH_SEL(0x5);
             break;
        default:
              ErrVal=  ATVERR_INV_PARM; 
              break;
    }
    VRX_set_PIXBUS_MSB_TO_LSB_REORDER(PixBusInfo->BitReorder);
    VRX_set_OP_SWAP_CB_CR(PixBusInfo->CrCbSwap);
    return (ErrVal);
}

ATV_ERR HAL_RxHdmiEnAudioClock (BOOL Enable)
{
    /*VRX_set_audio_core_pdn(Enable); */
     return (ATVERR_NOT_AVAILABLE);
}

#endif
