/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#ifndef _CEC_H_
#define _CEC_H_

/*===============================================
 * Registers
 *==============================================*/
#define CEC_REG_TX_FRAME_HDR            0x00
#define CEC_REG_TX_FRAME_DATA0          0x01
#define CEC_REG_TX_FRAME_LEN            0x10
#define CEC_REG_TX_ENABLE               0x11
#define CEC_REG_TX_RETRY_COUNT          0x12
#define CEC_REG_TX_ERR_COUNTERS         0x14
#define CEC_REG_RX_FRAME_HDR            0x15
#define CEC_REG_RX_FRAME_DATA0          0x16
#define CEC_REG_RX_FRAME_LEN            0x25
#define CEC_REG_RX_ENABLE               0x26
#if ((TX_INCLUDE_CEC) && ((TX_DEVICE == 8002) || (TX_DEVICE == 8003)) )
#define CEC_REG_RX_BUFFER_STATUS        0x49
#define CEC_REG_RX_BUFFER_CONTROL       0x4A
#define CEC_REG_RX_LOG_ADDR_MASK        0x4B
#define CEC_REG_RX_LOG_ADDR_0_1         0x4C
#define CEC_REG_RX_LOG_ADDR_2           0x4D
#define CEC_REG_CLK_DIV                 0x4E
#define CEC_REG_SOFT_RESET              0x50             
#else
#define CEC_REG_RX_LOG_ADDR_MASK        0x27
#define CEC_REG_RX_LOG_ADDR_0_1         0x28
#define CEC_REG_RX_LOG_ADDR_2           0x29
#define CEC_REG_CLK_DIV                 0x2A
#define CEC_REG_SOFT_RESET              0x2C
#endif

#if CEC_TX_2G
#undef  CEC_REG_RX_LOG_ADDR_MASK        
#undef  CEC_REG_RX_LOG_ADDR_0_1         
#undef  CEC_REG_RX_LOG_ADDR_2 
#undef  CEC_REG_CLK_DIV 
#undef  CEC_REG_SOFT_RESET 
#define CEC_REG_RX_BUFFER_STATUS        0x49
#define CEC_REG_RX_BUFFER_CONTROL       0x4A
#define CEC_REG_RX_LOG_ADDR_MASK        0x4B
#define CEC_REG_RX_LOG_ADDR_0_1         0x4C
#define CEC_REG_RX_LOG_ADDR_2           0x4D
#define CEC_REG_CLK_DIV                 0x4E
#define CEC_REG_SOFT_RESET              0x50             
#endif



/*===============================================
 * Macros
 *==============================================*/
#if ((TX_INCLUDE_CEC) && ((TX_DEVICE == 8002) || (TX_DEVICE == 8003)) )
#define CECREG_SET_TX_FRAME_HDR(val)        ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_TX_FRAME_HDR), 0xff, 0, val)
#define CECREG_SET_TX_FRAME_LEN(val)        ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_TX_FRAME_LEN), 0xff, 0, val)
#define CECREG_SET_TX_ENABLE(val)           ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_TX_ENABLE), 0x01, 0, val)
#define CECREG_SET_TX_RETRY_COUNT(val)      ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_TX_RETRY_COUNT), 0x70, 4, val)
#define CECREG_GET_TX_LOW_DRIVE_COUNT       ATV_I2C16ReadField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_TX_ERR_COUNTERS), 0xf0, 4)
#define CECREG_GET_TX_NACK_COUNT            ATV_I2C16ReadField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_TX_ERR_COUNTERS), 0x0f, 0)
#define CECREG_GET_RX_FRAME_HDR             ATV_I2C16ReadField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_FRAME_HDR), 0xff, 0)
#define CECREG_GET_RX_FRAME_LEN             ATV_I2C16ReadField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_FRAME_LEN), 0x1f, 0)
#define CECREG_SET_RX_ENABLE(val)           ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_ENABLE), 0x01, 0, val)
#define CECREG_SET_RX_LOG_ADDR_MASK(val)    ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_LOG_ADDR_MASK), 0x70, 4, val)
#define CECREG_GET_RX_LOG_ADDR_MASK         ATV_I2C16ReadField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_LOG_ADDR_MASK), 0x70, 4)
#define CECREG_SET_RX_LOG_ADDR_0(val)       ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_LOG_ADDR_0_1), 0x0f, 0, val)
#define CECREG_SET_RX_LOG_ADDR_1(val)       ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_LOG_ADDR_0_1), 0xf0, 4, val)
#define CECREG_SET_RX_LOG_ADDR_2(val)       ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_LOG_ADDR_2), 0x0f, 0, val)
#define CECREG_SET_CLK_DIV(val)             ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_CLK_DIV), 0xfc, 2, val)
#define CECREG_SET_POWER_MODE(val)          ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_CLK_DIV), 0x03, 0, val)
#define CECREG_SET_SOFT_RESET(val)          ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_SOFT_RESET), 0x01, 0, val)
#define CECREG_SET_NUM_RX_BUFFERS(val)      ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_BUFFER_CONTROL), 0x08, 3, val)
#define CECREG_GET_RX_BUFFER1_READY(val)    ATV_I2C16ReadField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_BUFFER_CONTROL), 0x01, 0)
#define CECREG_GET_RX_BUFFER2_READY(val)    ATV_I2C16ReadField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_BUFFER_CONTROL), 0x02, 1)
#define CECREG_GET_RX_BUFFER3_READY(val)    ATV_I2C16ReadField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_BUFFER_CONTROL), 0x04, 2)
#define CECREG_SET_RX_BUFFER1_READY(val)    ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_BUFFER_CONTROL), 0x01, 0, val)
#define CECREG_SET_RX_BUFFER2_READY(val)    ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_BUFFER_CONTROL), 0x02, 1, val)
#define CECREG_SET_RX_BUFFER3_READY(val)    ATV_I2C16WriteField8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CEC_REG_RX_BUFFER_CONTROL), 0x04, 2, val)
#define CECREG_I2CWriteBlock(CECMap, CECReg, MsgPtr, MsgLen)    HAL_I2C16WriteBlock8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CECReg), MsgPtr, MsgLen)
#define CECREG_I2CReadBlock(CECMap, CECReg, MsgPtr, MsgLen)     HAL_I2C16ReadBlock8(CEC_I2C_MAP_BASE, VTX_REG_OFFSET(0xF0, CECReg), MsgPtr, MsgLen)
 
#else

#define CECREG_SET_TX_FRAME_HDR(val)        ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_TX_FRAME_HDR, 0xff, 0, val)
#define CECREG_SET_TX_FRAME_LEN(val)        ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_TX_FRAME_LEN, 0x1f, 0, val)
#define CECREG_SET_TX_ENABLE(val)           ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_TX_ENABLE, 0x01, 0, val)
#define CECREG_SET_TX_RETRY_COUNT(val)      ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_TX_RETRY_COUNT, 0x70, 4, val)
#define CECREG_GET_TX_LOW_DRIVE_COUNT       ATV_I2CReadField8(CEC_I2C_MAP_BASE, CEC_REG_TX_ERR_COUNTERS, 0xf0, 4)
#define CECREG_GET_TX_NACK_COUNT            ATV_I2CReadField8(CEC_I2C_MAP_BASE, CEC_REG_TX_ERR_COUNTERS, 0x0f, 0)

#define CECREG_GET_RX_FRAME_HDR             ATV_I2CReadField8(CEC_I2C_MAP_BASE, CEC_REG_RX_FRAME_HDR, 0xff, 0)
#define CECREG_GET_RX_FRAME_LEN             ATV_I2CReadField8(CEC_I2C_MAP_BASE, CEC_REG_RX_FRAME_LEN, 0x1f, 0)
#if ((RX_INCLUDE_CEC) && (CEC_CFG02_REV_2))
#define CECREG_SET_RX_ENABLE(val)           ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_SOFT_RESET, 0x02, 1, val)
#else
#define CECREG_SET_RX_ENABLE(val)           ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_ENABLE, 0x01, 0, val)
#endif
#define CECREG_SET_RX_LOG_ADDR_MASK(val)    ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_LOG_ADDR_MASK, 0x70, 4, val)
#define CECREG_GET_RX_LOG_ADDR_MASK         ATV_I2CReadField8(CEC_I2C_MAP_BASE, CEC_REG_RX_LOG_ADDR_MASK, 0x70, 4)
#define CECREG_SET_RX_LOG_ADDR_0(val)       ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_LOG_ADDR_0_1, 0x0f, 0, val)
#define CECREG_SET_RX_LOG_ADDR_1(val)       ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_LOG_ADDR_0_1, 0xf0, 4, val)
#define CECREG_SET_RX_LOG_ADDR_2(val)       ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_LOG_ADDR_2, 0x0f, 0, val)

#define CECREG_SET_CLK_DIV(val)             ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_CLK_DIV, 0xfc, 2, val)
#define CECREG_SET_POWER_MODE(val)          ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_CLK_DIV, 0x03, 0, val)
#define CECREG_SET_SOFT_RESET(val)          ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_SOFT_RESET, 0x01, 0, val)


#if CEC_TX_2G
#define CECREG_SET_NUM_RX_BUFFERS(val)      ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_BUFFER_CONTROL, 0x08, 3, val)
#define CECREG_GET_RX_BUFFER1_READY(val)    ATV_I2CReadField8(CEC_I2C_MAP_BASE, CEC_REG_RX_BUFFER_CONTROL, 0x01, 0)
#define CECREG_GET_RX_BUFFER2_READY(val)    ATV_I2CReadField8(CEC_I2C_MAP_BASE, CEC_REG_RX_BUFFER_CONTROL, 0x02, 1)
#define CECREG_GET_RX_BUFFER3_READY(val)    ATV_I2CReadField8(CEC_I2C_MAP_BASE, CEC_REG_RX_BUFFER_CONTROL, 0x04, 2)
#define CECREG_SET_RX_BUFFER1_READY(val)    ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_BUFFER_CONTROL, 0x01, 0, val)
#define CECREG_SET_RX_BUFFER2_READY(val)    ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_BUFFER_CONTROL, 0x02, 1, val)
#define CECREG_SET_RX_BUFFER3_READY(val)    ATV_I2CWriteField8(CEC_I2C_MAP_BASE, CEC_REG_RX_BUFFER_CONTROL, 0x04, 2, val)
#endif

#define CECREG_I2CWriteBlock(CECMap, CECReg, MsgPtr, MsgLen)        HAL_I2CWriteBlock (CECMap, CECReg, MsgPtr, MsgLen)
#define CECREG_I2CReadBlock(CECMap, CECReg, MsgPtr, MsgLen)         HAL_I2CReadBlock (CECMap, CECReg, MsgPtr, MsgLen)

#endif

/*===============================================
 * Declarations and externals
 *==============================================*/
extern UINT16 Cec_Notification(UCHAR Event, UINT16 Len, void *Buff);

enum
{
    CEC_EVENT_RX_MSG, 
    CEC_EVENT_TX_DONE,
    CEC_EVENT_TX_TIMEOUT,
    CEC_EVENT_TX_ARB_LOST,
    CEC_EVENT_LOG_ADDR_ALLOC,
    CEC_EVENT_LOG_ADDR_LIST,
    CEC_EVENT_RX_MSG_RESPOND
};

typedef struct
{
    BOOL TxReady;
    BOOL RxReady;
    BOOL ArbLost;
    BOOL Timeout;
    BOOL RxReady1;
    BOOL RxReady2;
    BOOL RxReady3;
    UCHAR RxFrameOrder[CEC_TRIPLE_NUMBER];    
}CEC_INTERRUPTS;

/*===============================================
 * Prototypes
 *==============================================*/
ATV_ERR CEC_Reset (void);
ATV_ERR CEC_Enable (BOOL Enable);
ATV_ERR CEC_SetLogicalAddr (UCHAR LogAddr, UCHAR DevId, BOOL Enable);
ATV_ERR CEC_SendMessage (UCHAR *MsgPtr, UCHAR MsgLen);
ATV_ERR CEC_SendMessageOut (void);
ATV_ERR CEC_ResendLastMessage (void);
ATV_ERR CEC_AllocateLogAddr (UCHAR *LogAddrList);
ATV_ERR CEC_GetLogAddrList (UCHAR SrcLogAddr);
void    CEC_Isr (CEC_INTERRUPTS *CecInts);

#endif
