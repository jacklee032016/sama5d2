/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "atv_types.h"
//#include "platform_config.h"

#define I2C_PROFILER                0
#define ASSERT_DISABLE				1 

#define HAL_PORT_IDX_ALL            0xFE

#define RX_BOARD_DEV_ID             0xE8
#define TX_BOARD_DEV_ID             0xEA

#define RX_7840_BOARD_REV_0         0x06
#define RX_7840_BOARD_REV_A         0x10
#define RX_7604_BOARD_REV_0         0x0B
#define RX_7604_BOARD_REV_A         0x11
#define RX_7844_BOARD_REV_0         0x13
#define RX_7844_BOARD_REV_A         0x1A
#define RX_7844_BOARD_REV_B         0x1B
#define RX_7842_BOARD_REV_0         0x17
#define RX_7842_BOARD_REV_A         0x17
#define RX_7842_BOARD_REV_B         0x18

#define TX_7510_BOARD_REV_A         0x04
#define TX_7510_BOARD_REV_B         0x06
#define TX_7510_BOARD_REV_C         0x0A
#define TX_7510_BOARD_REV_D         0x05
#define TX_7511_BOARD_REV_D         0x0F
#define TX_7520_BOARD_REV_A         0x0C
#define TX_7520_BOARD_REV_B         0x08
#define TX_7520NK_BOARD_REV_A       0x04

enum
{
    OUTPUT_CHAN_AUTO=0,
    OUTPUT_CHAN_AVO1,
    OUTPUT_CHAN_AVO2,
    OUTPUT_CHAN_DAC,
    OUTPUT_CHAN_ENC,
    OUTPUT_CHAN_SDP_ENC,
    PATHB_OUTPUT_CHAN_AUTO,
    OUTPUT_CHAN_3G_A /* Added for 7619 build */
};

enum
{
    STATUS_LED1=1,
    STATUS_LED2,
    STATUS_LED3,
    STATUS_LED4,
    STATUS_LED5,
    STATUS_LED6
};

#if I2C_PROFILER
void    Profiler_ResetCounters (void);
void    Profiler_PrintLog (void);
void    Profiler_LogI2cRead (UCHAR Dev, UINT16 Count);
void    Profiler_LogI2cWrite (UCHAR Dev, UINT16 Count);
#else
#define Profiler_ResetCounters()
#define Profiler_PrintLog()
#define Profiler_LogI2cRead(a,b)
#define Profiler_LogI2cWrite(a,b)
#endif

#if ASSERT_DISABLE || NDEBUG
#define ASSERT(...)
#else
#define ASSERT(condition)           adi_assert(__FILE__, __LINE__, __func__, (condition), #condition)
#endif

UCHAR   HAL_I2CReadByte (UCHAR Dev, UCHAR Reg, UCHAR *Data);
UCHAR   HAL_I2CWriteByte (UCHAR Dev, UCHAR Reg, UCHAR Data);
UINT16  HAL_I2CReadBlock (UCHAR Dev, UCHAR Reg, UCHAR *Data, UINT16 NumberBytes);
UINT16  HAL_I2CWriteBlock (UCHAR Dev, UCHAR Reg, UCHAR *Data, UINT16 NumberBytes);
UINT16  HAL_I2CReadAddr16Block8 (UCHAR Dev, UINT16 Reg, UCHAR* Data, 
                                 UINT16 NumberBytes);
UINT16  HAL_I2CWriteAddr16Block8 (UCHAR Dev, UINT16 Reg, UCHAR* Data, 
                                  UINT16 NumberBytes);
UINT16  HAL_I2CReadAddr16Block16 (UCHAR Dev, UINT16 Reg, UINT16* Data, 
                                  UINT16 NumberWords);
UINT16  HAL_I2CWriteAddr16Block16 (UCHAR Dev, UINT16 Reg, UINT16* Data, 
                                   UINT16 NumberWords);
UINT16  HAL_I2CGenericAccess (UCHAR Addr, UINT16 WriteCount, UCHAR *WriteBuf, 
                              UINT16 ReadCount, UCHAR *ReadBuf);
void    HAL_DelayMs (UINT16 Counter);
UINT32  HAL_GetCurrentMsCount(void);
BOOL    HAL_GetUartByte (UCHAR *UartCh);
BOOL    HAL_SendUartByte (UCHAR UartCh);
BOOL    HAL_GetIrCode (UINT16 *Ir_code);
void    HAL_AudioDACInit(void);
void    HAL_AudioDACMute(void);
void    HAL_AudioDACUnmute(void);
void    HAL_AudioOutputConfig(UINT16 SampFreq);
void    HAL_VideoDACEnable(BOOL Enable);
BOOL    HAL_GetMBSwitchState (void);
UCHAR   HAL_GetMBButtonsState (void);
BOOL    HAL_RxInt1Pending (void);
BOOL    HAL_RxInt2Pending (void);
BOOL    HAL_TxIntPending (void);
void    HAL_PlatformInit (void);
void    HAL_AssertHPD (UCHAR PortIdx, BOOL Val);
BOOL    HAL_IsHPDOn (UCHAR PortIdx);
void    HAL_EnableTxHPD (BOOL Val);
BOOL    HAL_5VDetected (UCHAR PortIdx);
UINT16  HAL_GetHwRevision (void);
char   *HAL_GetPlatformName (void);
void    HAL_EnableHDMIPath (UCHAR ChanId);
UCHAR   HAL_GetRxBoardRevision (void);
UCHAR   HAL_GetTxBoardRevision (void);
void    HAL_MainReset (void);
void    HAL_MpuReset (void);
void    HAL_ContextCheck (void);
void    HAL_SwitchContext (BOOL RepContext, void (*EntryPoint) (void));
void    HAL_SetLedState (UCHAR LedId, BOOL value);
void    HAL_PlatformRXInit (void);
void    HAL_PlatformTXInit (void);

#define LED1	0x01
#define LED2	0x02
#define LED3	0x04
#define LED4	0x08
#define LED5	0x10
#define HEARTBEAT_RATE 2000

#ifdef ADV7850_BOARD
void HAL_OutputLEDS();
void HAL_LED_On(UCHAR led);
void HAL_LED_Off(UCHAR led);
void HAL_LED_Toggle(UCHAR led);
void HAL_HeartBeat();
#endif
void HAL_I2C_LogInit(void);
void HAL_I2C_LogStop(void);
void HAL_I2C_GetLogInfo(UINT32*, UINT16*);

#endif
