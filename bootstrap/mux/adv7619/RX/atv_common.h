/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#ifndef _ATV_COMMON_H_
#define _ATV_COMMON_H_

#include "atv_types.h"

/*===========================================================
 * Global defines
 *==========================================================*/
EXTERNAL UINT32 AtvDummy;
EXTERNAL CONSTANT UINT16 VicInfo[];

/*===========================================================
 * Defines and Macros
 *==========================================================*/
#define HDMI_VER_20                     20    /* HDMI Specification Version 2.0  */
#define HDMI_VER_14                     14    /* HDMI Specification Version 1.4B */ 
#define CEA_861F_VIC                    107    /* CEA_861_F supports 1-107 VICs  */ 
#define CEA_861D_VIC                     64    /* CEA_861_D supports 1-64  VICs  */ 
#define MAX_VIC_VALUE                   CEA_861F_VIC
#define NUM_OF_VICS                     (MAX_VIC_VALUE+1)

#define AV_IF_PKT_SIZE                  16
#define AUD_IF_PKT_SIZE                 13
#define ACP_PKT_SIZE                    31
#define SPD_PKT_SIZE                    31
#define ISRC1_PKT_SIZE                  31
#define ISRC2_PKT_SIZE                  31
#define GMD_PKT_SIZE                    31
#define VS_PKT_SIZE                     31
#define MS_PKT_SIZE                     17
#define CHAN_STATUS_SIZE                5
#define MAX_HDMI_PACKET_SIZE            32

#define PKT_AV_INFO_FRAME               0x0001
#define PKT_AUDIO_INFO_FRAME            0x0002
#define PKT_ACP_PACKET                  0x0004
#define PKT_SPD_PACKET                  0x0008
#define PKT_ISRC1_PACKET                0x0010
#define PKT_ISRC2_PACKET                0x0020
#define PKT_GMD_PACKET                  0x0040
#define PKT_GC_PACKET                   0x0080
#define PKT_MPEG_PACKET                 0x0100
#define PKT_VS_PACKET                   0x0200
#define PKT_AUDIO_CHANNEL_STATUS        0x0400
#define PKT_AUDIO_SAMPLE_PACKET         0x0800
#define PKT_ACR_PACKET                  0x1000
#define PKT_EXT_AV_INFO_FRAME           0x2000
#define PKT_UNKNOWN_PACKET              0x8000
#define PKT_ALL_PACKETS                 0xffff

#define ATV_DUMMY_SET(a)                AtvDummy=(UINT32)a
#define HDMI_LCC_REG                    0x000C03
/*========================================
 * AVI info frame macros
 *=======================================*/
#define SET_AVIF_BYTE1(Y,A,B,S)         ((Y<<5) | (A<<4) | (B<<2) | S)
#define SET_AVIF_BYTE2(C,M,R)           ((C<<6) | (M<<4) | R)
#define SET_AVIF_BYTE3(ITC,EC,Q,SC)     ((ITC<<7)| (EC<<4) | (Q<<2) | SC)
#define SET_AVIF_BYTE4(VIC)             (VIC)
#define SET_AVIF_BYTE5(PR)              (PR)

#define SET_AVIF_Y(Ptr,Y)               (Ptr[3]=(Ptr[3]&(~0xE0))|(Y<<5))
#define SET_AVIF_PR(Ptr,PR)             (Ptr[7]=(Ptr[7]&(~0x0f))|PR)
#define SET_AVIF_VIC(Ptr,VIC)           (Ptr[6]=VIC)
#define SET_AVIF_C(Ptr,C)               (Ptr[4]=(Ptr[4]&(~0xc0))|(C<<6))
#define SET_AVIF_M(Ptr,M)               (Ptr[4]=(Ptr[4]&(~0x30))|(M<<4))
#define SET_AVIF_R(Ptr,R)               (Ptr[4]=(Ptr[4]&(~0x0f))|R)
#define SET_AVIF_Q(Ptr,Q)               (Ptr[5]=(Ptr[5]&(~0x0c))|(Q<<2))
#define SET_AVIF_A(Ptr,A)               (Ptr[3]=(Ptr[3]&(~0x10))|(A<<4))

#define GET_AVIF_Y(Ptr)                 ((Ptr[3] & 0xE0) >> 5)
#define GET_AVIF_A(Ptr)                 ((Ptr[3] & 0x10) >> 4)
#define GET_AVIF_B(Ptr)                 ((Ptr[3] & 0x0c) >> 2)
#define GET_AVIF_S(Ptr)                 ( Ptr[3] & 0x03)
#define GET_AVIF_C(Ptr)                 ((Ptr[4] & 0xc0) >> 6)
#define GET_AVIF_M(Ptr)                 ((Ptr[4] & 0x30) >> 4)
#define GET_AVIF_R(Ptr)                 ( Ptr[4] & 0x0f)
#define GET_AVIF_ITC(Ptr)               ((Ptr[5] & 0x80) >> 7)
#define GET_AVIF_EC(Ptr)                ((Ptr[5] & 0x70) >> 4)
#define GET_AVIF_Q(Ptr)                 ((Ptr[5] & 0x0c) >> 2)
#define GET_AVIF_SC(Ptr)                ( Ptr[5] & 0x03)
#define GET_AVIF_VIC(Ptr)               ( Ptr[6] & 0xff)
#define GET_AVIF_PR(Ptr)                ( Ptr[7] & 0x0f)
#define GET_AVIF_YQ(Ptr)                ((Ptr[7] & 0xC0) >> 6)
/*========================================
 * Vendor Specific info frame macros
 *=======================================*/
#define GET_VSIF_HVFRMT(Ptr)            ((Ptr[7] & 0xE0) >> 5)              
#define GET_VSIF_3D_STRUCT(Ptr)         ((Ptr[8] & 0xF0) >> 4)
#define GET_VSIF_HDMI_VIC(Ptr)          (Ptr[8])
#define GET_VSIF_3D_STATUS(Ptr)         ((Ptr[7] & 0x40) >> 6)


#define GET_3D_STATUS(Ptr)                 ((Ptr[7] & 0x40) >> 6)
#define GET_3D_FORMAT(Ptr)                 ((Ptr[8] & 0xF0) >> 4)
/*========================================
 * Audio info frame macros
 *=======================================*/
#define SET_AUDIF_CC(Ptr,cc)            (Ptr[3]=(Ptr[3]&(~0x07))|(cc&7))
#define SET_AUDIF_CA(Ptr,ca)            (Ptr[6] = ca)

#define GET_AUDIF_VER(Ptr)              (Ptr[1])
#define GET_AUDIF_CC(Ptr)               (Ptr[3]&0x7)
#define GET_AUDIF_CT(Ptr)               ((Ptr[3]>>4)&0xf)
#define GET_AUDIF_SS(Ptr)               (Ptr[4]&0x3)
#define GET_AUDIF_SF(Ptr)               ((Ptr[4]>>2)&0x7)
#define GET_AUDIF_CT_X(Ptr)             (Ptr[5])
#define GET_AUDIF_CA(Ptr)               (Ptr[6])
#define GET_AUDIF_LSV(Ptr)              ((Ptr[7]>>3)&0xf)
#define GET_AUDIF_DM_INH(Ptr)           ((Ptr[7]>>7)&0x1)

/*========================================
 * Channel status macros
 *=======================================*/
#define GET_CHST_PROFF_APP(Ptr)         (Ptr[0]&1)
#define GET_CHST_AUD_SAMP_TYPE(Ptr)     ((Ptr[0]>>1) & 0x01)
#define GET_CHST_COPYRIGHT(Ptr)         ((Ptr[0]>>2) & 0x01)
#define GET_CHST_EMPHASIS(Ptr)          ((Ptr[0]>>3) & 0x07)
#define GET_CHST_MODE(Ptr)              ((Ptr[0]>>6) & 0x03)
#define GET_CHST_CATG_CODE(Ptr)         (Ptr[1])
#define GET_CHST_SRC_NUM(Ptr)           (Ptr[2] & 0x0f)
#define GET_CHST_CH_NUM(Ptr)            ((Ptr[2]>>4) & 0x0f)
#define GET_CHST_SAMP_FREQ(Ptr)         (Ptr[3] & 0x0f)
#define GET_CHST_CLK_ACCUR(Ptr)         ((Ptr[3]>>4) & 0x03)
#define GET_CHST_RESERV0(Ptr)           ((Ptr[3]>>6) & 0x03)
#define GET_CHST_MAX_WORD_LEN(Ptr)      (Ptr[4] & 0x01)
#define GET_CHST_WORD_LEN(Ptr)          ((Ptr[4]>>1) & 0x07)
#define GET_CHST_4BIT_WORD_LEN(Ptr)     (Ptr[4] & 0x0f)

#define GET_N_VALUE(Ptr)                ((((UINT32)(Ptr[0]))<<16) | (((UINT32)(Ptr[1]))<< 8) | ((UINT32)(Ptr[1])))

/*========================================
 * CEC macros
 *=======================================*/
/*========================================
 * CEC macros
 *=======================================*/
#define CEC_RX_BUFFER1                         0
#define CEC_RX_BUFFER2                         1
#define CEC_RX_BUFFER3                         2
#define CEC_TRIPLE_NUMBER                      3

/* CEC OPCODE */

#define CEC_MSG_MIN_LENGTH                       2  /* each CEC message has at least 2 bytes (header+opcode) */
#define CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST_LENGTH 4
#define CEC_TV_LOGIC_ADDRESS                   0x00

#define CEC_MSG_FEATURE_ABORT                  0x00
#define CEC_MSG_IMAGE_VIEW_ON                  0x04
#define CEC_MSG_TUNER_STEP_INC                 0x05
#define CEC_MSG_TUNER_STEP_DEC                 0x06
#define CEC_MSG_TUNER_DEV_STATUS               0x07
#define CEC_MSG_GIVE_TUNER_DEV_STATUS          0x08
#define CEC_MSG_RECORD_ON                      0x09
#define CEC_MSG_RECORD_STATUS                  0x0A
#define CEC_MSG_RECORD_OFF                     0x0B
#define CEC_MSG_TEXT_VIEW_ON                   0x0D
#define CEC_MSG_RECORD_TV_SCREEN               0x0F
#define CEC_MSG_GIVE_DECK_STATUS               0x1A
#define CEC_MSG_DECK_STATUS                    0x1B
#define CEC_MSG_SET_MENU_LANGUAGE              0x32
#define CEC_MSG_CLR_ANALOGUE_TIMER             0x33
#define CEC_MSG_SET_ANALOGUE_TIMER             0x34
#define CEC_MSG_TIMER_STATUS                   0x35
#define CEC_MSG_STANDBY                        0x36
#define CEC_MSG_PLAY                           0x41
#define CEC_MSG_DECK_CONTROL                   0x42
#define CEC_MSG_TIMER_CLEARED_STATUS           0x43
#define CEC_MSG_USER_CONTROL_PRESSED           0x44
#define CEC_MSG_USER_CONTROL_RELEASED          0x45
#define CEC_MSG_GIVE_OSD_NAME                  0x46
#define CEC_MSG_SET_OSD_NAME                   0x47
#define CEC_MSG_SET_OSD_STRING                 0x64
#define CEC_MSG_SET_TIMER_PROG_TITLE           0x67
#define CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST      0x70
#define CEC_MSG_GIVE_AUDIO_STATUS              0x71
#define CEC_MSG_SET_SYSTEM_AUDIO_MODE          0x72
#define CEC_MSG_REPORT_AUDIO_STATUS            0x7A
#define CEC_MSG_GIVE_SYSTEM_AUDIO_MODE_STATUS  0x7D
#define CEC_MSG_SYSTEM_AUDIO_MODE_STATUS       0x7E
#define CEC_MSG_ROUTE_CHANGE                   0x80
#define CEC_MSG_ROUTE_INFO                     0x81
#define CEC_MSG_ACTIVE_SRC                     0x82
#define CEC_MSG_GIVE_PHYS_ADDR                 0x83
#define CEC_MSG_REPORT_PHYS_ADDR               0x84
#define CEC_MSG_REQ_ACTIVE_SRC                 0x85
#define CEC_MSG_SET_STRM_PATH                  0x86
#define CEC_MSG_DEVICE_VENDOR_ID               0x87
#define CEC_MSG_VENDOR_COMMAND                 0x89
#define CEC_MSG_VENDOR_REMOTE_BTN_DOWN         0x8A
#define CEC_MSG_VENDOR_REMOTE_BTN_UP           0x8B
#define CEC_MSG_GET_VENDOR_ID                  0x8C
#define CEC_MSG_MENU_REQUEST                   0x8D
#define CEC_MSG_MENU_STATUS                    0x8E
#define CEC_MSG_GIVE_PWR_STATUS                0x8F
#define CEC_MSG_REPORT_PWR_STATUS              0x90
#define CEC_MSG_GET_MENU_LANGUAGE              0x91
#define CEC_MSG_SEL_ANALOGUE_SERVICE           0x92
#define CEC_MSG_SEL_DIGITAL_SERVICE            0x93
#define CEC_MSG_SET_DIGITAL_TIMER              0x97
#define CEC_MSG_CLR_DIGITAL_TIMER              0x99
#define CEC_MSG_SET_AUDIO_RATE                 0x9A
#define CEC_MSG_INACTIVE_SOURCE                0x9D
#define CEC_MSG_CEC_VERSION                    0x9E
#define CEC_MSG_GET_CEC_VERSION                0x9F
#define CEC_MSG_VENDOR_CMD_WITH_ID             0xA0
#define CEC_MSG_CLR_EXTERNAL_TIMER             0xA1
#define CEC_MSG_SET_EXTERNAL_TIMER             0xA2
#define CEC_MSG_REPORT_SHORT_AUDIO_DESCRIPTOR  0xA3
#define CEC_MSG_REQUEST_SHORT_AUDIO_DESCRIPTOR 0xA4
#define CEC_MSG_INITIATE_ARC                   0xC0
#define CEC_MSG_REPORT_ARC_INITIATED           0xC1
#define CEC_MSG_REPORT_ARC_TERMINATED          0xC2
#define CEC_MSG_REQUEST_ARC_INITIATION         0xC3
#define CEC_MSG_REQUEST_ARC_TERMINATION        0xC4
#define CEC_MSG_TERMINATE_ARC                  0xC5
#define CEC_MSG_CDC_MESSAGE                    0xF8
#define CEC_MSG_ABORT                          0xFF


#define CEC_POWER_STATUS_ON             0
#define CEC_POWER_STATUS_STNDBY         1
#define CEC_POWER_STATUS_STNDBY2ON      2 /*  in transition Standby to ON */
#define CEC_POWER_STATUS_ON2STNDBY      3 /*  in transition ON to Standby */
#define CEC_SYSTEM_AUDIO_STATUS_ON      1
#define CEC_SYSTEM_AUDIO_STATUS_OFF     0
#define CEC_UICOMMAND_POWER_ON          0x6D

#define CEC_MSG_UICOMMAND_VOLUME_UP                            0x41
#define CEC_MSG_UICOMMAND_VOLUME_DOWN                          0x42
#define CEC_MSG_UICOMMAND_VOLUME_MUTE                          0x43
#define CEC_MSG_UICOMMAND_PLAY                                 0x44
#define CEC_MSG_UICOMMAND_STOP                                 0x45
#define CEC_MSG_UICOMMAND_REWIND                               0x48
#define CEC_MSG_UICOMMAND_FAST_FORWARD                         0x49
#define CEC_MSG_UICOMMAND_PAUSE                                0x46
#define CEC_MSG_UICOMMAND_FORWARD                              0x4B
#define CEC_MSG_UICOMMAND_BACKWARD                             0x4C

#define CEC_MSG_UICOMMAND_ROOT_MENU                            0x09
#define CEC_MSG_UICOMMAND_SETUP_MENU                           0x0A
#define CEC_MSG_UICOMMAND_CONTENTS_MENU                        0x0B
#define CEC_MSG_UICOMMAND_UP                                   0x01
#define CEC_MSG_UICOMMAND_DOWN                                 0x02
#define CEC_MSG_UICOMMAND_LEFT                                 0x03
#define CEC_MSG_UICOMMAND_RIGHT                                0x04
#define CEC_MSG_UICOMMAND_SELECT                               0x00
#define CEC_MSG_UICOMMAND_EXIT                                 0x0D
#define CEC_MSG_UICOMMAND_POWER_ON                             0x6D

#define CEC_MENU_ACTIVATED                                 0
#define CEC_MENU_DEACTIVATED                               1

/* CEC Logical Address */
#define CEC_TV_LOG_ADDRESS                   0
#define CEC_RECORD1_LOG_ADDRESS              1
#define CEC_RECORD2_LOG_ADDRESS              2
#define CEC_TUNER1_LOG_ADDRESS               3
#define CEC_PLAYBACK1_LOG_ADDRESS            4
#define CEC_AUDIO_LOG_ADDRESS                5
#define CEC_TUNER2_LOG_ADDRESS               6
#define CEC_TUNER3_LOG_ADDRESS               7
#define CEC_PLAYBACK2_LOG_ADDRESS            8
#define CEC_TUNER4_LOG_ADDRESS               10
#define CEC_PLAYBACK3_LOG_ADDRESS            11
#define CEC_FREE_LOG_ADDRESS                 14
#define CEC_BROADCAST_LOG_ADDRESS            15
#define CEC_UNREGISTED_LOG_ADDRESS           15

/* CEC Feature Abort Reason */
#define CEC_ABORT_REASON_UNRECOGNIZED_OPCODE            0
#define CEC_ABORT_REASON_NOT_IN_CORRECT_MODE            1
#define CEC_ABORT_REASON_CANNOT_PROVIDE_SOURCE          2
#define CEC_ABORT_REASON_INVALID_OPERAND                3
#define CEC_ABORT_REASON_REFUSED                        4
#define CEC_ABORT_REASON_UNABLE_TO_DETERMINE            5

#define CEC_OPCODE(a)                   (a[1])
#define CEC_SRC(a)                      ((a[0]&0xf0)>>4)
#define CEC_DST(a)                      (a[0]&0x0f)
#define CEC_BROADCAST(a)                (CEC_DST(a)==0x0f)
#define CEC_PHYS_ADDR1(a)               (((UINT16)(a[2])<<8)+(UINT16)(a[3]))
#define CEC_PHYS_ADDR2(a)               (((UINT16)(a[4])<<8)+(UINT16)(a[5]))

#define CEC_SET_HDR_BC(m,pa)            m[0]=(pa<<4)|0x0f
#define CEC_SET_HDR_DA(m,a,pa)          m[0]=(pa<<4)|a
#define CEC_SET_OPCODE(m,o)             m[1]=o
#define CEC_SET_PHYS_ADDR1(m,a)         m[2]=(UCHAR)((a)>>8);m[3]=(UCHAR)(a)&0xff
#define CEC_SET_PHYS_ADDR2(m,a)         m[4]=(UCHAR)((a)>>8);m[5]=(UCHAR)(a)&0xff

/*========================================
 * Auxiliary 8-bit I2C field access macros
 *=======================================*/
#define ATV_I2CIsField8                             (BOOL)ATV_I2CReadField8
#define ATV_I2CGetField8(d,r,m,b,p)                 *p=ATV_I2CReadField8(d,r,m,b)
#define ATV_I2CGetField32(d,r,Mm,Lm,b,s,p)          *p=ATV_I2CReadField32(d,r,Mm,Lm,b,s)
#define ATV_I2CGetFragField32(d,t,Mm,Lm,b,s,p)      *p=ATV_I2CReadFragField32(d,t,Mm,Lm,b,s)
#define ATV_I2CGetField32LE(d,r,Mm,Lm,b,s,p)        *p=ATV_I2CReadField32LE(d,r,Mm,Lm,b,s)
#define ATV_I2CGetFragField32LE(d,t,Mm,Lm,b,s,p)    *p=ATV_I2CReadFragField32LE(d,t,Mm,Lm,b,s)
#define ATV_I2CGetRandField32(d,Fi,s,p)             *p=ATV_I2CReadRandField32(d,Fi,s)
#define ATV_I2CGetMultiField(d,r,s,p)               HAL_I2CReadBlock(d,r,p,(UINT16)s)
#define ATV_I2CSetMultiField(d,r,s,p)               HAL_I2CWriteBlock(d,r,p,(UINT16)s)
/*========================================
 * Auxiliary 16-bit I2C field access macros
 *=======================================*/
#define ATV_I2C16IsField8                           (BOOL)ATV_I2C16ReadField8
#define ATV_I2C16GetField8(d,r,m,b,p)               *p=ATV_I2C16ReadField8(d,r,m,b)
#define ATV_I2C16GetField32(d,r,Mm,Lm,b,s,p)        *p=ATV_I2C16ReadField32(d,r,Mm,Lm,b,s)
#define ATV_I2C16GetFragField32(d,t,Mm,Lm,b,s,p)    *p=ATV_I2C16ReadFragField32(d,t,Mm,Lm,b,s)
#define ATV_I2C16GetField32LE(d,r,Mm,Lm,b,s,p)      *p=ATV_I2C16ReadField32LE(d,r,Mm,Lm,b,s)
#define ATV_I2C16GetFragField32LE(d,t,Mm,Lm,b,s,p)  *p=ATV_I2C16ReadFragField32LE(d,t,Mm,Lm,b,s)
#define ATV_I2C16GetMultiField(d,r,s,p)             HAL_I2C16ReadBlock8(d,r,p,(UINT16)s)
#define ATV_I2C16SetMultiField(d,r,s,p)             HAL_I2C16WriteBlock8(d,r,p,(UINT16)s)

/*========================================
 * Enums and structures
 *=======================================*/
typedef enum
{
    ATVERR_OK=0,
    ATVERR_FALSE=0,
    ATVERR_TRUE=1,
    ATVERR_INV_PARM,
    ATVERR_NOT_AVAILABLE,
    ATVERR_FAILED
} ATV_ERR;

typedef struct
{
    UCHAR   StrgId;
    char    *Strg;
} STRG_TABLE;

typedef struct
{
    UINT32  StartTime;      /* Will be set to 0 when threshold expires */
    UINT16  Threshold;      /* Timeout in ms from StartTime */
    BOOL    TimedOut;       /* Will be set to TRUE when timeout expires */
} TIMEOUT_TABLE;

typedef struct
{
    UCHAR   RegAddr;        /* Register address */
    UCHAR   Mask;           /* Mask for the bits in RegAddr */
    UCHAR   LShift;         /* Absolute left shift (in 2's complement) of */
                            /* field bits in RegAddr */
    UCHAR   Reserved;
} I2C_FIELD_INFO;

typedef enum
{
    VID3D_STRCT_FR_PCK=0,
    VID3D_STRCT_FLD_ALT,
    VID3D_STRCT_LINE_ALT,
    VID3D_STRCT_SIDE_BY_SIDE_FULL,
    VID3D_STRCT_L_PLUS_DEPTH,
    VID3D_STRCT_LDEPTH_PLUS_GR_PLUS_GRDEPTH,
    VID3D_STRCT_TOP_AND_BOTTOM,
    VID3D_STRCT_RSVD_1,
    VID3D_STRCT_SIDE_BY_SIDE_HALF,
    VID3D_NONE
}VID3D_FORMAT;

typedef enum
{
    ATV_PORT_A      = 0,                /* The value of the port is also    */
    ATV_PORT_B      = 1,                /* the port index (0-3)             */
    ATV_PORT_C      = 2,
    ATV_PORT_D      = 3,
    ATV_PORT_NONE   = 0xFE,
    ATV_PORT_ALL    = 0xFF
} ATV_HDMI_PORT;

typedef struct
{
    UCHAR Major;
    UCHAR Minor;
    UCHAR ReleaseCandidate;
    UCHAR Build[6];
} ATV_APP_REVISION_INFO;

/*================================
 * Prototypes
 *===============================*/
#ifdef __cplusplus
extern "C" {
#endif

UCHAR   ATV_I2CReadField8   (UCHAR DevAddr, UCHAR RegAddr, UCHAR Mask,
                             UCHAR BitPos);
void    ATV_I2CWriteField8  (UCHAR DevAddr, UCHAR RegAddr, UCHAR Mask,
                             UCHAR BitPos,  UCHAR FieldVal);
void   ATV_I2CWriteOField8  (UCHAR DevAddr, UCHAR RegAddr, UCHAR Mask,
                             UCHAR BitPos, UCHAR *CurrVal, UCHAR FieldVal);
UINT32  ATV_I2CReadField32  (UCHAR DevAddr, UCHAR RegAddr, UCHAR MsbMask,
                             UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan);
UINT32  ATV_I2CReadFragField32 (UCHAR DevAddr, UCHAR *AddrTbl, UCHAR MsbMask,
                                UCHAR LsbMask, UCHAR LsbPos,  UCHAR FldSpan);
UINT32  ATV_I2CReadField32LE   (UCHAR DevAddr, UCHAR RegAddr, UCHAR MsbMask,
                                UCHAR LsbMask, UCHAR LsbPos,  UCHAR FldSpan);
UINT32  ATV_I2CReadFragField32LE (UCHAR DevAddr, UCHAR *AddrTbl, UCHAR MsbMask,
                                  UCHAR LsbMask, UCHAR LsbPos,  UCHAR FldSpan);
void    ATV_I2CWriteField32      (UCHAR DevAddr, UCHAR RegAddr, UCHAR MsbMask,
                                  UCHAR LsbMask, UCHAR LsbPos,  UCHAR FldSpan,
                                  UINT32 Val);
void    ATV_I2CWriteField32LE    (UCHAR DevAddr, UCHAR RegAddr, UCHAR MsbMask,
                                  UCHAR LsbMask, UCHAR LsbPos,  UCHAR FldSpan,
                                  UINT32 Val);
void    ATV_I2CWriteFragField32  (UCHAR DevAddr, UCHAR *AddrTbl, UCHAR MsbMask,
                                  UCHAR LsbMask, UCHAR LsbPos,   UCHAR FldSpan,
                                  UINT32 Val);
void    ATV_I2CWriteFragField32LE (UCHAR DevAddr, UCHAR *AddrTbl,
                                   UCHAR MsbMask, UCHAR LsbMask,
                                   UCHAR LsbPos,  UCHAR FldSpan, UINT32 Val);
UINT32  ATV_I2CReadRandField32  (UCHAR DevAddr, I2C_FIELD_INFO *FldInfo,
                                 UCHAR FldSpan);
void    ATV_I2CWriteRandField32 (UCHAR DevAddr, I2C_FIELD_INFO *FldInfo,
                                 UCHAR FldSpan, UINT32 Val);
UCHAR   ATV_I2C16ReadField8 (UCHAR DevAddr, UINT16 RegAddr, UCHAR Mask,
                             UCHAR BitPos);
void    ATV_I2C16WriteField8 (UCHAR DevAddr, UINT16 RegAddr, UCHAR Mask,
                              UCHAR BitPos, UCHAR FieldVal);
void    ATV_I2C16WriteOField8 (UCHAR DevAddr, UINT16 RegAddr, UCHAR Mask,
                               UCHAR BitPos, UCHAR *CurrVal, UCHAR FieldVal);
UINT32  ATV_I2C16ReadField32 (UCHAR DevAddr, UINT16 RegAddr, UCHAR MsbMask,
                              UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan);
UINT32  ATV_I2C16ReadFragField32 (UCHAR DevAddr, UINT16 *AddrTbl, UCHAR MsbMask,
                                  UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan);
UINT32  ATV_I2C16ReadField32LE (UCHAR DevAddr, UINT16 RegAddr, UCHAR MsbMask,
                                UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan);
UINT32  ATV_I2C16ReadFragField32LE (UCHAR DevAddr, UINT16 *AddrTbl, UCHAR MsbMask,
                                    UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan);
void    ATV_I2C16WriteField32 (UCHAR DevAddr, UINT16 RegAddr, UCHAR MsbMask,
                               UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val);
void    ATV_I2C16WriteField32LE (UCHAR DevAddr, UINT16 RegAddr, UCHAR MsbMask,
                                 UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val);
void    ATV_I2C16WriteFragField32 (UCHAR DevAddr, UINT16 *AddrTbl, UCHAR MsbMask,
                                   UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val);
void    ATV_I2C16WriteFragField32LE (UCHAR DevAddr, UINT16 *AddrTbl, UCHAR MsbMask,
                                     UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val);
UCHAR   ATV_Reg (UCHAR Dev, UCHAR Reg);
void    ATV_DelaySec (UINT16 Counter);
UINT32  ATV_GetElapsedMs (UINT32 StartCount, UINT32 *CurrMsCount);
UINT32  ATV_GetMsCountNZ (void);
UINT16  ATV_LookupValue8 (UCHAR *Table, UCHAR Value, UCHAR EndVal, UINT16 Step);
void    ATV_I2CWriteTable (UCHAR *Table, UCHAR EndVal);
void    ATV_I2C16WriteTable (UCHAR *Table, UCHAR EndVal);
void    ATV_I2CWriteFields (UCHAR *Table, UCHAR EndVal);
char   *ATV_LookupStrgTable (STRG_TABLE *Table, UCHAR StrgId, UCHAR EndVal);
void    ATV_PrintTime (char *Prefix, UCHAR Gran, char *Postfix);
void    ATV_CheckTimeOut (TIMEOUT_TABLE *Table);
ATV_ERR ATV_CecValidMsgSize (UCHAR *Table, UCHAR TableSize, UCHAR Opcode,
                             UCHAR MsgSize, UCHAR *ExpMsgSize);

CONSTANT ATV_APP_REVISION_INFO  *ATV_GetAppRevisionInfo (void);

#ifdef __cplusplus
}
#endif

#endif
