/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
#include "atv_types.h"
#include "atv_common.h"
#include "atv_platform.h"

STATIC CONSTANT ATV_APP_REVISION_INFO RevisionInfo =
{
    1,      /* Major revision */
    61,     /* Minor revision */
    7,     /* Release candidate revision
             * 99 represents trunk development, pre-branching before next minor revision
             */
    " "       /* Experimental Version, intermediate release before next release candidate
             * should be 0x20 or " " for Release Candidate
             */
};

UINT32 AtvDummy;         /* This variable is exported to all modules of the  */
                        /* ATV tree to used for compiler warning removal    */

#error  please customise befor use

CONSTANT UINT16 VicInfo[NUM_OF_VICS*4] =
{/* H     V     I   Hz */
    0,    0,    0, 0,       /* 0 */
    640,  480,  0, 60,
    720,  480,  0, 60,
    720,  480,  0, 60,
    1280, 720,  0, 60,
    1920, 1080, 1, 60,      /* 5 */
    1440, 480,  1, 60,
    1440, 480,  1, 60,
    1440, 240,  0, 60,
    1440, 240,  0, 60,
    2880, 480,  1, 60,      /* 10 */
    2880, 480,  1, 60,
    2880, 240,  0, 60,
    2880, 240,  0, 60,
    1440, 480,  0, 60,
    1440, 480,  0, 60,      /* 15 */
    1920, 1080, 0, 60,
    720,  576,  0, 50,
    720,  576,  0, 50,
    1280, 720,  0, 50,
    1920, 1080, 1, 50,      /* 20 */
    1440, 576,  1, 50,
    1440, 576,  1, 50,
    1440, 288,  0, 50,
    1440, 288,  0, 50,
    2880, 576,  1, 50,      /* 25 */
    2880, 576,  1, 50,
    2880, 288,  0, 50,
    2880, 288,  0, 50,
    1440, 576,  0, 50,
    1440, 576,  0, 50,      /* 30 */
    1920, 1080, 0, 50,
    1920, 1080, 0, 24,
    1920, 1080, 0, 25,
    1920, 1080, 0, 30,
    2880, 480,  0, 60,      /* 35 */
    2880, 480,  0, 60,
    2880, 576,  0, 50,
    2880, 576,  0, 50,
    1920, 1080, 1, 50,
    1920, 1080, 1, 100,     /* 40 */
    1280, 720,  0, 100,
    720,  576,  0, 100,
    720,  576,  0, 100,
    1440, 576,  1, 100,
    1440, 576,  1, 100,     /* 45 */
    1920, 1080, 1, 120,
    1280, 720,  0, 120,
    720,  480,  0, 120,
    720,  480,  0, 120,
    1440, 480,  1, 120,     /* 50 */
    1440, 480,  1, 120,
    720,  576,  0, 200,
    720,  576,  0, 200,
    1440, 576,  1, 200,
    1440, 576,  1, 200,     /* 55 */
    720,  480,  0, 240,
    720,  480,  0, 240, 
    1440, 480,  1, 240,
    1440, 480,  1, 240,
    1280, 720,  0, 24,      /* 60 */
    1280, 720,  0, 25,
    1280, 720,  0, 30,
    1920, 1080, 0, 120,
    1920, 1080, 0, 100,     /* 64 */
/* CEA-861-F, new VIC 65~107 */
	/* Aspect: 64:27 */
    1280, 720,  0, 24,      /* 65,*/ 
    1280, 720,  0, 25,
    1280, 720,  0, 30, 
    1280, 720,  0, 50, 
    1280, 720,  0, 60,
    1280, 720,  0, 100,    /* 70 */ 
    1280, 720,  0, 120,
    1920, 1080, 0, 24,
    1920, 1080, 0, 25,
    1920, 1080, 0, 30,
    1920, 1080, 0, 50,
    1920, 1080, 0, 60,
    1920, 1080, 0, 100, 
    1920, 1080, 0, 120, 
    1680, 720,  0, 24,
    1680, 720,  0, 25,     /* 80 */
    1680, 720,  0, 30, 
    1680, 720,  0, 50, 
    1680, 720,  0, 60, 
    1680, 720,  0, 100,
    1680, 720,  0, 120, 
    2560, 1080, 0, 24,
    2560, 1080, 0, 25,
    2560, 1080, 0, 30,
    2560, 1080, 0, 50, 
    2560, 1080, 0, 60,    /* 90 */
    2560, 1080, 0, 100,
    2560, 1080, 0, 120,
    /* 3840x2160, Aspect: 16:9 */
    3840, 2160, 0, 24,
    3840, 2160, 0, 25,
    3840, 2160, 0, 30,
    3840, 2160, 0, 50,   /* 96, for YCbCr 4:2:0 */
    3840, 2160, 0, 60,   /* 97, for YCbCr 4:2:0 */
    /* 4096x2160, Aspect 256:135 */
    4096, 2160, 0, 24,
    4096, 2160, 0, 25, 
    4096, 2160, 0, 30,   /* 100 */ 
    4096, 2160, 0, 50,   /* 101, for YCbCr 4:2:0 */
    4096, 2160, 0, 60,   /* 102, for YCbCr 4:2:0 */
    /* 3840x2160, Aspect 64:27 */
    3840, 2160, 0, 24,
    3840, 2160, 0, 25,
    3840, 2160, 0, 30,
    3840, 2160, 0, 50,   /* 106, for YCbCr 4:2:0 */
    3840, 2160, 0, 60    /* 107, for YCbCr 4:2:0 */
                         /* 108 */
};

/*============================================================================
 * Read up to 8-bit field from a single 8-bit register
 *              ________
 * Example     |___***__|  Mask = 0x1C     BitPos = 2
 *
 *
 * Entry:   DevAddr = Device Address
 *          RegAddr = 8-bit register address
 *          Mask    = Field mask
 *          BitPos  = Field LSBit position in the register (0-7)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UCHAR ATV_I2CReadField8 (UCHAR DevAddr, UCHAR RegAddr, UCHAR Mask,
                         UCHAR BitPos)
{
    UCHAR Val;

    HAL_I2CReadByte (DevAddr, RegAddr, &Val);
    return ((Val & Mask) >> BitPos);
}

/*============================================================================
 * Write up to 8-bit field to a single 8-bit register
 *              ________
 * Example     |___****_|  Mask = 0x1E     BitPos = 1
 *
 * Entry:   DevAddr = Device Address
 *          RegAddr = 8-bit register address
 *          Mask    = Field mask
 *          BitPos  = Field LSBit position in the register (0-7)
 *                    Set to 0 if FieldVal is in correct position of the reg
 *          FieldVal= Value (in the LSBits) of the field to be written
 *                    If FieldVal is already in the correct position (i.e.,
 *                    does not need to be shifted,) set BitPos to 0
 *
 * Return:  None
 *
 *===========================================================================*/
void ATV_I2CWriteField8 (UCHAR DevAddr, UCHAR RegAddr, UCHAR Mask,
                         UCHAR BitPos, UCHAR FieldVal)
{
    UCHAR Val;

    HAL_I2CReadByte (DevAddr, RegAddr, &Val);
    Val = (Val & ~Mask) | ((FieldVal << BitPos) & Mask);
    HAL_I2CWriteByte (DevAddr, RegAddr, Val);
}

/*============================================================================
 * Write up to 8-bit field in a write-only 8-bit register
 *              ________
 * Example     |___****_|  Mask = 0x1E     BitPos = 1
 *
 * Entry:   DevAddr = Device Address
 *          RegAddr = 8-bit register address
 *          Mask    = Field mask
 *          BitPos  = Field LSBit position in the register (0-7)
 *                    Set to 0 if FieldVal is in correct position of the reg
 *          CurrVal = Pointer to current register value. This value will be
 *                    updated with the new filed value upon return
 *          FieldVal= Value (in the LSBits) of the field to be written
 *                    If FieldVal is already in the correct position (i.e.,
 *                    does not need to be shifted,) set BitPos to 0
 *
 * Return:  None
 *
 *===========================================================================*/
void ATV_I2CWriteOField8 (UCHAR DevAddr, UCHAR RegAddr, UCHAR Mask,
                          UCHAR BitPos, UCHAR *CurrVal, UCHAR FieldVal)
{
    *CurrVal = (*CurrVal & ~Mask) | ((FieldVal << BitPos) & Mask);
    HAL_I2CWriteByte (DevAddr, RegAddr, *CurrVal);
}

/*============================================================================
 * Read up to 32-bit field from two or more consecutive 8-bit registers in
 * Big Endian format
 *
 *                   ________
 * Start Reg Addr-> |     ***|  MSbits          MsbMask = 0x07
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 * End Reg Addr---> |**______|  LSbits          LsbMask = 0xC0, LsbPos = 6
 *                                              FldSpan = 5
 *
 * Entry:   DevAddr = Device Address
 *          RegAddr = Starting 8-bit register address
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *                    (Maximum 5 registers)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2CReadField32 (UCHAR DevAddr, UCHAR RegAddr, UCHAR MsbMask,
                           UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan)
{
    UCHAR i, j, Bytes[5];
    UINT32 Val=0;

    HAL_I2CReadBlock (DevAddr, RegAddr, Bytes, FldSpan);
    Bytes[0] &= MsbMask;
    FldSpan--;
    Bytes[FldSpan] &= LsbMask;

    j = (FldSpan << 3) - LsbPos;
    for (i=0; i<FldSpan; i++)
    {
        Val |= (((UINT32)(Bytes[i])) << j);
        j-= 8;
    }
    Val |= (UINT32) ((Bytes[FldSpan]) >> LsbPos);
    return (Val);
}

/*============================================================================
 * Read up to 32-bit field from two or more non-consecutive 8-bit registers
 * in Big Endian format
 *
 *                   ________
 * Start Reg Addr-> |  ******|  MSbits          MsbMask = 0x3F
 *                  .        .
 *                  |********|  Middle bits
 *                  .        .
 * End Reg Addr---> |*****___|  LSbits          LsbMask = 0xF8, LsbPos = 3
 *                                              FldSpan = 3
 *
 * Entry:   DevAddr = Device Address
 *          AddrTbl = Pointer to array of register addresses. The size of the
 *                    array is FldSpan. The first address in the array is of
 *                    the register that contain the MSBits of the field. The
 *                    last address in the array is of the register that contain
 *                    the LSBits of the field
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of non-consecutive registers containing field bits
 *                    (Maximum 5 registers)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2CReadFragField32 (UCHAR DevAddr, UCHAR *AddrTbl, UCHAR MsbMask,
                               UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan)
{
    UCHAR i, j, Bytes[5];
    UINT32 Val=0;

    ASSERT(sizeof(Bytes) >= FldSpan);
    memset(Bytes, 0, sizeof(Bytes));

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2CReadByte (DevAddr, AddrTbl[i], &(Bytes[i]));
    }
    Bytes[0] &= MsbMask;
    FldSpan--;
    Bytes[FldSpan] &= LsbMask;

    j = (FldSpan << 3) - LsbPos;
    for (i=0; i<FldSpan; i++)
    {
        Val |= (((UINT32)(Bytes[i])) << j);
        j-= 8;
    }
    Val |= (UINT32) ((Bytes[FldSpan]) >> LsbPos);
    return (Val);
}

/*============================================================================
 * Read up to 32-bit field from two or more consecutive 8-bit registers in
 * Little Endian format
 *
 *                   ________
 * Start Reg Addr-> |***     |  LSbits          LsbMask = 0xE0, LsbPos = 5
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 * End Reg Addr---> |______**|  MSbits          MsbMask = 0x03
 *                                              FldSpan = 5
 *
 * Entry:   DevAddr = Device Address
 *          RegAddr = Starting 8-bit register address
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *                    (Maximum 5 registers)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2CReadField32LE (UCHAR DevAddr, UCHAR RegAddr, UCHAR MsbMask,
                             UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan)
{
    UCHAR i, j, Bytes[5];
    UINT32 Val=0;

    HAL_I2CReadBlock (DevAddr, RegAddr, Bytes, FldSpan);
    FldSpan--;
    Bytes[FldSpan] &= MsbMask;

    Val = (Bytes[0] & LsbMask) >> LsbPos;
    j = 8 - LsbPos;
    for (i=1; i<=FldSpan; i++)
    {
        Val |= (((UINT32)(Bytes[i])) << j);
        j+= 8;
    }
    return (Val);
}

/*============================================================================
 * Read up to 32-bit field from two or more non-consecutive 8-bit registers
 * in Big Endian format
 *
 *                   ________
 * Start Reg Addr-> |*****   |  LSbits          LsbMask = 0xF8, LsbPos = 3
 *                  .        .
 *                  |********|  Middle bits
 *                  .        .
 * End Reg Addr---> |__******|  MSbits          MsbMask = 0x3F
 *                                              FldSpan = 3
 *
 * Entry:   DevAddr = Device Address
 *          AddrTbl = Pointer to array of register addresses. The size of the
 *                    array is FldSpan. The first address in the array is of
 *                    the register that contain the LSBits of the field. The
 *                    last address in the array is of the register that contain
 *                    the MSBits of the field
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of non-consecutive registers containing field bits
 *                    (Maximum 5 registers)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2CReadFragField32LE (UCHAR DevAddr, UCHAR *AddrTbl, UCHAR MsbMask,
                                 UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan)
{
    UCHAR i, j, Bytes[5];
    UINT32 Val=0;

    ASSERT(sizeof(Bytes) >= FldSpan);
    memset(Bytes, 0, sizeof(Bytes));

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2CReadByte (DevAddr, AddrTbl[i], &(Bytes[i]));
    }
    FldSpan--;
    Bytes[FldSpan] &= MsbMask;

    Val = (Bytes[0] & LsbMask) >> LsbPos;
    j = 8 - LsbPos;
    for (i=1; i<=FldSpan; i++)
    {
        Val |= (((UINT32)(Bytes[i])) << j);
        j+= 8;
    }
    return (Val);
}

/*============================================================================
 * Write up to 32-bit field to two or more consecutive 8-bit registers in
 * Big Endian format
 *                   ________
 * Start Reg Addr-> |   *****|  MSbits          MsbMask = 0x1F
 *                  |********|  Middle bits
 * End Reg Addr---> |******__|  LSbits          LsbMask = 0xFC, LsbPos = 2
 *                                              FldSpan = 3
 * Entry:   DevAddr = Device Address
 *          RegAddr = Starting 8-bit register address
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *
 *===========================================================================*/
void ATV_I2CWriteField32 (UCHAR DevAddr, UCHAR RegAddr, UCHAR MsbMask,
                     UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val)
{
    UCHAR Bytes[5];
    UCHAR i;

    HAL_I2CReadBlock (DevAddr, RegAddr, Bytes, FldSpan);
    /*=======================
     * MSBits
     *======================*/
    Bytes[0] = (Bytes[0] & (~MsbMask)) |
                       ((Val >> (8 * (FldSpan - 1) - LsbPos)) & MsbMask);
    for (i=1; i<FldSpan-1; i++)
    {
        Bytes[i] = Val >> (8 * (FldSpan - i - 1) - LsbPos);
    }
    /*=======================
     * LSBits
     *======================*/
    Bytes[FldSpan-1] = (Bytes[FldSpan-1] & ~LsbMask) |
                       (UCHAR) ((Val << LsbPos) & LsbMask);
    HAL_I2CWriteBlock (DevAddr, RegAddr, Bytes, FldSpan);
}

/*============================================================================
 * Write up to 32-bit field to two or more consecutive 8-bit registers in
 * Little Endian format
 *                   ________
 * Start Reg Addr-> |*       |  LSbits          LsbMask = 0x80, LsbPos = 7
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 * End Reg Addr---> |********|  MSbits          MsbMask = 0xFF
 *                                              FldSpan = 4
 * Entry:   DevAddr = Device Address
 *          RegAddr = Starting 8-bit register address
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *
 *===========================================================================*/
void ATV_I2CWriteField32LE (UCHAR DevAddr, UCHAR RegAddr, UCHAR MsbMask,
                     UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val)
{
    UCHAR Bytes[5];
    UCHAR i;

    HAL_I2CReadBlock (DevAddr, RegAddr, Bytes, FldSpan);
    /*===================
     * LS Byte
     *==================*/
    Bytes[0] = (Bytes[0] & ~LsbMask) | (UCHAR) ((Val << LsbPos) & LsbMask);
    for (i=1; i<FldSpan-1; i++)
    {
        Bytes[i] = Val >> (8 * (FldSpan - i - 1) - LsbPos);
    }
    /*===================
     * MS Byte
     *==================*/
    Bytes[FldSpan-1] = (Bytes[FldSpan-1] & (~MsbMask)) |
                       ((Val >> (8 * (FldSpan - 1) - LsbPos)) & MsbMask);
    HAL_I2CWriteBlock (DevAddr, RegAddr, Bytes, FldSpan);
}

/*============================================================================
 * Write up to 32-bit field to two or more non-consecutive 8-bit registers in
 * Big Endian format
 *                   ________
 * Start Reg Addr-> |   *****|  MSbits          MsbMask = 0x1F
 *                  .        .
 * End Reg Addr---> |******__|  LSbits          LsbMask = 0xFC, LsbPos = 2
 *                                              FldSpan = 2
 * Entry:   DevAddr = Device Address
 *          AddrTbl = Pointer to array of register addresses. The size of the
 *                    array is FldSpan. The first address in the array is of
 *                    the register that contain the MSBits of the field. The
 *                    last address in the array is of the register that contain
 *                    the LSBits of the field
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *
 *===========================================================================*/
void ATV_I2CWriteFragField32 (UCHAR DevAddr, UCHAR *AddrTbl, UCHAR MsbMask,
                     UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val)
{
    UCHAR Bytes[5];
    UCHAR i;

    ASSERT(sizeof(Bytes) >= FldSpan);
    memset(Bytes, 0, sizeof(Bytes));

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2CReadByte (DevAddr, AddrTbl[i], &(Bytes[i]));
    }
    /*===================
     * MS Byte
     *==================*/
    Bytes[0] = (Bytes[0] & (~MsbMask)) |
                       ((Val >> (8 * (FldSpan - 1) - LsbPos)) & MsbMask);
    for (i=1; i<FldSpan-1; i++)
    {
        Bytes[i] = Val >> (8 * (FldSpan - i - 1) - LsbPos);
    }
    /*===================
     * LS Byte
     *==================*/
    Bytes[FldSpan-1] = (Bytes[FldSpan-1] & ~LsbMask) |
                       (UCHAR) ((Val << LsbPos) & LsbMask);
    for (i=0; i<FldSpan; i++)
    {
        HAL_I2CWriteByte (DevAddr, AddrTbl[i], Bytes[i]);
    }
}

/*============================================================================
 * Write up to 32-bit field to two or more consecutive 8-bit registers in
 * Little Endian format
 *                   ________
 * Start Reg Addr-> |*****   |  LSbits          LsbMask = 0xF8, LsbPos = 3
 *                  .        .
 * End Reg Addr---> |__******|  MSbits          MsbMask = 0x3F
 *                                              FldSpan = 2
 * Entry:   DevAddr = Device Address
 *          AddrTbl = Pointer to array of register addresses. The size of the
 *                    array is FldSpan. The first address in the array is of
 *                    the register that contain the LSBits of the field. The
 *                    last address in the array is of the register that contain
 *                    the MSBits of the field
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *
 *===========================================================================*/
void ATV_I2CWriteFragField32LE (UCHAR DevAddr, UCHAR *AddrTbl, UCHAR MsbMask,
                     UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val)
{
    UCHAR Bytes[5];
    UCHAR i;

    ASSERT(sizeof(Bytes) >= FldSpan);
    memset(Bytes, 0, sizeof(Bytes));

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2CReadByte (DevAddr, AddrTbl[i], &(Bytes[i]));
    }
    /*===================
     * LS Byte
     *==================*/
    Bytes[0] = (Bytes[0] & ~LsbMask) | (UCHAR) ((Val << LsbPos) & LsbMask);
    for (i=1; i<FldSpan-1; i++)
    {
        Bytes[i] = Val >> (8 * (FldSpan - i - 1) - LsbPos);
    }
    /*===================
     * MS Byte
     *==================*/
    Bytes[FldSpan-1] = (Bytes[FldSpan-1] & (~MsbMask)) |
                       ((Val >> (8 * (FldSpan - 1) - LsbPos)) & MsbMask);
    for (i=0; i<FldSpan; i++)
    {
        HAL_I2CWriteByte (DevAddr, AddrTbl[i], Bytes[i]);
    }
}

/*============================================================================
 * Read up to 32-bit field from one or more non-consecutive 8-bit registers
 * in Big or Little Endian format
 *
 *           ________       Mask    LShift(LE)  LShift(BE)
 * Reg 0    | ****** |      0x7E   -1           3
 *          .        .
 * Reg 1    |   ***  |      0x1C    4          -1
 *          .        .
 * Reg 2    |____*___|      0x08    6          -3
 *
 *
 * Entry:   DevAddr = Device Address
 *          FldInfo = Pointer to array of field-inofrmation structures
 *                    The LShift member of the structure is the absolute left
 *                    shift of each register field bits (in 2's complement) as
 *                    illustrated in the above example for both Little and Big
 *                    Edndian cases. An absolute right-shift is indicated by
 *                    a negative value for LShift.
 *          FldSpan = Number of entries in FldInfo
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2CReadRandField32 (UCHAR DevAddr, I2C_FIELD_INFO *FldInfo,
                               UCHAR FldSpan)
{
    UCHAR i, ByteX;
    UINT32 Val=0;

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2CReadByte (DevAddr, FldInfo[i].RegAddr, &ByteX);
        ByteX &= FldInfo[i].Mask;
        if (FldInfo[i].LShift & 0x80)
        {
            Val |= (((UINT32)ByteX) >> (0 - FldInfo[i].LShift));
        }
        else
        {
            Val |= (((UINT32)ByteX) << FldInfo[i].LShift);
        }
    }
    return (Val);
}

/*============================================================================
 * Write up to 32-bit field to one or more non-consecutive 8-bit registers
 * in Big or Little Endian format
 *
 *           ________       Mask    LShift(LE)  LShift(BE)
 * Reg 0    | ****** |      0x7E   -1           3
 *          .        .
 * Reg 1    |   ***  |      0x1C    4          -1
 *          .        .
 * Reg 2    |____*___|      0x08    6          -3
 *
 *
 * Entry:   DevAddr = Device Address
 *          FldInfo = Pointer to array of field-inofrmation structures
 *                    The LShift member of the structure is the absolute left
 *                    shift of each register field bits (in 2's complement) as
 *                    illustrated in the above example for both Little and Big
 *                    Edndian cases. An absolute right-shift is indicated by
 *                    a negative value for LShift.
 *          FldSpan = Number of entries in FldInfo
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *===========================================================================*/
void ATV_I2CWriteRandField32 (UCHAR DevAddr, I2C_FIELD_INFO *FldInfo,
                              UCHAR FldSpan, UINT32 Val)
{
    UCHAR i, ByteX;

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2CReadByte (DevAddr, FldInfo[i].RegAddr, &ByteX);
        ByteX &= ~(FldInfo[i].Mask);
        if (FldInfo[i].LShift & 0x80)
        {
            ByteX |= ((UCHAR)(Val << (0-FldInfo[i].LShift)) & FldInfo[i].Mask);
        }
        else
        {
            ByteX |= ((UCHAR)(Val >> FldInfo[i].LShift) & FldInfo[i].Mask);
        }
        HAL_I2CWriteByte (DevAddr, FldInfo[i].RegAddr, ByteX);
    }
}

/*===========================================================================
 * Return the value of i2c register.
 * This function is intended for debugging purposes.
 *
 * Entry:   Dev = Device address
 *          Reg = Required register
 *
 * Return:  Register value
 *
 *==========================================================================*/
UCHAR ATV_Reg (UCHAR Dev, UCHAR Reg)
{
    UCHAR ByteX;
    HAL_I2CReadByte(Dev, Reg, &ByteX);
    return(ByteX);
}

/*===========================================================================
 * Max delay 65 seconds
 *
 * Entry:   Counter = Required delay in seconds
 *
 * Return:  This function returns after the requested delay time has elapsed
 *
 *==========================================================================*/
void ATV_DelaySec (UINT16 Counter)
{
    HAL_DelayMs(Counter*1000);
}

/*===========================================================================
 * Get the elapsed time in milliseconds from a defined starting point
 *
 * Entry:   StartCount  = Time in milliseconds at the sarting point
 *          CurrMsCount = Pointer to receive the current time in milliseconds
 *                        Can be set to NULL if not needed
 *
 * Return:  Time (in milliseconds) that elapsed since StartCount.
 *          This function can not return elapsed time over 65 seconds
 *
 *==========================================================================*/
UINT32 ATV_GetElapsedMs (UINT32 StartCount, UINT32 *CurrMsCount)
{
    UINT32 Elapsed;
    UINT32 CurrCount;

    CurrCount = HAL_GetCurrentMsCount();
    if (CurrCount >= StartCount)
    {
        Elapsed = CurrCount - StartCount;
    }
    else
    {
        Elapsed = 0xFFFFFFFFL - (StartCount - CurrCount);
    }

    if (CurrMsCount)
    {
        *CurrMsCount = CurrCount;
    }
    return (Elapsed);
}

/*===========================================================================
 * Return the current time in milliseconds.
 * If the current time is 0, return 0xFFFFFFFF
 *
 *
 *==========================================================================*/
UINT32 ATV_GetMsCountNZ (void)
{
    UINT32 i;
    i = HAL_GetCurrentMsCount();
    if (i == 0)
    {
        i = 0xFFFFFFFFL;
    }
    return(i);
}

/*===========================================================================
 * Look for UCHAR Value in Table with Step increments
 *
 * Return:  Offset of matching value or end value
 *
 *==========================================================================*/
UINT16 ATV_LookupValue8 (UCHAR *Table, UCHAR Value, UCHAR EndVal, UINT16 Step)
{
    UINT16 i=0;

    while (Table[i] != EndVal)
    {
        if (Table[i] == Value)
        {
            break;
        }
        i+= Step;
    }
    return(i);
}

/*===========================================================================
 * Look for StrgId and return a pointer to the matching string
 *
 * Return:  Pointer to matching string
 *
 *==========================================================================*/
char *ATV_LookupStrgTable (STRG_TABLE *Table, UCHAR StrgId, UCHAR EndVal)
{
    UINT16 i=0;

    while (Table[i].StrgId != EndVal)
    {
        if (Table[i].StrgId == StrgId)
        {
            return (Table[i].Strg);
        }
        i++;
    }
    return (Table[i].Strg);
}

/*===========================================================================
 * Print current system time
 *
 * Entry:   Gran    = 0  for ms
 *                    1  for sec:ms
 *                    2  for min:sec:ms
 *          PostFix = Postfix string
 *
 *==========================================================================*/
void ATV_PrintTime (char *Prefix, UCHAR Gran, char *Postfix)
{
    UINT32 i, min, sec, ms;

    i = HAL_GetCurrentMsCount() % 3600000;   /* i = one hour max */
    ms = i % 1000;
    sec = (i / 1000) % 60;
    min = (i / 1000) / 60;

    if (Gran == 2)
    {
        DBG_MSG("%s%2d:%02d:%03d%s", Prefix, min, sec, ms, Postfix);
    }
    else if (Gran == 1)
    {
        DBG_MSG("%s%02d:%03d%s", Prefix, sec, ms, Postfix);
    }
    else
    {
        DBG_MSG("%s%03d%s", Prefix, ms, Postfix);
    }
}

/*===========================================================================
 * Perform multiple I2C register writes from a user-supplied Table
 *
 * Entry:   Table : A pointer to the table of writes that must be performed
 *                  Table[] = {
 *                              DevAddr0, RegOffset0, Value0,
 *                              DevAddr1, RegOffset1, Value1,
 *                              DevAddr2, RegOffset2, Value2,
 *                              ...
 *                              DevAddrN, RegOffsetN, ValueN,
 *                              EndVal
 *                             }
 *          EndVal : The last value in the table
 *
 * Return:  None
 *
 *==========================================================================*/
void ATV_I2CWriteTable (UCHAR *Table, UCHAR EndVal)
{
    UINT16 i=0;

    while (Table[i]!= EndVal)
    {
        HAL_I2CWriteByte (Table[i], Table[i+1], Table[i+2]);
        i+=3;
    }
}


/*===========================================================================
 * Perform multiple I2C register (16-bit) writes from a user-supplied Table
 *
 * Entry:   Table : A pointer to the table of writes that must be performed
 *                  Table[] = {
 *                              DevAddr0, segAddr0, RegOffset0, Value0,
 *                              DevAddr1, segAddr1, RegOffset1, Value1,
 *                              DevAddr2, segAddr2, RegOffset2, Value2,
 *                              ...
 *                              DevAddrN, segAddrN, RegOffsetN, ValueN,
 *                              EndVal
 *                             }
 *          EndVal : The last value in the table
 *
 * Return:  None
 *
 *==========================================================================*/
void ATV_I2C16WriteTable (UCHAR *Table, UCHAR EndVal)
{
    UCHAR i=0, data;
    UINT16 subaddr;

    while (Table[i]!= EndVal)
    {
        subaddr = (Table[i+1]<<8) + Table[i+2];
        data = Table[i+3];
        HAL_I2C16WriteBlock8 (Table[i], subaddr, &data, 1);
        i+=4;
    }
}



/*===========================================================================
 * Modify multiple registers fields from a user-supllied table
 *
 * Entry:   Table : A pointer to the table of writes that must be performed
 *                  Table[] = {
 *                              DevAddr0, RegOffset0, Mask0, Value0,
 *                              DevAddr1, RegOffset1, Mask1, Value1,
 *                              DevAddr2, RegOffset2, Mask2, Value2,
 *                              ...
 *                              DevAddrN, RegOffsetN, MaskN, ValueN,
 *                              EndVal
 *                             }
 *          EndVal : The last value in the table
 *
 * Return:  None
 *
 *==========================================================================*/
void ATV_I2CWriteFields (UCHAR *Table, UCHAR EndVal)
{
    UINT16 i=0;

    while (Table[i]!= EndVal)
    {
        ATV_I2CWriteField8(Table[i], Table[i+1], Table[i+2], 0, Table[i+3]);
        i+= 4;
    }
}

/*===========================================================================
 * Check multiple time out values and sets a flag for each if time-out
 * expires.
 *
 * Entry:   Table: Pointer to a table of time-out structures
 *                  .StartTime = Time at start of event
 *                               Will be set to 0 when threshold expires
 *                               Must be set to NZ value to start a time-out
 *                  .Threshold = Time-out value from event start
 *                  .TimedOut  = Will be set to TRUE if Threshold expired
 *                               Will be set to FALSE if not expired
 *
 * Return:  None
 *
 *==========================================================================*/
void ATV_CheckTimeOut (TIMEOUT_TABLE *Table)
{
    UINT32 i=0, Elapsed, CurrCount;

    CurrCount = HAL_GetCurrentMsCount();
    while (Table[i].Threshold != 0xFFFF)
    {
        if (Table[i].StartTime)
        {
            if (CurrCount >= Table[i].StartTime)
            {
                Elapsed = CurrCount - Table[i].StartTime;
            }
            else
            {
                Elapsed = 0xFFFFFFFFL - (Table[i].StartTime - CurrCount);
            }
            if (Elapsed > Table[i].Threshold)
            {
                Table[i].TimedOut = TRUE;
                Table[i].StartTime = 0;
            }
            else
            {
                Table[i].TimedOut = FALSE;
            }
        }
        i++;
    }
}

/*===========================================================================
 * Look for the expected size of a CEC message
 *
 * Entry:   Table : A pointer to the table of of CEC opcode and corresponding
 *                  message size
 *                  Table[] = {
 *                              Opcode0, MsgSize0,
 *                              Opcode1, MsgSize1,
 *                              Opcode2, MsgSize2,
 *                              ...
 *                              OpcodeN, MsgSizeN
 *                             }
 *          TableSize : The total number of elements in the table.
 *                      This number is (N+1)*2 in the example above.
 *          Opcode : The value of the opcode for which the expected message
 *                   size mut be found
 *          MsgSize : The size of the CEC message for which the expected size
 *                    must be found
 *          ExpMsgSize : A pointer to the expected size of CEC message
 *                       returned by this function. Set to NULL if this
 *                       information is not required
 *
 * Return:  ATVERR_TRUE  : The size of the CEC message matches the expected
 *                         size for this message
 *          ATVERR_FALSE : The size of the CEC message does not match the
 *                         expected size for this message
 *
 *==========================================================================*/
ATV_ERR ATV_CecValidMsgSize (UCHAR *Table, UCHAR TableSize, UCHAR Opcode,
                             UCHAR MsgSize, UCHAR *ExpMsgSize)
{
    UCHAR i;
    ATV_ERR RetVal = ATVERR_TRUE;

    for (i=0; i<TableSize; i+=2)
    {
        if (Opcode == Table[i])
        {
            if (MsgSize != Table[i+1])
            {
                if(ExpMsgSize)
                {
                    *ExpMsgSize = Table[i+1];
                }
                RetVal = ATVERR_FALSE;
            }
            break;
        }
    }
    return (RetVal);
}

/*============================================================================
 * Read up to 8-bit field from a single 8-bit register of a 16-bit register
 * addressable device
 *              ________
 * Example     |___***__|  Mask = 0x1C     BitPos = 2
 *
 *
 * Entry:   DevAddr = 8-bit Device Address
 *          RegAddr = 16-bit register address
 *          Mask    = Field mask
 *          BitPos  = Field LSBit position in the register (0-7)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UCHAR ATV_I2C16ReadField8 (UCHAR DevAddr, UINT16 RegAddr, UCHAR Mask,
                           UCHAR BitPos)
{
    UCHAR Val;

    HAL_I2C16ReadBlock8 (DevAddr, RegAddr, &Val, 1);
    return ((Val & Mask) >> BitPos);
}

/*============================================================================
 * Write up to 8-bit field to a single 8-bit register of a 16-bit register
 * addressable device
 *              ________
 * Example     |___****_|  Mask = 0x1E     BitPos = 1
 *
 * Entry:   DevAddr = 8-bit Device Address
 *          RegAddr = 16-bit register address
 *          Mask    = Field mask
 *          BitPos  = Field LSBit position in the register (0-7)
 *                    Set to 0 if FieldVal is in correct position of the reg
 *          FieldVal= Value (in the LSBits) of the field to be written
 *                    If FieldVal is already in the correct position (i.e.,
 *                    does not need to be shifted,) set BitPos to 0
 *
 * Return:  None
 *
 *===========================================================================*/
void ATV_I2C16WriteField8 (UCHAR DevAddr, UINT16 RegAddr, UCHAR Mask,
                           UCHAR BitPos, UCHAR FieldVal)
{
    UCHAR Val;

    HAL_I2C16ReadBlock8 (DevAddr, RegAddr, &Val, 1);
    Val = (Val & ~Mask) | ((FieldVal << BitPos) & Mask);
    HAL_I2C16WriteBlock8 (DevAddr, RegAddr, &Val, 1);
}

/*============================================================================
 * Write up to 8-bit field in a write-only 8-bit register of a 16-bit register
 * addressable device
 *              ________
 * Example     |___****_|  Mask = 0x1E     BitPos = 1
 *
 * Entry:   DevAddr = 8-bit Device Address
 *          RegAddr = 16-bit register address
 *          Mask    = Field mask
 *          BitPos  = Field LSBit position in the register (0-7)
 *                    Set to 0 if FieldVal is in correct position of the reg
 *          CurrVal = Pointer to current register value. This value will be
 *                    updated with the new filed value upon return
 *          FieldVal= Value (in the LSBits) of the field to be written
 *                    If FieldVal is already in the correct position (i.e.,
 *                    does not need to be shifted,) set BitPos to 0
 *
 * Return:  None
 *
 *===========================================================================*/
void ATV_I2C16WriteOField8 (UCHAR DevAddr, UINT16 RegAddr, UCHAR Mask,
                            UCHAR BitPos, UCHAR *CurrVal, UCHAR FieldVal)
{
    *CurrVal = (*CurrVal & ~Mask) | ((FieldVal << BitPos) & Mask);
    HAL_I2C16WriteBlock8 (DevAddr, RegAddr, CurrVal, 1);
}

/*============================================================================
 * Read up to 32-bit field from two or more consecutive 8-bit registers in
 * Big Endian format. The registers from the I2C devices are 16-bit
 * adressable.
 *                   ________
 * Start Reg Addr-> |     ***|  MSbits          MsbMask = 0x07
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 * End Reg Addr---> |**______|  LSbits          LsbMask = 0xC0, LsbPos = 6
 *                                              FldSpan = 5
 *
 * Entry:   DevAddr = 8-bit evice Address
 *          RegAddr = Starting 16-bit register address
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *                    (Maximum 5 registers)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2C16ReadField32 (UCHAR DevAddr, UINT16 RegAddr, UCHAR MsbMask,
                             UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan)
{
    UCHAR i, j, Bytes[5];
    UINT32 Val=0;

    HAL_I2C16ReadBlock8 (DevAddr, RegAddr, Bytes, FldSpan);
    Bytes[0] &= MsbMask;
    FldSpan--;
    Bytes[FldSpan] &= LsbMask;

    j = (FldSpan << 3) - LsbPos;
    for (i=0; i<FldSpan; i++)
    {
        Val |= (((UINT32)(Bytes[i])) << j);
        j-= 8;
    }
    Val |= (UINT32) ((Bytes[FldSpan]) >> LsbPos);
    return (Val);
}

/*============================================================================
 * Read up to 32-bit field from two or more non-consecutive 8-bit registers
 * in Big Endian format. The registers from the I2C devices are 16-bit
 * adressable.
 *                   ________
 * Start Reg Addr-> |  ******|  MSbits          MsbMask = 0x3F
 *                  .        .
 *                  |********|  Middle bits
 *                  .        .
 * End Reg Addr---> |*****___|  LSbits          LsbMask = 0xF8, LsbPos = 3
 *                                              FldSpan = 3
 *
 * Entry:   DevAddr = 8-bit Device Address
 *          AddrTbl = Pointer to array of 16-bit register addresses. The size of the
 *                    array is FldSpan. The first address in the array is of
 *                    the register that contain the MSBits of the field. The
 *                    last address in the array is of the register that contain
 *                    the LSBits of the field
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of non-consecutive registers containing field bits
 *                    (Maximum 5 registers)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2C16ReadFragField32 (UCHAR DevAddr, UINT16 *AddrTbl, UCHAR MsbMask,
                                 UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan)
{
    UCHAR i, j, Bytes[5];
    UINT32 Val=0;

    ASSERT(sizeof(Bytes) >= FldSpan);
    memset(Bytes, 0, sizeof(Bytes));

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2C16ReadBlock8 (DevAddr, AddrTbl[i], &(Bytes[i]), 1);
    }
    Bytes[0] &= MsbMask;
    FldSpan--;
    Bytes[FldSpan] &= LsbMask;

    j = (FldSpan << 3) - LsbPos;
    for (i=0; i<FldSpan; i++)
    {
        Val |= (((UINT32)(Bytes[i])) << j);
        j-= 8;
    }
    Val |= (UINT32) ((Bytes[FldSpan]) >> LsbPos);
    return (Val);
}

/*============================================================================
 * Read up to 32-bit field from two or more consecutive 8-bit registers in
 * Little Endian format. The registers from the I2C devices are 16-bit
 * adressable.
 *                   ________
 * Start Reg Addr-> |***     |  LSbits          LsbMask = 0xE0, LsbPos = 5
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 * End Reg Addr---> |______**|  MSbits          MsbMask = 0x03
 *                                              FldSpan = 5
 *
 * Entry:   DevAddr = 8-bit device Address
 *          RegAddr = Starting 16-bit register address
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *                    (Maximum 5 registers)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2C16ReadField32LE (UCHAR DevAddr, UINT16 RegAddr, UCHAR MsbMask,
                               UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan)
{
    UCHAR i, j, Bytes[5];
    UINT32 Val=0;

    HAL_I2C16ReadBlock8 (DevAddr, RegAddr, Bytes, FldSpan);
    FldSpan--;
    Bytes[FldSpan] &= MsbMask;

    Val = (Bytes[0] & LsbMask) >> LsbPos;
    j = 8 - LsbPos;
    for (i=1; i<=FldSpan; i++)
    {
        Val |= (((UINT32)(Bytes[i])) << j);
        j+= 8;
    }
    return (Val);
}

/*============================================================================
 * Read up to 32-bit field from two or more non-consecutive 8-bit registers
 * in Big Endian format. The registers from the I2C devices are 16-bit
 * adressable.
 *                   ________
 * Start Reg Addr-> |*****   |  LSbits          LsbMask = 0xF8, LsbPos = 3
 *                  .        .
 *                  |********|  Middle bits
 *                  .        .
 * End Reg Addr---> |__******|  MSbits          MsbMask = 0x3F
 *                                              FldSpan = 3
 *
 * Entry:   DevAddr = Device Address
 *          AddrTbl = Pointer to array of register addresses. The size of the
 *                    array is FldSpan. The first address in the array is of
 *                    the register that contain the LSBits of the field. The
 *                    last address in the array is of the register that contain
 *                    the MSBits of the field
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of non-consecutive registers containing field bits
 *                    (Maximum 5 registers)
 *
 * Return:  Field value in the LSBits of the return value
 *
 *===========================================================================*/
UINT32 ATV_I2C16ReadFragField32LE (UCHAR DevAddr, UINT16 *AddrTbl, UCHAR MsbMask,
                                   UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan)
{
    UCHAR i, j, Bytes[5];
    UINT32 Val=0;

    ASSERT(sizeof(Bytes) >= FldSpan);
    memset(Bytes, 0, sizeof(Bytes));

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2C16ReadBlock8 (DevAddr, AddrTbl[i], &(Bytes[i]), 1);
    }
    FldSpan--;
    Bytes[FldSpan] &= MsbMask;

    Val = (Bytes[0] & LsbMask) >> LsbPos;
    j = 8 - LsbPos;
    for (i=1; i<=FldSpan; i++)
    {
        Val |= (((UINT32)(Bytes[i])) << j);
        j+= 8;
    }
    return (Val);
}

/*============================================================================
 * Write up to 32-bit field to two or more consecutive 8-bit registers in
 * Big Endian format. The registers from the I2C devices are 16-bit
 * adressable.
 *                    ________
 * Start Reg Addr-> |   *****|  MSbits          MsbMask = 0x1F
 *                  |********|  Middle bits
 * End Reg Addr---> |******__|  LSbits          LsbMask = 0xFC, LsbPos = 2
 *                                              FldSpan = 3
 * Entry:   DevAddr = 8-bit Device Address
 *          RegAddr = Starting 16-bit register address
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *
 *===========================================================================*/
void ATV_I2C16WriteField32 (UCHAR DevAddr, UINT16 RegAddr, UCHAR MsbMask,
                   UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val)
{
    UCHAR Bytes[5];
    UCHAR i;

    HAL_I2C16ReadBlock8 (DevAddr, RegAddr, Bytes, FldSpan);
    /*=======================
     * MSBits
     *======================*/
    Bytes[0] = (Bytes[0] & (~MsbMask)) |
                       ((Val >> (8 * (FldSpan - 1) - LsbPos)) & MsbMask);
    for (i=1; i<FldSpan-1; i++)
    {
        Bytes[i] = Val >> (8 * (FldSpan - i - 1) - LsbPos);
    }
    /*=======================
     * LSBits
     *======================*/
    Bytes[FldSpan-1] = (Bytes[FldSpan-1] & ~LsbMask) |
                       (UCHAR) ((Val << LsbPos) & LsbMask);
    HAL_I2C16WriteBlock8 (DevAddr, RegAddr, Bytes, FldSpan);
}

/*============================================================================
 * Write up to 32-bit field to two or more consecutive 8-bit registers in
 * Little Endian format. The registers from the I2C devices are 16-bit
 * adressable.
 *                   ________
 * Start Reg Addr-> |*       |  LSbits          LsbMask = 0x80, LsbPos = 7
 *                  |********|  Middle bits
 *                  |********|  Middle bits
 * End Reg Addr---> |********|  MSbits          MsbMask = 0xFF
 *                                              FldSpan = 4
 * Entry:   DevAddr = 8-bit Device Address
 *          RegAddr = Starting 16-bit register address
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *
 *===========================================================================*/
void ATV_I2C16WriteField32LE (UCHAR DevAddr, UINT16 RegAddr, UCHAR MsbMask,
                     UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val)
{
    UCHAR Bytes[5];
    UCHAR i;

    HAL_I2C16ReadBlock8 (DevAddr, RegAddr, Bytes, FldSpan);
    /*===================
     * LS Byte
     *==================*/
    Bytes[0] = (Bytes[0] & ~LsbMask) | (UCHAR) ((Val << LsbPos) & LsbMask);
    for (i=1; i<FldSpan-1; i++)
    {
        Bytes[i] = Val >> (8 * (FldSpan - i - 1) - LsbPos);
    }
    /*===================
     * MS Byte
     *==================*/
    Bytes[FldSpan-1] = (Bytes[FldSpan-1] & (~MsbMask)) |
                       ((Val >> (8 * (FldSpan - 1) - LsbPos)) & MsbMask);
    HAL_I2C16WriteBlock8 (DevAddr, RegAddr, Bytes, FldSpan);
}

/*============================================================================
 * Write up to 32-bit field to two or more non-consecutive 8-bit registers in
 * Big Endian format. The registers from the I2C devices are 16-bit
 * adressable.
 *                   ________
 * Start Reg Addr-> |   *****|  MSbits          MsbMask = 0x1F
 *                  .        .
 * End Reg Addr---> |******__|  LSbits          LsbMask = 0xFC, LsbPos = 2
 *                                              FldSpan = 2
 * Entry:   DevAddr = Device Address
 *          AddrTbl = Pointer to array of register addresses. The size of the
 *                    array is FldSpan. The first address in the array is of
 *                    the register that contain the MSBits of the field. The
 *                    last address in the array is of the register that contain
 *                    the LSBits of the field
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *
 *===========================================================================*/
void ATV_I2C16WriteFragField32 (UCHAR DevAddr, UINT16 *AddrTbl, UCHAR MsbMask,
                       UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val)
{
    UCHAR Bytes[5];
    UCHAR i;

    ASSERT(sizeof(Bytes) >= FldSpan);
    memset(Bytes, 0, sizeof(Bytes));

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2C16ReadBlock8 (DevAddr, AddrTbl[i], &(Bytes[i]), 1);
    }
    /*===================
     * MS Byte
     *==================*/
    Bytes[0] = (Bytes[0] & (~MsbMask)) |
                       ((Val >> (8 * (FldSpan - 1) - LsbPos)) & MsbMask);
    for (i=1; i<FldSpan-1; i++)
    {
        Bytes[i] = Val >> (8 * (FldSpan - i - 1) - LsbPos);
    }
    /*===================
     * LS Byte
     *==================*/
    Bytes[FldSpan-1] = (Bytes[FldSpan-1] & ~LsbMask) |
                       (UCHAR) ((Val << LsbPos) & LsbMask);
    for (i=0; i<FldSpan; i++)
    {
        HAL_I2C16WriteBlock8 (DevAddr, AddrTbl[i], &Bytes[i], 1);
    }
}

/*============================================================================
 * Write up to 32-bit field to two or more consecutive 8-bit registers in
 * Little Endian format. The registers from the I2C devices are 16-bit
 * adressable.
 *                   ________
 * Start Reg Addr-> |*****   |  LSbits          LsbMask = 0xF8, LsbPos = 3
 *                  .        .
 * End Reg Addr---> |__******|  MSbits          MsbMask = 0x3F
 *                                              FldSpan = 2
 * Entry:   DevAddr = Device Address
 *          AddrTbl = Pointer to array of register addresses. The size of the
 *                    array is FldSpan. The first address in the array is of
 *                    the register that contain the LSBits of the field. The
 *                    last address in the array is of the register that contain
 *                    the MSBits of the field
 *          MsbMask = Mask for MSbits
 *          LsbMask = Mask for LSbits
 *          LsbPos  = LSbit position (0-7)
 *          FldSpan = Number of consecutive registers containing field bits
 *          Val     = Field value (in the LSBits) to be written
 *
 * Return:  None
 *
 *
 *===========================================================================*/
void ATV_I2C16WriteFragField32LE (UCHAR DevAddr, UINT16 *AddrTbl, UCHAR MsbMask,
                        UCHAR LsbMask, UCHAR LsbPos, UCHAR FldSpan, UINT32 Val)
{
    UCHAR Bytes[5];
    UCHAR i;

    ASSERT(sizeof(Bytes) >= FldSpan);
    memset(Bytes, 0, sizeof(Bytes));

    for (i=0; i<FldSpan; i++)
    {
        HAL_I2C16ReadBlock8 (DevAddr, AddrTbl[i], &(Bytes[i]), 1);
    }
    /*===================
     * LS Byte
     *==================*/
    Bytes[0] = (Bytes[0] & ~LsbMask) | (UCHAR) ((Val << LsbPos) & LsbMask);
    for (i=1; i<FldSpan-1; i++)
    {
        Bytes[i] = Val >> (8 * (FldSpan - i - 1) - LsbPos);
    }
    /*===================
     * MS Byte
     *==================*/
    Bytes[FldSpan-1] = (Bytes[FldSpan-1] & (~MsbMask)) |
                       ((Val >> (8 * (FldSpan - 1) - LsbPos)) & MsbMask);
    for (i=0; i<FldSpan; i++)
    {
        HAL_I2C16WriteBlock8 (DevAddr, AddrTbl[i], &Bytes[i], 1);
    }
}

/*============================================================================
 * Return pointer to application revision info to track builds.
 *
 * Entry:   void
 *
 * Return:  Pointer to constant ATV_APP_REVISION_INFO
 *===========================================================================*/
CONSTANT ATV_APP_REVISION_INFO  *ATV_GetAppRevisionInfo (void)
{
    return &RevisionInfo;
}
