/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#ifndef _REP_CONFIG_H_
#define _REP_CONFIG_H_
#include "atv_types.h"

/*==========================================
 * RX related configurations
 *=========================================*/
#define ENABLE_EXTERNAL_EDID            0 
#if (MULTI_RX_NUM)
#define RX_NUM_OF_DEVICES               2
#define RX_CEC_ENABLE_ID                1
#else
#define RX_NUM_OF_DEVICES               1
#define RX_CEC_ENABLE_ID                0
#endif

#if (RX_DEVICE == 7623) || (RX_DEVICE == 7622) || (RX_DEVICE == 76221)
#define RX_I2C_IO_MAP_ADDR              0xB2
#define RX2_I2C_IO_MAP_ADDR             0xB0
#elif (RX_DEVICE == 7612) || (RX_DEVICE == 7611) || (RX_DEVICE == 7619)
#define RX_I2C_IO_MAP_ADDR              0x98
#define RX2_I2C_IO_MAP_ADDR             0x98
#elif (RX_DEVICE == 7630)
#define RX_I2C_IO_MAP_ADDR              0xB0
#else
#define RX_I2C_IO_MAP_ADDR              0x40
#define RX2_I2C_IO_MAP_ADDR             0xB4
#endif
#define RX_I2C_SDP_VDP_MAP_ADDR         0x22
#define RX_I2C_AFE_DPLL_MAP_ADDR        0x30
#define RX_I2C_ESDP_MAP_ADDR            0x34    /* Def 0x70 */
#define RX_I2C_SDP_IO_MAP_ADDR          0x42
#define RX_I2C_CP_MAP_ADDR              0x44
#define RX_I2C_VDP_MAP_ADDR             0x48
#define RX_I2C_TEST_MAP3_ADDR           0x52
#define RX_I2C_TEST_MAP1_ADDR           0x60
#define RX_I2C_TEST_MAP2_ADDR           0x62
#define RX_I2C_REPEATER_MAP_ADDR        0x64
#define RX_I2C_HDMI_MAP_ADDR            0x68
#define RX_I2C_EDID_MAP_ADDR            0x6C
#define RX_I2C_DPP_MAP_ADDR             0x74    /* Def 0x78 */
#define RX_I2C_INFOFRAME_MAP_ADDR       0x76    /* 0x7C on ATV_MB seems to cause readback of all 0x00, occasionally. Happens more often when no Sink is connected */
#define RX_I2C_CEC_MAP_ADDR             0x80
#define RX_I2C_SDP_MAP_ADDR             0x82
#define RX_I2C_AVLINK_MAP_ADDR          0x84
#define RX_I2C_OSD_MAP_ADDR             0x88
#define RX_I2C_AUDIO_CODEC_MAP_ADDR     0x5C
#define RX_I2C_XMEM_MAP_ADDR            0xA8
#define RX_I2C_VFE_MAP_ADDR             0xA0
#define RX_I2C_VSP_MAP_ADDR             0x00
#define RX_I2C_VPP_MAP_ADDR             0x00
#define RX_I2C_OLDI_MAP_ADDR            0x00

#define RX2_I2C_SDP_VDP_MAP_ADDR        0xC0
#define RX2_I2C_AFE_DPLL_MAP_ADDR       0xC2
#define RX2_I2C_ESDP_MAP_ADDR           0xC4    /* Def 0x70 */
#define RX2_I2C_SDP_IO_MAP_ADDR         0xC6
#define RX2_I2C_CP_MAP_ADDR             0xC8
#define RX2_I2C_VDP_MAP_ADDR            0xCA
#define RX2_I2C_TEST_MAP3_ADDR          0xCC
#define RX2_I2C_TEST_MAP1_ADDR          0xCE
#define RX2_I2C_TEST_MAP2_ADDR          0xD0
#define RX2_I2C_REPEATER_MAP_ADDR       0xD2
#define RX2_I2C_HDMI_MAP_ADDR           0xD4
#define RX2_I2C_EDID_MAP_ADDR           0xD6
#define RX2_I2C_DPP_MAP_ADDR            0xD8    /* Def 0x78 */
#define RX2_I2C_INFOFRAME_MAP_ADDR      0xDA
#define RX2_I2C_CEC_MAP_ADDR            0xDC
#define RX2_I2C_SDP_MAP_ADDR            0xDE
#define RX2_I2C_AVLINK_MAP_ADDR         0xE0
#define RX2_I2C_OSD_MAP_ADDR            0xE2
#define RX2_I2C_VFE_MAP_ADDR            RX_I2C_VFE_MAP_ADDR
#define RX2_I2C_AUDIO_CODEC_MAP_ADDR    RX_I2C_AUDIO_CODEC_MAP_ADDR
#define RX2_I2C_XMEM_GAMMA_MAP_ADDR     RX_I2C_XMEM_MAP_ADDR
#define	RX2_I2C_VSP_MAP_ADDR            0
#define RX2_I2C_VPP_MAP_ADDR            0
#define RX2_I2C_OLDI_MAP_ADDR           0
#define RX2_I2C_OLDI_MAP_ADDR           0



#define RX_INCLUDE_HDMI                 1   /* device has HDMI interface */

#if (RX_DEVICE != 7850)
#define RX_INCLUDE_CEC                  1
#else
#define RX_INCLUDE_CEC                  0
#endif

#if (RX_DEVICE == 7850)
#define RX_INCLUDE_VBI                  1
#else
#define RX_INCLUDE_VBI                  0
#endif

#if (RX_DEVICE == 7623)
#define RX_INCLUDE_OSD                  1
#else
#define RX_INCLUDE_OSD                  0
#endif

#define RX_AUDIO_UNMUTE_DELAY           RX_AUD_UNMUTE_AFTER_250MS
                            /* This value define the audio unmute delay for */
                            /* HDMI RX. Possible values depend on the RX    */
                            /* device in use. Only 250 ms and 1000 ms are   */
                            /* defined for all RX devices. Other values can */
                            /* range from 10 ms to 2 seconds                */

/*==========================================
 * TX related configurations
 *=========================================*/
#if ((TX_DEVICE == 8002) && (ES_X_8002 > 1)) /* 8002 has TX1 and TX2 separately from ES2 and later version */
#define TX_NUM_OF_DEVICES               2
#elif( TX_DEVICE == 7630)
#define TX_NUM_OF_DEVICES               2
#else
#define TX_NUM_OF_DEVICES               1
#endif

#if (TX_DEVICE == 7623) || (TX_DEVICE == 7622) || (TX_DEVICE == 76221)
#define TX_I2C_MAIN_MAP_ADDR            0xBA
#define TX2_I2C_MAIN_MAP_ADDR           0xB8
#elif (TX_DEVICE == 7850)
#define TX_I2C_MAIN_MAP_ADDR            0xB8
#define TX2_I2C_MAIN_MAP_ADDR           0xBA    /* Not used */
#elif (TX_DEVICE == 8002)
#define TX_I2C_MAIN_MAP_ADDR            0xEC
#define TX2_I2C_MAIN_MAP_ADDR           0xF4
#elif (TX_DEVICE == 7630)
#define TX_I2C_MAIN_MAP_ADDR            0xBA
#define TX2_I2C_MAIN_MAP_ADDR           0xF4
#elif ( (TX_DEVICE == 7511) && (MULTI_RX_NUM))
#define TX_I2C_MAIN_MAP_ADDR            0x7A
#define TX2_I2C_MAIN_MAP_ADDR           0x72
#else
#define TX_I2C_MAIN_MAP_ADDR            0x72
#define TX2_I2C_MAIN_MAP_ADDR           0x7A
#endif

#if (TX_DEVICE == 8002)
#define TX_I2C_PKT_MEM_MAP_ADDR         0xF2
#define TX_I2C_CEC_MAP_ADDR             0xF0
#define TX_I2C_EDID_MAP_ADDR            0xEE
#define TX2_I2C_PKT_MEM_MAP_ADDR        0xFA
#define TX2_I2C_CEC_MAP_ADDR            0xF8
#define TX2_I2C_EDID_MAP_ADDR           0xF6
#else
#define TX_I2C_PKT_MEM_MAP_ADDR         0x70
#define TX_I2C_CEC_MAP_ADDR             0x78
#if ( (TX_DEVICE == 7511) && (TX_NUM_OF_DEVICES > 1))
#define TX_I2C_EDID_MAP_ADDR            0xFE
#else
#define TX_I2C_EDID_MAP_ADDR            0x7E
#endif
#define TX_I2C_TEST1_MAP_ADDR           0x92
#if ( RX_NUM_OF_DEVICES > 1 )
#define TX2_I2C_PKT_MEM_MAP_ADDR        0x7C
#else
#define TX2_I2C_PKT_MEM_MAP_ADDR        0x76
#endif
#define TX2_I2C_CEC_MAP_ADDR            0x82
#define TX2_I2C_EDID_MAP_ADDR           0x86
#define TX2_I2C_TEST1_MAP_ADDR          0x94     /* please make sure there is on confliction with other address when you modify the value */
#endif



#define TX_INCLUDE_CEC                  0
#define TX_INCLUDE_OSD                  0
#define TX_EDID_RETRY_COUNT             8

/*==========================================
 * System wide configurations
 *=========================================*/
/* Maximum DS_DEVICE_COUNT limited by RX Fifo capacity*/
#if ( RX_DEVICE == 7619) || ( RX_DEVICE == 7612) || ( RX_DEVICE == 7611) || ( RX_DEVICE == 7630)
#define REP_SUPPORTED_DS_DEVICE_COUNT   127
#elif (RX_DEVICE == 7850) || (RX_DEVICE == 7844)|| (RX_DEVICE == 7842)
#define REP_SUPPORTED_DS_DEVICE_COUNT   24
#else
#define REP_SUPPORTED_DS_DEVICE_COUNT   12
#endif

#define REP_SUPPORTED_EDID_SEGMENTS     2
                            /* Maximum is 2, limited by RX capacity */

#define REP_INCLUDE_CEC_SWITCH          (TX_INCLUDE_CEC || RX_INCLUDE_CEC)
                            /* This macro enable the inclusion of a CEC      */
                            /* switch application                            */

#define REP_INCLUDE_OSD                 (TX_INCLUDE_OSD || RX_INCLUDE_OSD)
#if (RX_DEVICE == 7623) || (RX_DEVICE == 7622) || (RX_DEVICE == 76221)
#define OSD_IN_EXTERNAL_FLASH           0       /* Location to store the OSD */
#define OSD_FOR_PREVIEW_ONLY            0       /* To display the osd only   */
#define OSD_CONTROLED_BY_SPI            1       /* uP use spi to control osd */
#endif
                            /* This macro enable the inclusion of a OSD      */
                            /* application                                   */

#define REP_AKSV_TO_TX_ENC_OFF_DELAY    7000
                            /* This defines the min time following an AKSV   */
                            /* interrupt, after which if RX is still not     */
                            /* encrypted, TX will turn its encryption off    */
                            /* when encryption mode is FOLLOW_INPUT          */

/*#define REP_INP_STABLE_TO_UNMUTE_DELAY  300*/
                            /* This defines delay in ms from the time input  */
                            /* audio/video signals stabilize to the time     */
                            /* output audio/video are un-muted following any */
                            /* disruption in nput signal                     */
                            /* Output audio and/or video are automatically   */
                            /* muted following any input audio/video signal  */
                            /* disruption if the system mute mode is set to  */
                            /* MUTE_AUTO.                                    */
                            /* (@@@@ RX started delay to unmute) message     */
                            /* will apear once this delay is started         */
                            /* (@@@@ RX unmute delay expired) message will   */
                            /* apear once this delay is expired              */

#define TMDS_MUTE_TIME                  235 /*1400*/
                            /* Defines the minimum time output TMDS will be  */
                            /* turned off following any disruption in the    */
                            /* input signal to the TX device                 */
                            /* (TMDS off delay passed) message will appear   */
                            /* once this delay is expired                    */
                            /* ADIAPI_RepTxSetMutingTime() can be used to    */
                            /* change this default setting                   */

#define BLACKOUT_MUTE_TIME              265 /*1600*/
                            /* Defines the minimum time output video will be */
                            /* blacked-out after output TMDS is turned on    */
                            /* (Blackout delay passed) message will appear   */
                            /* once this delay is expired                    */
                            /* ADIAPI_RepTxSetMutingTime() can be used to    */
                            /* change this default setting                   */

#define FS_TMDS_MUTE_TIME               100 /*1200*/
                            /* Will be used instead of TMDS_MUTE_TIME when   */
                            /* fast switching is enabled                     */
                            /* (TMDS off delay passed) message will appear   */
                            /* once this delay is expired                    */
                            /* ADIAPI_RepTxSetMutingTime() can be used to    */
                            /* change this default setting                   */

#define FS_BLACKOUT_MUTE_TIME           100 /*600*/
                            /* Will be used instead of BLACKOUT_MUTE_TIME    */
                            /* when fast switching is enabled                */
                            /* (Blackout delay passed) message will appear   */
                            /* once this delay is expired                    */
                            /* ADIAPI_RepTxSetMutingTime() can be used to    */
                            /* change this default setting                   */

#define HDCP_MUTE_TIME                  20
                            /* Defines the minimum time output TMDS will be  */
                            /* turned off before Tx HDCP becomes enabled     */
                            /* Note : large vlaues may cause HDCP CT failure */
                            /* ADIAPI_RepTxSetMutingTime() can be used to    */
                            /* change this default setting                   */

#define REP_HDCP_SUPPORT_FAST_REAUTH    0
                            /* Set to 1 if HDCP is required to support fast  */
                            /* reauthentication                              */

#define REP_HDCP_SUPPORT_EESS           0
                            /* Set to 1 if HDCP support enhanced encryption  */
                            /* status signaling                              */


#define REP_MODE_SWITCH_DELAY           500
                            /* This value defines the delay from the time   */
                            /* a mode change is detected to the time it is  */
                            /* reported to the application. It is used to   */
                            /* make sure the new mode is stable before it   */
                            /* is processed                                 */

#define APP_SEND_PACKET_TO_RX(PktPtr, PktSize)      ADIAPI_RepRxTask(ADI_TASK_EVENT_PACKET, PktPtr)
                            /* This macro is used to send a data packet     */
                            /* from TX to RX side when the RX and TX are    */
                            /* running on two different micro-controllers.  */
                            /* For single processor systems, use as above   */

#define APP_SEND_PACKET_TO_TX(PktPtr, PktSize)      ADIAPI_RepTxTask(ADI_TASK_EVENT_PACKET, PktPtr)
                            /* This macro is used to send a data packet     */
                            /* from RX to TX side when the RX and TX are    */
                            /* running on two different micro-controllers.  */
                            /* For single processor systems, use as above   */

#define REP_HOUSEKEEPING_DELAY      100
                            /* This value defines the delay (in ms) between */
                            /* any two consecutive calls to the repeater    */
                            /* housekeeping function. This function provide */
                            /* periodic check on hardware status to make    */
                            /* sure everything is in order. Smaller delay   */
                            /* result in better response but generate much  */
                            /* more i2c access. Recommended 100-200 ms      */

/*==========================================
 * Pixel bus configuration
 *=========================================*/
#define REP_RX_PIX_BUS_FORMAT_444       RX_BUS_OP_MODE_444SDR
#define REP_RX_PIX_BUS_CH_ORDER_444     RX_BUS_OP_CH_RGB
#define REP_RX_PIX_BUS_WIDTH_444        RX_BUS_BITS_12
#define REP_RX_PIX_BUS_MODE_444         RX_BUS_MODE0
#define REP_RX_PIX_BUS_CRCB_SWAP_444    FALSE
#define REP_RX_PIX_BUS_REORDER_444      FALSE
                            /* The above values define the configuration of */
                            /* the RX pixel bus output when the input video */
                            /* is RGB or YUV 4:4:4                          */

#define REP_RX_PIX_BUS_FORMAT_422       RX_BUS_OP_MODE_422SDR
#define REP_RX_PIX_BUS_CH_ORDER_422     RX_BUS_OP_CH_RGB
#define REP_RX_PIX_BUS_WIDTH_422        RX_BUS_BITS_12
#define REP_RX_PIX_BUS_MODE_422         RX_BUS_MODE0
#define REP_RX_PIX_BUS_CRCB_SWAP_422    FALSE
#define REP_RX_PIX_BUS_REORDER_422      FALSE
                            /* The above values define the configuration of */
                            /* the RX pixel bus output when the input video */
                            /* is YUV 4:2:2                                 */

#if (TX_DEVICE == 7520)
#undef  REP_RX_PIX_BUS_CH_ORDER_422
#undef  REP_RX_PIX_BUS_WIDTH_422
#define REP_RX_PIX_BUS_CH_ORDER_422     RX_BUS_OP_CH_BGR
#define REP_RX_PIX_BUS_WIDTH_422        RX_BUS_BITS_8
#endif

#if (TX_DEVICE == 8002)
#undef REP_RX_PIX_BUS_CH_ORDER_422
#define REP_RX_PIX_BUS_CH_ORDER_422     RX_BUS_OP_CH_BGR
#endif

#define REP_TX_PIX_BUS_WIDTH_444        12
#define REP_TX_PIX_BUS_FORMAT_444       SDR_444_SEP_SYNC
#define REP_TX_PIX_BUS_STYLE_444        1
#define REP_TX_PIX_BUS_ALIGN_444        ALIGN_RIGHT
#define REP_TX_PIX_BUS_DDR_EDGE_444     FALSE
#define REP_TX_PIX_BUS_BIT_SWAP_444     FALSE
                            /* The above values define the configuration of */
                            /* the TX pixel bus input for RGB/YUV 4:4:4     */

#define REP_TX_PIX_BUS_WIDTH_422        12
#define REP_TX_PIX_BUS_FORMAT_422       SDR_422_SEP_SYNC
#define REP_TX_PIX_BUS_STYLE_422        3
#define REP_TX_PIX_BUS_ALIGN_422        ALIGN_RIGHT
#define REP_TX_PIX_BUS_DDR_EDGE_422     FALSE
#define REP_TX_PIX_BUS_BIT_SWAP_422     FALSE
                            /* The above values define the configuration of */
                            /* the TX pixel bus input for YUV 4:2:2         */

#if (TX_DEVICE == 7520)
#undef  REP_TX_PIX_BUS_WIDTH_422
#undef  REP_TX_PIX_BUS_STYLE_422
#undef  REP_TX_PIX_BUS_ALIGN_422
#define REP_TX_PIX_BUS_WIDTH_422        8
#define REP_TX_PIX_BUS_STYLE_422        1
#define REP_TX_PIX_BUS_ALIGN_422        ALIGN_LEFT
#endif

#if ( RX_NUM_OF_DEVICES > 1 )
#define RX_SPLITTER                     1
#else
#define RX_SPLITTER                     0
#endif

#endif /* _REP_CONFIG_H_ */
