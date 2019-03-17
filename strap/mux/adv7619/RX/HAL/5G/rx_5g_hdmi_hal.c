/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
/*============================================================================
 * This file contains all functions that are 5G family specific
 *
 *===========================================================================*/
#include "rx_lib.h"

#if ((RX_DEVICE == 7612) || (RX_DEVICE == 7611) || (RX_DEVICE == 7619))

#include "rx_isr.h"

/*==========================================
 * Local Macro Definitions
 *=========================================*/
#define GMD_VALID           ((LvlStatus2 | EdgStatus2) & LvlRaw2 & VRX_BIT_GAMUT_MDATA_ST)
#define GMD_NOT_REC         (LvlStatus2 & (~LvlRaw2) & VRX_BIT_GAMUT_MDATA_ST)
#define NOF_MT_MASK_REGS    3
/*==========================================
 * Local constants
 *=========================================*/
STATIC CONSTANT UCHAR IntBits[] =
{
    VRX_BIT_TMDS_CLK_A_ST, VRX_BIT_TMDSPLL_LCK_A_ST, VRX_BIT_CABLE_DET_A_ST, VRX_BIT_HDMI_ENCRPT_A_ST, VRX_BIT_AKSV_UPDATE_A_ST,
    VRX_BIT_TMDS_CLK_B_ST, VRX_BIT_TMDSPLL_LCK_B_ST, VRX_BIT_CABLE_DET_B_ST, VRX_BIT_HDMI_ENCRPT_B_ST, VRX_BIT_AKSV_UPDATE_B_ST
};
CONSTANT UINT8 RxHdmiMtMaskRegs[] =
{
    VRX_REG_MUTE_MASK_21_16,
    VRX_REG_MUTE_MASK_15_8,
    VRX_REG_MUTE_MASK_7_0,
    0, 0
};



CONSTANT MT_MASK_INFO RxMuteMskInfo[] =
{

     {RX_MT_COMPRS_AUD,    0x0,    VRX_BIT_MT_MSK_COMPRS_AUD},
     {RX_MT_AUD_MODE_CHNG, 0x0,    VRX_BIT_MT_MSK_AUD_MODE_CHNG},
     {RX_MT_PARITY_ERR,    0x0,    VRX_BIT_MT_MSK_PARITY_ERR},
     {RX_MT_VCLK_CHNG,     0x0,    VRX_BIT_MT_MSK_VCLK_CHNG},
     {RX_MT_APLL_UNLOCK,   0x1,    VRX_BIT_MT_MSK_APLL_UNLOCK},
     {RX_MT_APLL_UNLOCK,   0x1,    VRX_BIT_MT_MSK_VPLL_UNLOCK},
     {RX_MT_FLATLINE_DET,  0x1,    VRX_BIT_MT_MSK_FLATLINE_DET},
     {RX_MT_FIFO_UNDRFL,   0x1,    VRX_BIT_MT_MSK_FIFO_UNDERLFOW},
     {RX_MT_FIFO_OVRFL,    0x1,    VRX_BIT_MT_MSK_FIFO_OVERFLOW},
     {RX_MT_AV_MUTE,       0x2,    VRX_BIT_MT_MSK_AVMUTE},
     {RX_MT_DVI_MODE,      0x2,    VRX_BIT_MT_MSK_NOT_HDMIMODE},
     {RX_MT_NEW_CTS,       0x2,    VRX_BIT_MT_MSK_NEW_CTS},
     {RX_MT_NEW_N,         0x2,    VRX_BIT_MT_MSK_NEW_N},
     {RX_MT_CHMODE_CHNG,   0x2,    VRX_BIT_MT_MSK_CHMODE_CHNG},
     {RX_MT_APCKT_ECC_ERR, 0x2,    VRX_BIT_MT_MSK_APCKT_ECC_ERR},
     {RX_MT_PORT_CHNG,     0x2,    VRX_BIT_MT_MSK_CHNG_PORT},
     {RX_MT_VCLK_NO_DET,   0x2,    VRX_BIT_MT_MSK_VCLK_DET},
     {(RX_AUD_MT_MSK )0xff,  0xff,  0xFF}
};

/*===============================================
 * ADV7612 Rx Event - HDMI Interrupt Mask Mapping
 *==============================================*/



 CONSTANT UCHAR RxHdmiMaskRegs[] =
{
    VRX_REG_HDMI_LVL_INT_MASKB_1,   VRX_REG_HDMI_LVL_INT2_MASKB_1, LVL_MASK_B1,
    VRX_REG_HDMI_LVL_INT_MASKB_2,   VRX_REG_HDMI_LVL_INT2_MASKB_2, LVL_MASK_B2,
    VRX_REG_HDMI_LVL_INT_MASKB_3,   VRX_REG_HDMI_LVL_INT2_MASKB_3, LVL_MASK_B3,
    VRX_REG_HDMI_LVL_INT_MASKB_4,   VRX_REG_HDMI_LVL_INT2_MASKB_4, LVL_MASK_B4,
    VRX_REG_HDMI_EDG_INT_MASKB_1,   VRX_REG_HDMI_EDG_INT2_MASKB_1, EDG_MASK_B1,
    VRX_REG_HDMI_EDG_INT_MASKB_2,   VRX_REG_HDMI_EDG_INT2_MASKB_2, EDG_MASK_B2,
    VRX_REG_HDMI_EDG_INT_MASKB_3,   VRX_REG_HDMI_EDG_INT2_MASKB_3, EDG_MASK_B3,
    VRX_REG_HDMI_EDG_INT_MASKB_4,   VRX_REG_HDMI_EDG_INT2_MASKB_4, EDG_MASK_B4,
    VRX_REG_CEC_STATUS1_INT1_MASKB, VRX_REG_CEC_STATUS1_INT2_MASKB, INT_MASK_B3,
    0, 0
};
CONSTANT MASK_INFO RxMaskInfo[] =
{
    { RX_EVENT_HDMI_AVI_INFO,
        LVL_MASK_B1, {VRX_BIT_AVI_INFO_MB1,       VRX_BIT_AVI_INFO_MB2},
        EDG_MASK_B1, {VRX_BIT_NEW_AVI_INFO_MB1,   VRX_BIT_NEW_AVI_INFO_MB2}
    },
    { RX_EVENT_HDMI_AUDIO_INFO,
        LVL_MASK_B1, {VRX_BIT_AUDIO_INFO_MB1,     VRX_BIT_AUDIO_INFO_MB2},
        EDG_MASK_B1, {VRX_BIT_NEW_AUDIO_INFO_MB1, VRX_BIT_NEW_AUDIO_INFO_MB2}
    },
    { RX_EVENT_HDMI_SPD_INFO,
        LVL_MASK_B1, {VRX_BIT_SPD_INFO_MB1,       VRX_BIT_SPD_INFO_MB2},
        EDG_MASK_B1, {VRX_BIT_NEW_SPD_INFO_MB1,   VRX_BIT_NEW_SPD_INFO_MB2},
    },
    { RX_EVENT_HDMI_MS_INFO,
        LVL_MASK_B1, {VRX_BIT_MS_INFO_MB1,        VRX_BIT_MS_INFO_MB2},
        EDG_MASK_B1, {VRX_BIT_NEW_MS_INFO_MB1,    VRX_BIT_NEW_MS_INFO_MB2},
    },
    { RX_EVENT_HDMI_VS_INFO,
        LVL_MASK_B1, {VRX_BIT_VS_INFO_MB1,        VRX_BIT_VS_INFO_MB2},
        EDG_MASK_B1, {VRX_BIT_NEW_VS_INFO_MB1,    VRX_BIT_NEW_VS_INFO_MB2},
    },
    { RX_EVENT_HDMI_ACP_PCKT,
        LVL_MASK_B1, {VRX_BIT_ACP_PCKT_MB1,       VRX_BIT_ACP_PCKT_MB2},
        EDG_MASK_B1, {VRX_BIT_NEW_ACP_PCKT_MB1,   VRX_BIT_NEW_ACP_PCKT_MB2},
    },
    { RX_EVENT_HDMI_ISRC1_PCKT,
        LVL_MASK_B1, {VRX_BIT_ISRC1_PCKT_MB1,     VRX_BIT_ISRC1_PCKT_MB2},
        EDG_MASK_B1, {VRX_BIT_NEW_ISRC1_PCKT_MB1, VRX_BIT_NEW_ISRC1_PCKT_MB2},
    },
    { RX_EVENT_HDMI_ISRC2_PCKT,
        LVL_MASK_B1, {VRX_BIT_ISRC2_PCKT_MB1,     VRX_BIT_ISRC2_PCKT_MB2},
        EDG_MASK_B1, {VRX_BIT_NEW_ISRC2_PCKT_MB1, VRX_BIT_NEW_ISRC2_PCKT_MB2},
    },
    { RX_EVENT_HDMI_GAMUT_PCKT,
        LVL_MASK_B2, {VRX_BIT_GAMUT_MDATA_MB1,    VRX_BIT_GAMUT_MDATA_MB2},
        EDG_MASK_B2, {VRX_BIT_NEW_GAMUT_MDATA_MB1,VRX_BIT_NEW_GAMUT_MDATA_MB2},
    },
    { RX_EVENT_HDMI_NO_PACKET,
        LVL_MASK_B1, {0,                          0},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_AV_MUTE,
        LVL_MASK_B2, {VRX_BIT_AV_MUTE_MB1,        VRX_BIT_AV_MUTE_MB2},
        0xff, {0, 0}
    },

    { RX_EVENT_HDMI_N_CHNG,
        EDG_MASK_B2, {VRX_BIT_CHANGE_N_MB1,       VRX_BIT_CHANGE_N_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_CTS_PASS_THRSH,
        EDG_MASK_B2, {VRX_BIT_CTS_PASS_THRSH_MB1, VRX_BIT_CTS_PASS_THRSH_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_FIFO_OVERFLO,
        EDG_MASK_B2, {VRX_BIT_FIFO_OVERFLO_MB1,   VRX_BIT_FIFO_OVERFLO_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_FIFO_UNDERFLO,
        EDG_MASK_B2, {VRX_BIT_FIFO_UNDERFLO_MB1,  VRX_BIT_FIFO_UNDERFLO_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_FIFO_NEAROVFL,
        EDG_MASK_B2, {VRX_BIT_FIFO_NEAR_OVFL_MB1, VRX_BIT_FIFO_NEAR_OVFL_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_FIFO_NEARUFLO,
        EDG_MASK_B3, {VRX_BIT_FIFO_NEAR_UFLO_MB1, VRX_BIT_FIFO_NEAR_UFLO_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_AUDIO_MODE_CHNG,
        EDG_MASK_B3, {VRX_BIT_AUDIO_MODE_CHNG_MB1, VRX_BIT_AUDIO_MODE_CHNG_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_AUDIO_CH_MODE_CHNG,
        LVL_MASK_B2, {VRX_BIT_AUDIO_CH_MD_MB1,    VRX_BIT_AUDIO_CH_MD_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_INTERNAL_MUTE,
        LVL_MASK_B2, {VRX_BIT_INTERNAL_MUTE_MB1,  VRX_BIT_INTERNAL_MUTE_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_CS_DATA_VALID,
        LVL_MASK_B2, {VRX_BIT_CS_DATA_VALID_MB1,  VRX_BIT_CS_DATA_VALID_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_AUDIO_FLT_LINE,
        EDG_MASK_B3, {VRX_BIT_AUDIO_FLT_LINE_MB1, VRX_BIT_AUDIO_FLT_LINE_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_NEW_SAMP_RT,
        EDG_MASK_B3, {VRX_BIT_NEW_SAMP_RT_MB1,    VRX_BIT_NEW_SAMP_RT_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_PARITY_ERR,
        EDG_MASK_B3, {VRX_BIT_PARITY_ERROR_MB1,   VRX_BIT_PARITY_ERROR_MB2},
        0xff, {0, 0}
    },

    { RX_EVENT_HDMI_TMDS_CLK_CHNG,
        LVL_MASK_B3, {VRX_BIT_TMDS_CLK_A_MB1 | VRX_BIT_TMDS_CLK_B_MB1,
                     VRX_BIT_TMDS_CLK_A_MB2 | VRX_BIT_TMDS_CLK_B_MB2 },/*nnic*/
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_CABLE_DETECT,
        LVL_MASK_B3, {VRX_BIT_CABLE_DET_B_MB1 ,VRX_BIT_CABLE_DET_B_MB2 },
        LVL_MASK_B4, {VRX_BIT_CABLE_DET_A_MB1 ,VRX_BIT_CABLE_DET_A_MB2 }, /*nnic*/

    },
    { RX_EVENT_HDMI_VIDEO_PLL_LCK,
       LVL_MASK_B3, {VRX_BIT_TMDSPLL_LCK_A_MB1 | VRX_BIT_TMDSPLL_LCK_B_MB1 ,/*nnic*/
                    VRX_BIT_TMDSPLL_LCK_A_MB2 | VRX_BIT_TMDSPLL_LCK_B_MB2 },
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_DE_REGEN_LCK,
        LVL_MASK_B3, {VRX_BIT_DE_REGEN_LCK_MB1,   VRX_BIT_DE_REGEN_LCK_MB2}, /*nnic*/
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_V_LOCKED,
        LVL_MASK_B3, {VRX_BIT_V_LOCKED_MB1,       VRX_BIT_V_LOCKED_MB2}, /*nnic*/
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_ENC_CHNG,
        LVL_MASK_B4, {VRX_BIT_HDMI_ENCRPT_A_MB1 | VRX_BIT_HDMI_ENCRPT_B_MB1 , /*nnic*/
                     VRX_BIT_HDMI_ENCRPT_A_MB2 | VRX_BIT_HDMI_ENCRPT_B_MB2 },
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_MODE_CHNG,
        LVL_MASK_B2, {VRX_BIT_HDMI_MODE_MB1,      VRX_BIT_HDMI_MODE_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_NEW_TMDS_FRQ,
        EDG_MASK_B3, {VRX_BIT_NEW_TMDS_FRQ_MB1,   VRX_BIT_NEW_TMDS_FRQ_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_AKSV_UPDATE,
        EDG_MASK_B4, {VRX_BIT_AKSV_UPDATE_A_MB1 | VRX_BIT_AKSV_UPDATE_B_MB1 ,
                     VRX_BIT_AKSV_UPDATE_A_MB2 | VRX_BIT_AKSV_UPDATE_B_MB2
                     },
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_VCLK_CHNG,
        EDG_MASK_B3, {VRX_BIT_VCLK_CHNG_MB1,      VRX_BIT_VCLK_CHNG_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_DEEP_COLOR_CHNG,
        EDG_MASK_B3, {VRX_BIT_DEEP_COLOR_CHNG_MB1,VRX_BIT_DEEP_COLOR_CHNG_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_3D_PACKET,
        LVL_MASK_B3, {0,                          0},
        0xff, {0, 0}
    },
    { RX_EVENT_HDMI_NO_3D_PACKET,
        LVL_MASK_B3, {0,                          0},
        0xff, {0, 0}
    },
#if RX_INCLUDE_CEC
    { RX_EVENT_CEC_RX_MSG,
        INT_MASK_B3, {VRX_BIT_CEC_RX_RDY0_MB1, VRX_BIT_CEC_RX_RDY0_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_CEC_TX_DONE,
        INT_MASK_B3, {VRX_BIT_CEC_TX_READY_MB1, VRX_BIT_CEC_TX_READY_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_CEC_TX_ARB_LOST,
        INT_MASK_B3, {VRX_BIT_CEC_TX_ARBITRATION_LOST_MB1, VRX_BIT_CEC_TX_ARBITRATION_LOST_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_CEC_TX_TIMEOUT,
        INT_MASK_B3, {VRX_BIT_CEC_TX_RETRY_TIMEOUT_MB1, VRX_BIT_CEC_TX_RETRY_TIMEOUT_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_CEC_LOG_ADDR_ALLOC,
        INT_MASK_B3, {VRX_BIT_CEC_RX_RDY0_MB1             | VRX_BIT_CEC_TX_READY_MB1 |
                      VRX_BIT_CEC_TX_ARBITRATION_LOST_MB1 | VRX_BIT_CEC_TX_RETRY_TIMEOUT_MB1,
                      VRX_BIT_CEC_RX_RDY0_MB2             | VRX_BIT_CEC_TX_READY_MB2 |
                      VRX_BIT_CEC_TX_ARBITRATION_LOST_MB2 | VRX_BIT_CEC_TX_RETRY_TIMEOUT_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_CEC_LOG_ADDR_LIST,
        INT_MASK_B3, {VRX_BIT_CEC_RX_RDY0_MB1             | VRX_BIT_CEC_TX_READY_MB1 |
                      VRX_BIT_CEC_TX_ARBITRATION_LOST_MB1 | VRX_BIT_CEC_TX_RETRY_TIMEOUT_MB1,
                      VRX_BIT_CEC_RX_RDY0_MB2             | VRX_BIT_CEC_TX_READY_MB2 |
                      VRX_BIT_CEC_TX_ARBITRATION_LOST_MB2 | VRX_BIT_CEC_TX_RETRY_TIMEOUT_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_CEC_RX_MSG_RESPOND,
        INT_MASK_B3, {VRX_BIT_CEC_RX_RDY0_MB1             | VRX_BIT_CEC_TX_READY_MB1 |
                      VRX_BIT_CEC_TX_ARBITRATION_LOST_MB1 | VRX_BIT_CEC_TX_RETRY_TIMEOUT_MB1,
                      VRX_BIT_CEC_RX_RDY0_MB2             | VRX_BIT_CEC_TX_READY_MB2 |
                      VRX_BIT_CEC_TX_ARBITRATION_LOST_MB2 | VRX_BIT_CEC_TX_RETRY_TIMEOUT_MB2},
        0xff, {0, 0}
    },
#endif
    { RX_EVENT_CP_LCK,
        INT_MASK_B6, {VRX_BIT_CP_LOCK_CH1_MB1,   VRX_BIT_CP_LOCK_CH1_MB2},  /*nnic*/
        INT_MASK_B6, {VRX_BIT_CP_UNLOCK_CH1_MB1, VRX_BIT_CP_UNLOCK_CH1_MB2},
    },
    { RX_EVENT_CP_STDI_LCK,                                                     /*nnic*/
       INT_MASK_B6, {VRX_BIT_STDI_DVALID_CH1_MB1, VRX_BIT_STDI_DVALID_CH1_MB2},
        0xff, {0, 0}
    },
    { RX_EVENT_CP_LCK_SYNC_CH1,                                                 /*nnic*/
        INT_MASK_B6, {VRX_BIT_CP_LOCK_CH1_MB1,   VRX_BIT_CP_LOCK_CH1_MB2},
        INT_MASK_B6, {VRX_BIT_CP_UNLOCK_CH1_MB1, VRX_BIT_CP_UNLOCK_CH1_MB2},
    },
    { RX_EVENT_CP_STDI_LCK_SYNC_CH1,
        INT_MASK_B6, {VRX_BIT_STDI_DVALID_CH1_MB1, VRX_BIT_STDI_DVALID_CH1_MB2}, /*nnic*/
        0xff, {0, 0}
    },
    {
        (RX_EVENT)0xff,
        0xff, {0, 0},
        0xff, {0, 0}
    }
};
/*==========================================================================
 * Finds out all the active nad unmasked HDMI interrupts. It clears all the
 * active interupts on return.
 *
 * Entry:   RxHdmiInts = is a BOOL array that contains status of all the HDMI
 *                      interrupts
 *
 * Notes:  None
 *=========================================================================*/
void HAL_RxHdmiGetInterrupts (RX_HDMI_INTERRUPTS *HdmiInts)
{
    UCHAR i, Port, LvlRaw1, LvlRaw2, LvlRaw3, LvlRaw4;
    UCHAR LvlStatus1, LvlStatus2, LvlStatus3, LvlStatus4;
    UCHAR EdgStatus1, EdgStatus2, EdgStatus3, EdgStatus4;
    UCHAR PktValid, PktNotRec;
    UINT16 j;

    /*===================================================================
     * VRX_REG_HDMI_RAW_STATUS_1   0x60    RAW  status
     * VRX_REG_HDMI_INT_STATUS_1   0x61    INT  status
     * VRX_REG_HDMI_INT_CLEAR_1    0x62    INT  clear
     * VRX_REG_HDMI_INT2_MASKB_1   0x63    INT1 mask
     * VRX_REG_HDMI_INT_MASKB_1    0x64    INT2 mask
     *
     * Raw = 1 -> New (or same) packet was received
     *       0 -> No packets were received after n vsync from last packet
     * Int = Raw bit changed state. This does not mean new packet is available
     * Clr = Clear Int bit
     * New = New packet arrived (different from last packet)
     *
     * A packet is thus valid when:
     *  - Packet is available (Raw == 1) AND
     *  - New packet OR Same packet resumed (New == 1 OR Int == 1)
     *
     * A packet is not received when:
     *  - Packet is not available (Raw == 0) AND
     *  - Packet has just changed status to not available (Int == 1)
     *
     * NOTE: Possible errors can result from the following scenario:
     *  1- Packet is not received, Raw bit=0 and Int bit=1
     *  2- S/W reads Raw and Int registers to local variables
     *  3- The packet is received again and the Int and Raw bit is now 1
     *  4- S/W clears the Int bit, but the Raw bit stored locally is still 0
     *  The Raw bit change from 0->1 interrupt is missed. The Raw bits thus
     *  must be read AFTER the interrupt is cleared.
     *  Note however that for new packets edge interrupts, the raw status must
     *  be read BEFORE the interrupt is cleared, since clearing the interrupt
     *  will also clear the Raw bit.
     *
     * Group 1 and 2 are always valid as long as TMDS clock is detected:
     *  - TMDS_CLK_x_ST       Group 1
     *  - INTERNAL_MUTE       Group 2
     *  - VIDEO_PLL_LOCK      Group 2
     *  - AKSV                Group 2
     *
     * Group 3 is all remaining interrupts and are valid if:
     *  - TMDS clock is detected      0x6A[4:2] = 1
     *  - Video PLL is locked         0x6A[6]   = 1
     *
     * In addition, the following interrupts are valid if 0x66[7] = 1
     *  - Audio channel mode        0x66[4]
     *  - Audio mode change         0x66[3]
     *  - New audio sampling rate   0x7A[3]
     *
     *===================================================================*/

    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_INT_STATUS_1, &LvlStatus1);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_INT_STATUS_2, &LvlStatus2);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_INT_STATUS_3, &LvlStatus3);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_INT_STATUS_4, &LvlStatus4);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_EDG_INT_STATUS_1, &EdgStatus1);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_EDG_INT_STATUS_2, &EdgStatus2);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_EDG_STATUS_3,     &EdgStatus3);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_EDG_STATUS_4,     &EdgStatus4);


    /*==================================================================
     * Acknowledge all interrupts immediately so we don't lose any
     * See note above for why LvlRawx is read after clearing
     * the LvStatusx interrupts, but not for EdgRawx
     *=================================================================*/
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_INT_CLR_1, LvlStatus1);
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_INT_CLR_2, LvlStatus2);
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_INT_CLR_3, LvlStatus3);
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_INT_CLR_4, LvlStatus4);

    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_RAW_STATUS_1, &LvlRaw1);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_RAW_STATUS_2, &LvlRaw2);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_RAW_STATUS_3, &LvlRaw3);
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_LVL_RAW_STATUS_4, &LvlRaw4);

    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_EDG_INT_CLR_1, EdgStatus1);
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_EDG_INT_CLR_2, EdgStatus2);
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_EDG_INT_CLR_3, EdgStatus3);
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_HDMI_EDG_INT_CLR_4, EdgStatus4);

    Port = VRX_ret_HDMI_PORT_SELECT();
    j = 0;
    for (i=0; i<RX_NOF_PORTS; i++)
    {
        HdmiInts->TmdsClkChg[i] = (BOOL) (LvlStatus3 & IntBits[j]);
        HdmiInts->VidPllChg[i]  = (BOOL) (LvlStatus3 & IntBits[j+1]);
        HdmiInts->EncChange[i]  = (BOOL) (LvlStatus4 & IntBits[j+3]);
        HdmiInts->AksvUpdate[i] = (BOOL) (EdgStatus4 & IntBits[j+4]);
        HdmiInts->TmdsClock[i]  = (BOOL) (LvlRaw3 & IntBits[j]);
        HdmiInts->VidPllLock[i] = (BOOL) (LvlRaw3 & IntBits[j+1]);
        HdmiInts->CableDetect[i]= (BOOL) (LvlRaw4 & IntBits[j+2]);
        HdmiInts->Encrypted[i]  = (BOOL) (LvlRaw4 & IntBits[j+3]);

        if(i <1)
        {
            HdmiInts->CableDetChg[i]= (BOOL) (LvlStatus4 & IntBits[j+2]);
            HdmiInts->CableDetect[i]= (BOOL) (LvlRaw4 & IntBits[j+2]);
        }
        else
        {
             HdmiInts->CableDetChg[i]= (BOOL) (LvlStatus3 & IntBits[j+2]);
             HdmiInts->CableDetect[i]= (BOOL) (LvlRaw3 & IntBits[j+2]);
        }
        j+= 5;
    }




    HdmiInts->AudChanMode  = (BOOL) (LvlStatus2 & VRX_BIT_AUDIO_CH_MD_ST);
    HdmiInts->AvMute       = (BOOL) (LvlStatus2 & VRX_BIT_AV_MUTE_ST);
    HdmiInts->InternMute   = (BOOL) (LvlStatus2 & VRX_BIT_INTERNAL_MUTE_ST);
    HdmiInts->CsDataValid  = (BOOL) (LvlStatus2 & VRX_BIT_CS_DATA_VALID_ST);
    HdmiInts->HdmiModeChg  = (BOOL) (LvlStatus2 & VRX_BIT_HDMI_MODE_ST);

    HdmiInts->DeRegenLck   = (BOOL) (LvlStatus3 & VRX_BIT_DE_REGEN_LCK_ST);
    HdmiInts->VsyncLck     = (BOOL) (LvlStatus3 & VRX_BIT_V_LOCKED_ST);

    HdmiInts->NChange      = (BOOL) (EdgStatus2 & VRX_BIT_CHANGE_N_ST);
    HdmiInts->CtsThresh    = (BOOL) (EdgStatus2 & VRX_BIT_CTS_PASS_THRSH_ST);
    HdmiInts->AudFifoOv    = (BOOL) (EdgStatus2 & VRX_BIT_FIFO_OVERFLO_ST);
    HdmiInts->AudFifoUn    = (BOOL) (EdgStatus2 & VRX_BIT_FIFO_UNDERFLO_ST);
    HdmiInts->AudFifoNrOv  = (BOOL) (EdgStatus2 & VRX_BIT_FIFO_NEAR_OVFL_ST);

    HdmiInts->AudModeChg   = (BOOL) (EdgStatus3 & VRX_BIT_AUDIO_MODE_CHNG_ST);
    HdmiInts->AudFifoNrUn  = (BOOL) (EdgStatus3 & VRX_BIT_FIFO_NEAR_UFLO_ST);
    HdmiInts->NewTmds      = (BOOL) (EdgStatus3 & VRX_BIT_NEW_TMDS_FRQ_ST);
    HdmiInts->AudFlatLine  = (BOOL) (EdgStatus3 & VRX_BIT_AUDIO_FLT_LINE_ST);
    HdmiInts->AudSampChg   = (BOOL) (EdgStatus3 & VRX_BIT_NEW_SAMP_RT_ST);
    HdmiInts->AudPrtyErr   = (BOOL) (EdgStatus3 & VRX_BIT_PARITY_ERROR_ST);
    HdmiInts->BadTmdsClk   = (BOOL) (EdgStatus3 & VRX_BIT_VCLK_CHNG_ST);
    HdmiInts->DeepClrChg   = (BOOL) (EdgStatus3 & VRX_BIT_DEEP_COLOR_CHNG_ST);

    /*=======================================================
     * Reset packet detection when mode changes to DVI
     *======================================================*/
    if (HdmiInts->HdmiModeChg && ((LvlRaw2 & VRX_BIT_HDMI_MODE_RAW) == 0))
    {
        ATV_I2CWriteField8 (VRX_HDMI_MAP_ADDR, 0x5A, 0x02, 1, 1);
    }

    {
        HdmiInts->CsValidRaw = (BOOL) (LvlRaw2 & VRX_BIT_CS_DATA_VALID_RAW);
        if (!HdmiInts->CsValidRaw)
        {
            HdmiInts->AudModeChg   = FALSE;
            HdmiInts->AudSampChg   = FALSE;
/*          HdmiInts->AudChanMode  = FALSE;     Valid Aud Ch Mode change event is missed because of this */
        }
    }

    /*============================================
     * Packet detection
     *===========================================*/
    PktValid = (LvlStatus1 | EdgStatus1) & LvlRaw1;
    PktNotRec = LvlStatus1 & (~LvlRaw1);

    HdmiInts->AvIfValid  = PktValid & VRX_BIT_AVI_INFO_ST;
    HdmiInts->AudIfValid = PktValid & VRX_BIT_AUDIO_INFO_ST;
    HdmiInts->SpdValid   = PktValid & VRX_BIT_SPD_INFO_ST;
    HdmiInts->MsValid    = PktValid & VRX_BIT_MS_INFO_ST;
    HdmiInts->VsValid    = PktValid & VRX_BIT_VS_INFO_ST;
    HdmiInts->AcpValid   = PktValid & VRX_BIT_ACP_PCKT_ST;
    HdmiInts->Isrc1Valid = PktValid & VRX_BIT_ISRC1_PCKT_ST;
    HdmiInts->Isrc2Valid = PktValid & VRX_BIT_ISRC2_PCKT_ST;
    HdmiInts->GamutValid = GMD_VALID & VRX_BIT_GAMUT_MDATA_ST;

    HdmiInts->AvIfNotRec = PktNotRec & VRX_BIT_AVI_INFO_ST;
    HdmiInts->AudIfNotRec= PktNotRec & VRX_BIT_AUDIO_INFO_ST;
    HdmiInts->SpdNotRec  = PktNotRec & VRX_BIT_SPD_INFO_ST;
    HdmiInts->MsNotRec   = PktNotRec & VRX_BIT_MS_INFO_ST;
    HdmiInts->VsNotRec   = PktNotRec & VRX_BIT_VS_INFO_ST;
    HdmiInts->AcpNotRec  = PktNotRec & VRX_BIT_ACP_PCKT_ST;
    HdmiInts->Isrc1NotRec= PktNotRec & VRX_BIT_ISRC1_PCKT_ST;
    HdmiInts->Isrc2NotRec= PktNotRec & VRX_BIT_ISRC2_PCKT_ST;
    HdmiInts->GamutNotRec= GMD_NOT_REC & VRX_BIT_GAMUT_MDATA_ST;

    /*=============================================
     * Workaround for deep color INT occasionally
     * not generated after plug-in
     *============================================*/
    if ((HdmiInts->TmdsClock[Port] && HdmiInts->VidPllLock[Port]) &&
        (HdmiInts->VidPllChg[Port] || HdmiInts->AvIfValid))
    {
        HdmiInts->DeepClrChg = TRUE;
    }



}
/*===========================================================================
 * Set and enable Rx internal EDID for all ports
 *
 * Entry:   EdidBuf   = Pointer to 256 byte buffer containing EDID data
 *          SpaOffset = Offset of SPA in the buffer. 0 to ignore spa seetings
 *                      in SpaValues
 *          SpaValues = Array of SPA values for port B, C and D (2 bytes/port)
 *                      First byte contains A.B, second contains C.D
 *                      if SpaOffset ise set to 0, this parameter is ignored
 *          Segment   = 0 or 1
 *
 * Return:  ATVERR_OK
 *
 * Notes:   This function DOES NOT change HPD state for any of the ports
 *
 *==========================================================================*/
 ATV_ERR HAL_RxHdmiSetInternalEdid (UCHAR *EdidBuffer, UINT16 SpaOffset,
                        UCHAR *SpaValues, UCHAR SegNum, BOOL DisableAutoEdid)
{
    HAL_RxPopulateEdidMap (EdidBuffer, 256, SegNum);
    if (SpaOffset && SpaValues)
    {
        HAL_RxSetSpas(SpaOffset, SpaValues);
    }
    HAL_RxToggleInternalEdid();
    return (ATVERR_OK);
}
/*==========================================================================
 * Finds out all the active nad unmasked CEC interrupts. It clears all the
 * active interupts on return.
 *
 * Entry:  CecInts = BOOL array that contains status of all CEC interrupts
 *
 * Notes:  None
 *
 *=========================================================================*/
void HAL_RxCecGetInterrupts (RX_CEC_INTS *CecInts)
{
    UCHAR CecStatus1;

    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_CEC_STATUS1_INT_STATUS,&CecStatus1);
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_CEC_STATUS1_INT_CLEAR, CecStatus1);

    CecInts->TxReady  = (BOOL) (CecStatus1 & VRX_BIT_CEC_TX_READY_ST);
    CecInts->RxReady  = (BOOL) (CecStatus1 & VRX_BIT_CEC_RX_RDY0_ST);
    CecInts->ArbLost  = (BOOL) (CecStatus1 & VRX_BIT_CEC_TX_ARBITRATION_LOST_ST);
    CecInts->Timeout  = (BOOL) (CecStatus1 & VRX_BIT_CEC_TX_RETRY_TIMEOUT_ST);
}

/*==========================================================================
 *
 * Return:  TRUE if Video PLL locked on currently selected port
 *
 *=========================================================================*/
BOOL HAL_RxVPllLocked (RX_HDMI_PORT Port)
{
    BOOL VpllLck = FALSE;

    if (Port == PORT_NONE)
    {
        Port = (RX_HDMI_PORT)VRX_ret_HDMI_PORT_SELECT();
    }
    else
    {
        if (!RX_VALID_PORT(Port) )
        {
        return ATVERR_INV_PARM;
        }
    }
    switch (Port)
    {
        case PORT_A:
            VpllLck = VRX_ret_TMDSPLL_LCK_A_RAW();
            break;
        case PORT_B:
            VpllLck = VRX_ret_TMDSPLL_LCK_B_RAW();
            break;
        default:
            break;
    }
    return (VpllLck);
}

/*==========================================================================
 *
 * Return:  TRUE if Video PLL locked on currently selected port
 *
 *=========================================================================*/
BOOL HAL_RxEncrypted (RX_HDMI_PORT Port)
{
    BOOL Enc = FALSE;

    if (Port == PORT_NONE)
    {
        Port = (RX_HDMI_PORT)VRX_ret_HDMI_PORT_SELECT();
    }
    else
    {
        if (!RX_VALID_PORT(Port) )
        {
        return ATVERR_INV_PARM;
        }
    }
    switch (Port)
    {
        case PORT_A:
            Enc = VRX_ret_HDMI_ENCRPT_A_RAW();
            break;
        case PORT_B:
            Enc = VRX_ret_HDMI_ENCRPT_B_RAW();
            break;
        default:
            return ATVERR_INV_PARM;

    }
    return (Enc);
}

/*==========================================================================
 *
 *
 *
 *=========================================================================*/
BOOL HAL_RxGetAksvUpdate (RX_HDMI_PORT Port)
{
    BOOL RetVal = FALSE;
    if (!RX_VALID_PORT(Port) )
    {
        return ATVERR_INV_PARM;
    }
    switch (Port)
    {
        case PORT_A:
            RetVal = VRX_ret_AKSV_UPDATE_A_RAW();
            break;
        case PORT_B:
            RetVal = VRX_ret_AKSV_UPDATE_B_RAW();
            break;
        default:
             return (ATVERR_INV_PARM);

    }
    return (RetVal);
}

/*============================================================================
 * Set value for the HDCP BCAPS register
 *
 * Entry:   Port    = Port to set Bcaps for
 *          Bcaps   = BCAPS register value
 *
 * Return:  ATVERR_OK = Indicates the function completed successfully
 *
 *
 * Notes:   None
 *
 *===========================================================================*/
ATV_ERR HAL_RxSetHdcpBcaps (RX_HDMI_PORT Port, UCHAR Bcaps)
{
    UCHAR i, j;

    if (RX_VALID_PORT(Port) || (Port == PORT_ALL))
    {
        i = j = RX_PORT_ID(Port);
        if (Port == PORT_ALL)
        {
            i = 0;
            j = RX_NOF_PORTS - 1;
        }

        for (; i<=j; i++)
        {
            VRX_set_HDCP_MAP_SELECT(i);
            VRX_set_BCAPS (Bcaps);
        }
        return (ATVERR_OK);
    }
    return (ATVERR_INV_PARM);
}

/*============================================================================
 * Set value for the HDCP BSTATUS register
 *
 * Entry:   Port    = Port to set Bstatus for
 *          Bstatus = BSTATUS register value
 *
 * Return:  ATVERR_OK = Indicates the function completed successfully
 *
 *
 * Notes:   None
 *
 *===========================================================================*/
ATV_ERR HAL_RxSetHdcpBstatus (RX_HDMI_PORT Port, UINT16 Bstatus)
{
    UCHAR i, j;

    if (RX_VALID_PORT(Port) || (Port == PORT_ALL))
    {
        i = j = RX_PORT_ID(Port);
        if (Port == PORT_ALL)
        {
            i = 0;
            j = RX_NOF_PORTS - 1;
        }

        for (; i<=j; i++)
        {
            VRX_set_HDCP_MAP_SELECT(i);
            VRX_set_BSTATUS(Bstatus);
        }
        return (ATVERR_OK);
    }
    return (ATVERR_INV_PARM);
}

/*============================================================================
 * Enable/Disable HPD on specified port
 *
 * Entry:   Port   = Port to enable/disable its HPD
 *          Enable = TRUE/FALSE to enable/disable HPD
 *
 * Return:  ATVERR_OK = Indicates the function completed successfully
 *
 * Notes:   None
 *
 *===========================================================================*/
ATV_ERR HAL_RxAssertHpd (RX_HDMI_PORT Port, BOOL Enable)
{
    UCHAR En;
    ATV_ERR RetVal = ATVERR_OK;

    En = Enable? 1: 0;

    VRX_set_HPD_MANUAL(1);

    switch (Port)
    {
        case PORT_A:
            VRX_set_HPA_MAN_VALUE_A(En);
            break;
        case PORT_B:
            VRX_set_HPA_MAN_VALUE_B(En);
            break;
        case PORT_ALL:
            VRX_set_HPA_MAN_VALUE_A(En);
            VRX_set_HPA_MAN_VALUE_B(En);
            break;
        default:
            RetVal = ATVERR_INV_PARM;
            break;
    }
    return (RetVal);
}

/*============================================================================
 * Enable/Disable HPD on specified port
 *
 * Entry:   Port   = Port to enable/disable its HPD
 *          Enable = TRUE/FALSE to enable/disable HPD
 *
 * Return:  ATVERR_OK = Indicates the function completed successfully
 *
 * Notes:   None
 *
 *===========================================================================*/
ATV_ERR HAL_RxIsHpdOn (RX_HDMI_PORT Port, BOOL *HpdOn)
{
    BOOL IsOn;
    ATV_ERR RetVal = ATVERR_OK;
    if (!RX_VALID_PORT(Port) )
    {
        return ATVERR_INV_PARM;
    }
    switch (Port)
    {
        case PORT_A:
            IsOn = VRX_ret_HPA_STATUS_PORT_A();
            break;
        case PORT_B:
            IsOn = VRX_ret_HPA_STATUS_PORT_B();
            break;
        default:
            IsOn = FALSE;
            RetVal = ATVERR_INV_PARM;
            break;
    }
    if (HpdOn)
    {
        *HpdOn = IsOn;
    }
    if (RetVal == ATVERR_OK)
    {
        RetVal = IsOn? ATVERR_TRUE: ATVERR_FALSE;
    }
    return (RetVal);
}

/*===========================================================================
 * Note: TO be Updated for ES2!!
 *
 *==========================================================================*/
void HAL_RxSetBksvListReadyBit (BOOL On)
{
#if ((ES1_7612)|| (ES1_7611) )
    UCHAR i=0, Bcaps=0;
#endif
    VRX_set_KSV_LIST_READY(On? 1: 0);
    if (!On)
    {
#if ((ES1_7612)|| (ES1_7611) )
        /*=========================================
         * Turn BCAPS ready bit off for all ports
         *========================================*/
        for (i=0; i<RX_NOF_PORTS; i++)
        {
            ADIAPI_RxHdmiGetHdcpBcaps((RX_HDMI_PORT)i, &Bcaps);
            Bcaps &= ~0x20;
            HAL_RxSetHdcpBcaps ((RX_HDMI_PORT)i, Bcaps);
        }
#else
        VRX_set_KSV_LIST_READY_CLR_A(0x1);
        VRX_set_KSV_LIST_READY_CLR_B(0x1);
#endif

    }
}

/*===========================================================================
 * Check if RX interrupt is pending
 *
 * Entry:   IntId = 1 or 2 for RX Interrupt1 or Interrupt2 lines
 *
 * Return:  TRUE  if interrupt is pending
 *          FALSE if no interrupt is pending
 *
 *==========================================================================*/
BOOL HAL_RxIntPending (UCHAR IntId)
{
    if (IntId == 1)
    {
        return (VRX_ret_INTRQ_RAW());
    }
    else
    {
        return (VRX_ret_INTRQ2_RAW());
    }
}
/*============================================================================
 * Enable/Disable Mute conditions applicable to Internal Mute Signal
 *
 * Entry:   RX_AUD_MT_MSK *MuteMaskList, List of mute conditions to be changed
 *              BOOL    Enable- whether or not mute conditions should be enabled or disabled.
 * Return:  ATVERR_OK
 *          ATVERR_NOT_AVAILABLE =  This API is not available
 *
 * Notes:   None
 *
 *===========================================================================*/
ATV_ERR HAL_RxHdmiSetInternalMuteMsk (RX_AUD_MT_MSK *MuteMaskList,  UCHAR ListSize, BOOL Enable)
{
        UINT8 i, j ;
        UINT8 MaskVal[NOF_MT_MASK_REGS];


    /*=======================================
     * Get interrupt mask values
     *======================================*/
    for (i=0;  RxHdmiMtMaskRegs[i]; i++)
    {
        HAL_I2CReadByte(VRX_HDMI_MAP_ADDR, RxHdmiMtMaskRegs[i], &(MaskVal[i] ) );
    }

    /*====================================
     * Enable/disable required events
     *===================================*/
    for (i=0; i<ListSize; i++)
    {
        for (j=0;RxMuteMskInfo[j].MtMask != 0xff; j++)
        {
            if (RxMuteMskInfo[j].MtMask == MuteMaskList[i])
            {
                if (Enable)
                {
                    MaskVal[RxMuteMskInfo[j].Reg] |=   RxMuteMskInfo[j].Mask;
                }
                else
                {
                  MaskVal[RxMuteMskInfo[j].Reg]&= ~(RxMuteMskInfo[j].Mask);
                }
           }
        }
     }

    /*=======================================
     * Update Mute  masks
     *======================================*/
    for (i=0;  RxHdmiMtMaskRegs[i];  i++)
    {
        HAL_I2CWriteByte(VRX_HDMI_MAP_ADDR,  RxHdmiMtMaskRegs[i], MaskVal[i]);
    }


    return(ATVERR_OK);
}
/*====================================================================================
 * Disable HDCP on HDMI Rx. HDMI Rx Looks like Non-HDCP Receiver to Upstream Device
 *
 * Entry:
 *          Port : Must be PORT_ALL,
 *
 *          Enable   =  True  HDMI Rx Acts As HDCP Receiver
 *                      False HDMI Rx As Non-HDCP Receiver
 * Return:
 *          ATVERR_OK
 *          ATVERR_INV_PARM
 * Notes:
 *
 *======================================================================================*/
ATV_ERR HAL_RxHdmiEnHdcp(RX_HDMI_PORT Port, BOOL Enable)

{
  if(Port != PORT_ALL )
  {
      return (ATVERR_INV_PARM);
  }
  if(Enable)
  {
     ADIAPI_RxHdmiSetHdcpI2CBase(0x74);
  }
  else
  {
     ADIAPI_RxHdmiSetHdcpI2CBase(0x76);
  }
  return(ATVERR_OK);
}
#if FAST_SWITCH_DEVICE
/*============================================================================
 *
 *
 * Entry:
 *
 * Return:
 *
 *===========================================================================*/
ATV_ERR HAL_RxHdmiReqBgMeas (RX_HDMI_PORT Port)
{
    if(RX_VALID_PORT(Port))
    {
        VRX_set_BG_MEAS_PORT_SEL(RX_PORT_ID(Port));
        VRX_set_BG_MEAS_REQ(1);
        return (ATVERR_OK);
    }
    return (ATVERR_INV_PARM);
}

/*============================================================================
 *
 *
 * Entry:
 *
 * Return:
 *
 *===========================================================================*/
ATV_ERR HAL_RxHdmiGetBgTmdsFreq (RX_HDMI_PORT Port, UINT16 * TmdsFreqMHz, UCHAR * TmdsFreqFrac)
{
    BOOL TmdsDetected;
    UINT32 Temp;
    UINT32 i=0;
    *TmdsFreqMHz =0x0;
    ADIAPI_RxHdmiTmdsDetected (Port, &TmdsDetected);
    if (TmdsDetected)
    {
        VRX_set_BG_MEAS_DONE_CLR(0x1);
        if((HAL_RxHdmiReqBgMeas(Port) == (ATVERR_OK)))
        {
          while (!VRX_ret_BG_MEAS_DONE_RAW())
          {
              HAL_DelayMs(1);
              i++;
              if(i>500)
              {
                return (ATVERR_FAILED);
              }
           }
           Temp = VRX_ret_BG_TMDSFREQ();
           *TmdsFreqMHz = (UINT16)(Temp);
           if(TmdsFreqFrac != NULL)
           {
             *TmdsFreqFrac = VRX_ret_BG_TMDSFREQ_FRAC() << TMDSFRAQ_SHIFT;
           }
           return (ATVERR_OK);
         }
     }
    return (ATVERR_FAILED);

}

/*============================================================================
 *
 *
 * Entry:
 *
 * Return:
 *
 *===========================================================================*/
ATV_ERR HAL_RxHdmiGetBgHsyncInfo (RX_HDMI_PORT Port, RX_HDMI_HSYNC * HsyncInfo)
{
    UINT16 HTotal, LineWid, PRValue;


    if((HAL_RxHdmiReqBgMeas(Port) == (ATVERR_OK)))
    {
        if (VRX_ret_BG_MEAS_DONE_RAW())
        {
            VRX_get_BG_TOTAL_LINE_WIDTH (&HTotal);
            VRX_get_BG_LINE_WIDTH(&LineWid);
            HsyncInfo->FrontPorch = 0;
            HsyncInfo->SyncPulse = 0;
            HsyncInfo->BackPorch = 0;

            PRValue = (UINT16)VRX_ret_BG_PIX_REP() + 1;

            HsyncInfo->LinesTotal = (HTotal * PRValue);
            HsyncInfo->LinesActive = (LineWid * PRValue);
            return (ATVERR_OK);
        }
    }
    return (ATVERR_FAILED);
}

/*============================================================================
 *
 *
 * Entry:
 *
 * Return:
 *
 *===========================================================================*/
ATV_ERR HAL_RxHdmiGetBgVsyncInfo (RX_HDMI_PORT Port, RX_HDMI_VSYNC * VsyncInfo)
{
    UINT16 VTotal0, VTotal1;

    if((HAL_RxHdmiReqBgMeas(Port) == (ATVERR_OK)))
    {
        if (VRX_ret_BG_MEAS_DONE_RAW())
        {
            VsyncInfo->FrontPorch = 0;
            VsyncInfo->SyncPulse = 0;
            VsyncInfo->BackPorch = 0;
            VRX_get_BG_TOTAL_FIELD_HEIGHT(&VTotal0);
            VRX_get_BG_FIELD_HEIGHT(&(VsyncInfo->LinesActive));
            VsyncInfo->Interlaced = ((VRX_is_BG_HDMI_INTERLACED_true())? TRUE : FALSE);
            if (VsyncInfo->Interlaced)
            {
                VsyncInfo->F1FrontPorch = 0;
                VsyncInfo->F1SyncPulse = 0;
                VsyncInfo->F1BackPorch = 0;
                VRX_get_BG_TOTAL_FIELD_HEIGHT(&(VsyncInfo->F1LinesActive));
                VRX_get_BG_TOTAL_FIELD_HEIGHT(&VTotal1);
                VsyncInfo->F1LinesTotal = (VTotal1 >> 1);
            }
            VsyncInfo->LinesTotal = (VTotal0 >> 1);
            return (ATVERR_OK);
        }
    }
    return (ATVERR_FAILED);
}

/*============================================================================
 *
 *
 * Entry:
 *
 * Return:
 *
 *===========================================================================*/
ATV_ERR HAL_RxHdmiGetBgDeepColorMode (RX_HDMI_PORT Port, RX_COLOR_DEPTH * DeepColorMode)
{
    UCHAR ColDepth;
    RX_COLOR_DEPTH CdTable[] = {RX_CD_24, RX_CD_30, RX_CD_36, RX_CD_48};


    if((HAL_RxHdmiReqBgMeas(Port) == (ATVERR_OK)))
    {
        if(VRX_ret_BG_MEAS_DONE_RAW())
        {
            VRX_get_BG_DEEP_COLOR_MODE(&ColDepth);
            if (ColDepth < 4)
            {
                *DeepColorMode = CdTable[ColDepth];
            }
            else
            {
                *DeepColorMode = RX_CD_24;
            }
            return (ATVERR_OK);
        }
    }
    return (ATVERR_FAILED);
}

/*============================================================================
 *
 *
 * Entry:
 *
 * Return:
 *
 *===========================================================================*/
ATV_ERR HAL_RxHdmiGetBgPixelRepeat (RX_HDMI_PORT Port, UCHAR * RepeatFactor)
{
    UCHAR RetVal;

    if((HAL_RxHdmiReqBgMeas(Port) == (ATVERR_OK)))
    {
        VRX_get_BG_PIX_REP(&RetVal);
        *RepeatFactor = RetVal + 1;
        return (ATVERR_OK);
    }
    return (ATVERR_FAILED);
}





/*============================================================================
 * Enable/Disable fast switching on specified port
 *
 * Entry:   Port   = Port to set Bstatus for
 *          Enable = TRUE/FALSE to enable/disable fast switching
 *
 * Return:  ATVERR_OK = Indicates the function completed successfully
 *
 *
 * Notes:   None
 *
 *===========================================================================*/
ATV_ERR HAL_RxSetFastSwitchPort (RX_HDMI_PORT Port, BOOL Enable)
{
    UCHAR En;
    ATV_ERR RetVal = ATVERR_OK;

    En = Enable? 1: 0;
    if (RX_VALID_PORT(Port) || (Port == PORT_ALL))
    {
            switch (Port)
        {
            case PORT_A:
                VRX_set_EN_BG_PORT_A(En);
                break;
            case PORT_B:
                VRX_set_EN_BG_PORT_B(En);
                break;

            case PORT_ALL:
                VRX_set_EN_BG_PORT_A(En);
                VRX_set_EN_BG_PORT_B(En);
                break;
            default:
                RetVal = ATVERR_INV_PARM;
                break;
        }

    }
    else
    {
         RetVal = ATVERR_INV_PARM;
    }
    return (RetVal);
}


#endif
#endif
