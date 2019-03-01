/**********************************************************************************************
*																						      *
* Copyright (c) 2015 Analog Devices, Inc.  All Rights Reserved.                               *
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.    *
*                                                                                             *
***********************************************************************************************/
/*============================================================================
 * This file contains all functions that are common across all chip families
 * but can not (or should not) be implemented in the LIB layer
 *
 *===========================================================================*/
#include "rx_lib.h"

#if (RX_DEVICE==7604||RX_DEVICE==7840||RX_DEVICE==7844 \
    ||RX_DEVICE==7842||RX_DEVICE == 7850||RX_DEVICE==7186)

#if RX_INCLUDE_VBI
#include "rx_isr.h"

/*===============================================
 * Defines and macros
 *==============================================*/
#define RX_MAX_NUMBER_OF_LINES          40
#define RX_HALF_NUMBER_OF_LINES         20

#define VRX_VDP_STATUS_REG              0x40
#define VRX_VDP_CONFIG_1_REG            0x60
#define VRX_VDP_CONFIG_2_REG            0x61
#define VRX_VDP_MAN_LINE_1_21_REG       0x64
#define VRX_VDP_VDP_STATUS_CONFIG       0x9C

#if (RX_DEVICE == 7840)
#define RX_VDP_MAP_ADDR                 RX_I2C_VDP_MAP_ADDR                 
#define RX_SDP_VDP_MAP_ADDR             RX_I2C_SDP_VDP_MAP_ADDR 
#endif

#if (RX_DEVICE==7604||RX_DEVICE==7844||RX_DEVICE==7842||RX_DEVICE==7850||RX_DEVICE==7186)
#define RX_VDP_MAP_ADDR                 RX_I2C_VDP_MAP_ADDR                 
#define RX_SDP_VDP_MAP_ADDR             RX_I2C_VDP_MAP_ADDR 
#endif

/*==========================================
 * Local constants
 *=========================================*/
CONSTANT UCHAR RxVdpMaskRegs[] = 
{
    VRX_REG_INTERRUPT_MASKB_4, VRX_REG_INTERRUPT2_MASKB_4, INT_MASK_B4,
    0, 0
};

CONSTANT UCHAR RxTtxtInit[] = 
{
#if (RX_DEVICE != 7850)
    0x7A, 0x02,   /*reg[5-0] => bit_clk_freq_int[6-1],  custom1 bit freq (integer part MSBs), default set for ttxt*/
    0x7B, 0x10,   /*reg[7]   => bit_clk_freq_int[0],   reg[6-0] => bit_clk_freq_fr [12-6], custom1 bit freq */
    0x7C, 0x5C,   /*reg[7-2] => bit_clk_freq_fr[5-0], custom1 bit freq (fractional part LSBs), default set for txtt */
#endif
#if (RX_DEVICE == 7850)
    0xF3, 0x02,
    0xF5, 0xF0,
    0x25, 0x06,
    0xA7, 0x00,
    0xA5, 0x90,
#endif
    0
};

CONSTANT UCHAR RxCcapInit[] = 
{
#if (RX_DEVICE != 7850)

    0x98, 0xA8,   /*Enable adaptive pk-pk detection: 88 is Default, previous recommnedation 08 */
    0x9B, 0x01,   /*Swing enable for CCAP */
    0x9D, 0x02,   /*Enable Slice corrector*/
#endif
#if (RX_DEVICE == 7850)
    0xF3, 0x02,
    0xF5, 0xF0,
    0x25, 0x06,
    0xA7, 0x00,
    0xA5, 0x90,
#endif
    0
};

CONSTANT UCHAR RxCgmsInit[] = 
{
    0x98, 0x08,   /*Enable adaptive pk-pk detection */
    0x9B, 0x04,   /*Swing enable for WSS CGMS*/
    0x9D, 0x02,   /*Enable Slice corrector*/
    0
};

CONSTANT UCHAR RxGems2xInit[] = 
{
    0x9A, 0x83,   /*slice mode 3 - from v780 script */
    0x9B, 0x20,   /*Swing enable for GEMSTAR 2x - from v780 script */
    0x9E, 0x20,   /*Fast lern for GEMSTAR 2x enalbe - from v780 script */
    0
};

CONSTANT UCHAR RxVdpDefaultInit[] = 
{

#if (RX_DEVICE !=7850)
    0x7A, 0x00,
    0x7B, 0x00,
    0x7C, 0x00,
    0x98, 0x88,
    0x9A, 0x80,
    0x9B, 0x05,
    0x9E, 0x00,
    0xA6, 0x00,
  #endif
    0
};
/*VBI Stardard Codes for each Resolution*/
/*Example VBI code 4 Means WSS when Video Resolution is PAL*/
/*Example VBI code 4 Means CGMS when Video Resolution is NTSC*/
CONSTANT UCHAR RxVdpCfgTable[] = 
{
    (UCHAR)RX_TTXT,         (UCHAR)RX_525I,         0x1,    
    (UCHAR)RX_TTXT,         (UCHAR)RX_625I,         0x1,
    (UCHAR)RX_VPS,          (UCHAR)RX_625I,         0x2,
    (UCHAR)RX_VITC,         (UCHAR)RX_525I,         0x3,
    (UCHAR)RX_VITC,         (UCHAR)RX_625I,         0x3,
    (UCHAR)RX_WSS,          (UCHAR)RX_625I,         0x4,
    (UCHAR)RX_CGMS,         (UCHAR)RX_525I,         0x4,
    (UCHAR)RX_CGMS,         (UCHAR)RX_525I,         0x4,
    (UCHAR)RX_CGMS,         (UCHAR)RX_625P,         0x4,        
    (UCHAR)RX_CGMS,         (UCHAR)RX_525P,         0x4, 
    (UCHAR)RX_CGMS,         (UCHAR)RX_720P,         0x4,
    (UCHAR)RX_CGMS,         (UCHAR)RX_1125I_274M,   0x4,
    (UCHAR)RX_GEMS_1X,      (UCHAR)RX_525I,         0x5,
    (UCHAR)RX_GEMS_2X,      (UCHAR)RX_625I,         0x6,
    (UCHAR)RX_CCAP,         (UCHAR)RX_625I,         0x7,
    (UCHAR)RX_CCAP,         (UCHAR)RX_525I,         0x7,
    (UCHAR)RX_CGMS_B,       (UCHAR)RX_525P,         0x8,
    (UCHAR)RX_CGMS_B,       (UCHAR)RX_720P,         0x8,
    (UCHAR)RX_CGMS_B,       (UCHAR)RX_1125I_274M,   0x8,
    (UCHAR)RX_NO_VBI,       0xff,                   0xf,
    0xff
};

/*VBI Stardard Codes for each Resolution*/
CONSTANT UCHAR RxVbiDataStdTable[] = 
{
    (UCHAR)RX_TTXT,    0x1,    
    (UCHAR)RX_VPS,     0x2,
    (UCHAR)RX_VITC,    0x3,
    (UCHAR)RX_WSS,     0x4,
    (UCHAR)RX_CGMS,    0x4,
    (UCHAR)RX_GEMS_1X, 0x5,
    (UCHAR)RX_GEMS_2X, 0x6,
    (UCHAR)RX_CCAP,    0x7,
    (UCHAR)RX_CGMS_B,  0x8,
    (UCHAR)RX_NO_VBI,  0xf,
    0xff
};
/*VBI  Lines for each Standard*/
CONSTANT UCHAR RxVbiF1RangeTable[] = 
{
    (UCHAR)RX_525I,          6, 25, 
    (UCHAR)RX_625I,          6, 25,
    (UCHAR)RX_625P,          6, 45,
    (UCHAR)RX_525P,          6, 25,
    (UCHAR)RX_720P,          6, 25,
    (UCHAR)RX_1125I_274M,    6, 25,
    0xff
};

CONSTANT UINT16 RxVbiF2RangeTable[] = 
{
    (UINT16) RX_525I,       272, 288, 
    (UINT16) RX_625I,       318, 337, 
    (UINT16 )RX_625P,       26,  45,
    (UINT16) RX_525P,       26,  45, 
    (UINT16) RX_720P,       26,  45, 
    (UINT16) RX_1125I_274M, 569, 584,  
    0xff
};


CONSTANT UCHAR RxVdpTtextForceTable[] = 
{
    /*ttext type, Man Value*/
    (UCHAR)RX_TTXT_A,      0,
    (UCHAR)RX_TTXT_B,      1,
    (UCHAR)RX_TTXT_C,      2,
    (UCHAR)RX_TTXT_D,      3,
    0xff
};

CONSTANT UCHAR RxVdpGvpucForceTable[] = 
{
    /*ttext type, Man Value*/
    (UCHAR)RX_GEMS,         0x0,
    (UCHAR)RX_VPS,          0x1,
    (UCHAR)RX_PDC,          0x2,
    (UCHAR)RX_UTC,          0x3,
    (UCHAR)RX_CGMS_B,       0x4,
    0xff
};

CONSTANT UCHAR RxVdpTypeRefreshTable[] = 
{
    /*vbi type, Reg Addr, Bit Location*/
    (UCHAR)RX_CCAP,     0x78,   0,
    (UCHAR)RX_CGMS,     0x78,   2,
    (UCHAR)RX_WSS,      0x78,   2,
    (UCHAR)RX_GEMS_1X,  0x78,   4,
    (UCHAR)RX_GEMS_2X,  0x78,   4,
    (UCHAR)RX_VPS,      0x78,   4,
    (UCHAR)RX_PDC,      0x78,   4,
    (UCHAR)RX_UTC,      0x78,   4,
    (UCHAR)RX_CGMS_B,   0x78,   4,
    (UCHAR)RX_VITC,     0x78,   6,
    (UCHAR)RX_TTXT_A,   0x78,   7,
    (UCHAR)RX_TTXT_B,   0x78,   7,
    (UCHAR)RX_TTXT_C,   0x78,   7,
    (UCHAR)RX_TTXT_D,   0x78,   7,
    (UCHAR)RX_TTXT,     0x78,   7,
    (UCHAR)RX_GEMS,     0x78,   4,      /*can be GEMS_1X or GEMS_2X*/
    0xff
};

CONSTANT UCHAR RxVdpReadBackInfoTable[] = 
{
    /*vbi type, Reg Addr1, Number byte1, Reg Addr2, Number byte2*/
    (UCHAR)RX_CCAP,     0x41,   RX_CCAP_SIZE,       0x00,   0,
    (UCHAR)RX_CGMS,     0x43,   RX_CGMS_SIZE,       0x00,   0,
    (UCHAR)RX_WSS,      0x44,   RX_WSS_SIZE,        0x00,   0,
    (UCHAR)RX_GEMS_1X,  0x47,   RX_GEMS_1X_SIZE,    0x00,   0,
    (UCHAR)RX_GEMS_2X,  0x47,   RX_GEMS_2X_SIZE,    0x00,   0,
    (UCHAR)RX_VPS,      0x47,   RX_VPS_SIZE,        0x00,   0,
    (UCHAR)RX_PDC,      0x47,   RX_PDC_SIZE,        0x00,   0,
    (UCHAR)RX_UTC,      0x78,   RX_UTC_SIZE,        0x00,   0,
    (UCHAR)RX_CGMS_B,   0x3C,   4,                  0x47,   RX_CGMS_B_SIZE-4,
    (UCHAR)RX_VITC,     0x78,   RX_VITC_SIZE,       0x00,   0,
    (UCHAR)RX_GEMS,     0x47,   RX_GEMS_SIZE,       0x00,   0,
    0xff
}; 

/*==============================================
 * Local prototypes
 *=============================================*/
STATIC void RxSdpWriteTable (UCHAR MapAddr, UCHAR *Table, UCHAR EndVal);
STATIC UCHAR RxGetVdpMapAddr (RX_OP_MODE OpMode);

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
STATIC void RxSdpWriteTable (UCHAR MapAddr, UCHAR *Table, UCHAR EndVal)
{
    UINT16 i=0;

    while (Table[i]!= EndVal)
    {
        HAL_I2CWriteByte (MapAddr, Table[i], Table[i+1]);
        i+=3;
    }
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
STATIC UCHAR RxGetVdpMapAddr (RX_OP_MODE OpMode)
{
#if ((RX_DEVICE == 7840))
    switch (OpMode)
    {
        case RX_OPMODE_PWRDWN:
        case RX_OPMODE_COMP:
        case RX_OPMODE_GR:
        case RX_OPMODE_HDMI:
            return (VRX_VDP_MAP_ADDR);
        default:
            return (RX_SDP_VDP_MAP_ADDR);
    }
 
#endif  
    return (VRX_VDP_MAP_ADDR);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpInit(RX_VBI_TYPE DataType)
{
    switch (RxOperatingMode)
    {
        case RX_OPMODE_PWRDWN:
        case RX_OPMODE_COMP:
        case RX_OPMODE_GR:
        case RX_OPMODE_HDMI:
            RxSdpWriteTable(RX_VDP_MAP_ADDR, (UCHAR *)RxVdpDefaultInit, 0);
            switch (DataType)
            {
                case RX_TTXT:
                    RxSdpWriteTable(RX_VDP_MAP_ADDR, (UCHAR *)RxTtxtInit, 0);
                    break;
                case RX_GEMS_2X:
                    RxSdpWriteTable(RX_VDP_MAP_ADDR, (UCHAR *)RxGems2xInit, 0);
                    break;
                default:
                    break;
            }
        case RX_OPMODE_SDP_CVBS:
        case RX_OPMODE_SDP_CVBS_HDMI_MON:
        case RX_OPMODE_SDP_CVBS_YC_AUTO:
        case RX_OPMODE_SDP_YC:
        case RX_OPMODE_SDP_SCART:
            RxSdpWriteTable(RX_SDP_VDP_MAP_ADDR, (UCHAR *)RxVdpDefaultInit, 0);
            switch (DataType)
            {
                case RX_GEMS_2X:
                    RxSdpWriteTable(RX_SDP_VDP_MAP_ADDR, (UCHAR *)RxGems2xInit, 0);
                    break;
                case RX_CCAP:
                    RxSdpWriteTable(RX_SDP_VDP_MAP_ADDR, (UCHAR *)RxCcapInit, 0);
                    break;
                case RX_CGMS:
                    RxSdpWriteTable(RX_SDP_VDP_MAP_ADDR, (UCHAR *)RxCgmsInit, 0);
                    break;
                case RX_TTXT:
                    RxSdpWriteTable(RX_SDP_VDP_MAP_ADDR, (UCHAR *)RxTtxtInit, 0);
                    break;
                default:
                    RxSdpWriteTable(RX_SDP_VDP_MAP_ADDR, (UCHAR *)RxVdpDefaultInit, 0);
                    break;
            }
            break;
        default:
            break;
    }
    return (ATVERR_OK);
}

/*==========================================================================
 * This function set the the lines over which VBI extraction is performed
 *
 * Entry:   DataType: VBI data type    
 *                      RX_TTXT
 *                      RX_VPS
 *                      RX_VITC
 *                      RX_WSS
 *                      RX_CGMS
 *                      RX_GEMS_1X
 *                      RX_GEMS_2X
 *                      RX_CCAP
 *                      RX_CGMS_B
 *                      RX_NO_VBI
 *                  Set Datatype and Video Resolution to apply default standard to line. 
 *                  When set to NULL Vbi Data type will be set as per VbiLine Parameter.
 *          VidRes: Video Resolutions
 *                      RX_525I
 *                      RX_625I
 *                      RX_720P
 *                      RX_1125I_274M
 *                Set to NULL when Datatype desire is RX_NO_VBI  to RX_NO_VBI
 *                Set VidRes and Dataypte to apply default standard to line
 *                When set to NULL Vbi data type will be set as per VbiLine Parameter.    
 *          VbiLine: A pointer to an array UCHAR values. Each lines has two numbers
 *                      - Line number between 1 and 40 as per chip documentation
 *                      - The Line configuration value as per documentation - This value is used only if
 *                        DataType and VidRes are set to NULL.   
 *                      VbiLine[] = { 
 *                                  Line1, Config1,     /configuration set for Line1 to Lin2-1/
 *                                  Line2, Config2,     /configuration set for Line2 to Lin3-1/
 *                                  Line3, Config3,     /configuration set for Line3 to Lin4-1/
 *                                  ...
 *                                  LineN, ConfigN,     /configuration set for LineN to 40/
 *                                  0
 *                                  }
 *                    NULL is not a valid value for VbiLine
 *    
 * Return:  ATVERR_OK = function completed successfully
 *          ATVERR_INV_PARM = Invalid parameters
 *
 * Notes:
 *=========================================================================*/
ATV_ERR HAL_RxVdpSetLines(RX_VBI_TYPE *DataType, RX_VID_RES *VidRes, UCHAR *VbiLine)
{
    UCHAR VdpConfig = 0xff; /*Use VbiLine details*/
    UINT16 i=0, EndLine=0;
    UCHAR LineConfig[RX_HALF_NUMBER_OF_LINES];
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    UINT16 k;

    if(VbiLine == NULL)
    {
        /*VbiLine info required*/
        return (ATVERR_INV_PARM);
    }
    else if ((*DataType == RX_NO_VBI) && (VidRes == NULL))
    {
        VdpConfig = 0xf;   /*Value of 0xf means no data extraction*/
    }     
    else if (DataType && VidRes)    /*Get data type code based on resolution*/
    {
        while (RxVdpCfgTable[i] != 0xff)
        {
            if((RxVdpCfgTable[i] == (UCHAR)(*DataType)) && (RxVdpCfgTable[i+1] == (UCHAR)(*VidRes)))
            {
                break;
            }
            i += 3;
        }
        if (RxVdpCfgTable[i] != 0xff)
        {
            VdpConfig = RxVdpCfgTable[i+2];  /*default code found*/
        }
        else
        {
            return (ATVERR_INV_PARM);       /*no default code found*/
        }
    }
    else if(VidRes && (DataType == NULL))
    {
        return (ATVERR_INV_PARM);
    }

    i = 0;
    HAL_I2CReadBlock (MapAddr, VRX_VDP_MAN_LINE_1_21_REG, LineConfig, RX_HALF_NUMBER_OF_LINES);

    while (VbiLine[i] != 0)
    {
        EndLine = (VbiLine[i+2] !=0 ? (VbiLine[i+2]-1) : RX_MAX_NUMBER_OF_LINES);
        for(k = VbiLine[i]; k <= EndLine; k++)
        {
            /*if VdpConfig configured use VdpConfig and ignore VbiLine info*/
            if(VdpConfig != 0xff)
            {
                /*if K is in Regblock 1*/
                if(k < (RX_HALF_NUMBER_OF_LINES+1))
                {
                    LineConfig[k-1] = (LineConfig[k-1] & 0x0F) | (VdpConfig << 4);
                }
                /*if K is in Regblock 2*/
                else if(k < (RX_MAX_NUMBER_OF_LINES+1))
                {
                    LineConfig[k-(RX_HALF_NUMBER_OF_LINES+1)] = (LineConfig[k-(RX_HALF_NUMBER_OF_LINES+1)] & 0xF0) | (VdpConfig);
                }
                else
                {
                    return (ATVERR_INV_PARM);
                }
            }
            else /*if VdpConfig not configured use VbiLine config info*/
            {
                /*if K is in Regblock 1*/
                if(k < (RX_HALF_NUMBER_OF_LINES+1))
                {
                    LineConfig[k-1] = (LineConfig[k-1] & 0x0F) | (VbiLine[i+1] << 4);
                }
                /*if K is in Regblock 2*/
                else if(k < (RX_MAX_NUMBER_OF_LINES+1))
                {
                    LineConfig[k-(RX_HALF_NUMBER_OF_LINES+1)] = (LineConfig[k-(RX_HALF_NUMBER_OF_LINES+1)] & 0xF0) | (VbiLine[i+1]);
                }
                else
                {
                    return (ATVERR_INV_PARM);
                }
            }
         
        }
        i=i+2; /*increment to next line configuration*/
    }

    HAL_I2CWriteBlock (MapAddr, VRX_VDP_MAN_LINE_1_21_REG, LineConfig, RX_HALF_NUMBER_OF_LINES);
    return (ATVERR_OK);
}

/*=====================================================================================
 *  Datatype  VBI datatype to be cofigured for extraction on Lines defined by VbiLine.
 *  VideRes   To indicate the current resolution.
 *  VbiLine   A pointer to a list of lines on which DataType will be assigned.
 *            Each resolution spec support a range of VBI areas. Refere to documentation for appropriate areas.
 *
 *====================================================================================*/
ATV_ERR HAL_RxVdpSetLinesMan(RX_VBI_TYPE DataType, RX_VID_RES VidRes, UINT16 *VbiLine)
{
    UCHAR VdpConfig = 0x0;
    UCHAR i=0, k;
    UCHAR LineConfig[RX_HALF_NUMBER_OF_LINES];
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    UINT16 VbiStartF1, VbiStartF2;
    UINT16 VbiEndF1, VbiEndF2;
    BOOL Field1= TRUE;
    
    if(VbiLine == NULL)
    {
       /*This function manually sets the standard for a particular line*/
       /*Line must be specified*/
       return (ATVERR_INV_PARM);
    }

    /*Check for WSS exception*/
    if((VidRes == RX_625I && DataType == RX_CGMS) || (VidRes != RX_625I && DataType == RX_WSS))
    {
        return (ATVERR_INV_PARM);
    }
    
    /*Get the VBI data standard code for that resolution*/
    k = ATV_LookupValue8 ((UCHAR*)RxVbiDataStdTable, (UCHAR)DataType, 0xff, 2);
    VdpConfig = RxVbiDataStdTable[k+1];

    /*Get VBI areas for selected video resolution*/
    k = ATV_LookupValue8 ((UCHAR*)RxVbiF1RangeTable, (UCHAR)VidRes, 0xff, 3);
    VbiStartF1= RxVbiF1RangeTable[k+1];
    VbiEndF1=   RxVbiF1RangeTable[k+2];
    VbiStartF2= RxVbiF2RangeTable[k+1];
    VbiEndF2=   RxVbiF2RangeTable[k+2];
    
    /* Read current status of manual configuration registers*/
    HAL_I2CReadBlock (MapAddr, VRX_VDP_MAN_LINE_1_21_REG, LineConfig, RX_HALF_NUMBER_OF_LINES);

    while((VbiLine[i] !=0) && (i<RX_MAX_NUMBER_OF_LINES))
    {
        /*Check if VBI line is valid for that resolution*/
        if((VbiLine[i] <= VbiEndF1) && (VbiLine[i] >= VbiStartF1))
        {
            Field1 = TRUE;
        }
        else if ((VbiLine[i] <= VbiEndF2) && (VbiLine[i] >= VbiStartF2))
        {
            Field1= FALSE;
        }
        else
        {
            return (ATVERR_INV_PARM);
        }  

        /*Get appropriate configuration register current line*/
        if(Field1)
        {
            /*Get line offset*/
            k = VbiLine[i] - VbiStartF1;
            /*Write new value to appropiate VDP config register*/
            LineConfig[k] = ((LineConfig[k] & 0x0F) | (VdpConfig << 4));
        }
        else
        {
            /*Get line offset*/
            k = VbiLine[i] - VbiStartF2;
            /*Write new value to appropiate VDP config register*/
            LineConfig[k] = ((LineConfig[k] & 0xF0) | (VdpConfig));
        }
        i++;
        
    }
   
    /*For each Line listed Vbi lines configure desired standard code*/
    HAL_I2CWriteBlock (MapAddr, VRX_VDP_MAN_LINE_1_21_REG, LineConfig, RX_HALF_NUMBER_OF_LINES);
    return (ATVERR_OK);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpForceDataType(RX_VBI_TYPE DataType, BOOL Enable)
{
    UCHAR RegValue, i, MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    
    switch(DataType)
    {
        case RX_TTXT_A:
        case RX_TTXT_B:
        case RX_TTXT_C:
        case RX_TTXT_D:
            if(!Enable)
            {
                ATV_I2CWriteField8(MapAddr, VRX_VDP_CONFIG_1_REG, 0x4, 0x2, 0);
                return (ATVERR_OK);
            }
            i = (UCHAR)(ATV_LookupValue8 ((UCHAR*)RxVdpTtextForceTable, 
                                                (UCHAR)DataType, 0xff, 2));
            if(RxVdpTtextForceTable[i] != 0xff)
            {
                RegValue = RxVdpTtextForceTable[i+1];
                ATV_I2CWriteField8(MapAddr, VRX_VDP_CONFIG_1_REG, 0x03, 0, RegValue);
                ATV_I2CWriteField8(MapAddr, VRX_VDP_CONFIG_1_REG, 0x4, 0x2, 1);
                return (ATVERR_OK);
            }
            break;
        
        case RX_GEMS:
        case RX_VPS:
        case RX_PDC:
        case RX_CGMS_B:
            i = (UCHAR)(ATV_LookupValue8 ((UCHAR*)RxVdpGvpucForceTable, 
                                                (UCHAR)DataType, 0xff, 2));
            if(RxVdpGvpucForceTable[i] != 0xff)
            {
                RegValue = RxVdpGvpucForceTable[i+1];
                ATV_I2CWriteField8(MapAddr, VRX_VDP_VDP_STATUS_CONFIG, 0x03, 0, RegValue);
                return (ATVERR_OK);
            }
            break;
        default:
            break;
    }
    return (ATVERR_INV_PARM);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpIsTypeDetected(RX_VBI_TYPE DataType)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    BOOL DataDetected = FALSE;
    UCHAR Byte;


    switch(DataType)
    {
        case RX_TTXT_A:
        case RX_TTXT_B:
        case RX_TTXT_D:
        case RX_TTXT_C:
            Byte = (UCHAR)(ATV_LookupValue8 ((UCHAR*)RxVdpTtextForceTable, 
                                        (UCHAR)DataType, 0xff, 2));
            if(RxVdpTtextForceTable[Byte] != 0xff)
            {
                if((ATV_I2CReadField8(MapAddr, VRX_VDP_CONFIG_1_REG, 0x4, 0x2) == RxVdpTtextForceTable[Byte+1])
                    && (ATV_I2CReadField8(MapAddr, VRX_VDP_STATUS_REG, 0x80, 0x7)))
                {
                   DataDetected = TRUE; 
                }
            }
            break;
        case RX_TTXT:
            DataDetected = (BOOL)ATV_I2CIsField8(MapAddr, VRX_VDP_STATUS_REG, 0x80, 0x7);
            break;
        case RX_GEMS_1X:
        case RX_GEMS_2X:
            if( ATV_I2CReadField8(MapAddr, VRX_VDP_STATUS_REG, 0x10, 0x4)
                && ATV_I2CIsField8(MapAddr, VRX_VDP_CONFIG_2_REG, 0x10, 0x4))
            {
                Byte = ATV_I2CReadField8(MapAddr, VRX_VDP_STATUS_REG, 0x20, 0x5);
                if (((DataType == RX_GEMS_1X) && (Byte == 0))
                    || ((DataType == RX_GEMS_2X) && (Byte == 1)))
                {
                    DataDetected = TRUE;
                }
            }
            break;
        case RX_WSS:
        case RX_CGMS:
            DataDetected = (BOOL)ATV_I2CIsField8(MapAddr, VRX_VDP_STATUS_REG, 0x4, 0x2);
            break;
        case RX_VITC:
            DataDetected = (BOOL)ATV_I2CIsField8(MapAddr, VRX_VDP_STATUS_REG, 0x40, 0x6);
            break;
        case RX_CCAP:
            DataDetected = (BOOL)ATV_I2CReadField8(MapAddr, VRX_VDP_STATUS_REG, 0x1, 0x0);
            break;
        case RX_GEMS:
        case RX_CGMS_B:
        case RX_UTC:
        case RX_PDC:
        case RX_VPS:
            DataDetected = (BOOL)ATV_I2CReadField8(MapAddr, VRX_VDP_STATUS_REG, 0x10, 0x4);
            /*review - use reg 0x9c to check the type that has been detected*/ 
            break;
        default:
            return (ATVERR_INV_PARM);
    }
    return (DataDetected ? (ATVERR_TRUE) : (ATVERR_FALSE));
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpClearTypeDetected(RX_VBI_TYPE DataType)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    UCHAR Byte = (UCHAR)(ATV_LookupValue8 ((UCHAR*)RxVdpTypeRefreshTable, 
                                           (UCHAR)DataType, 0xff, 3));


    if(RxVdpTypeRefreshTable[Byte] != 0xff)
    {
        ATV_I2CWriteField8(MapAddr, RxVdpTypeRefreshTable[Byte+1], 
                           1 << RxVdpTypeRefreshTable[Byte+2], 
                           RxVdpTypeRefreshTable[Byte+2], 1);
        return (ATVERR_OK);                   
    }
    return (ATVERR_INV_PARM);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/   
ATV_ERR HAL_RxVdpIsDataOnEvenField(RX_VBI_TYPE DataType)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    BOOL EvenField = FALSE;
    
    if(DataType == RX_CCAP)
    {
        EvenField = ATV_I2CIsField8(MapAddr, 0x40, 0x2, 0x1);
        return ((EvenField) ? ATVERR_TRUE : ATVERR_FALSE);
    }
    return (ATVERR_INV_PARM);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpGetData(RX_VBI_TYPE DataType, UCHAR* Data)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    UCHAR Index = (UCHAR)(ATV_LookupValue8 ((UCHAR*)RxVdpReadBackInfoTable, 
                                           (UCHAR)DataType, 0xff, 5));

    switch(DataType)
    {
        case RX_CCAP:
                   if(RxVdpReadBackInfoTable[Index] != 0xff)
                   {
                        Data[0]= ATV_I2CIsField8(MapAddr, 0x40, 0x2, 0x1);
                        if(RxVdpReadBackInfoTable[Index+2])
                        {
                            HAL_I2CReadBlock (MapAddr, 
                                  RxVdpReadBackInfoTable[Index+1], 
                                  Data+1, 
                                  RxVdpReadBackInfoTable[Index+2]);
                        }
                        if(RxVdpReadBackInfoTable[Index+4])
                        {
                            HAL_I2CReadBlock (MapAddr, 
                                              RxVdpReadBackInfoTable[Index+3], 
                                              (UCHAR*)&Data[(RxVdpReadBackInfoTable[Index+2]+1)],
                                              RxVdpReadBackInfoTable[Index+4]);
                        }
                        return (ATVERR_OK);
                    }
               
                    
                    break;
        default: 
                 if(RxVdpReadBackInfoTable[Index] != 0xff)
                   {
                        if(RxVdpReadBackInfoTable[Index+2])
                        {
                            HAL_I2CReadBlock (MapAddr, 
                                  RxVdpReadBackInfoTable[Index+1], 
                                  Data, 
                                  RxVdpReadBackInfoTable[Index+2]);
                        }
                        if(RxVdpReadBackInfoTable[Index+4])
                        {
                            HAL_I2CReadBlock (MapAddr, 
                                              RxVdpReadBackInfoTable[Index+3], 
                                              (UCHAR*)&Data[(RxVdpReadBackInfoTable[Index+2])],
                                              RxVdpReadBackInfoTable[Index+4]);
                        }
                        return (ATVERR_OK);
                    }
                
    }
    

    return (ATVERR_INV_PARM);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpGetCrc(RX_VBI_TYPE DataType, UCHAR* Crc)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    
    
    if(DataType == RX_VITC)
    {
        *Crc = ATV_I2CReadField8(MapAddr, 0x5e, 0xff, 0);
        return (ATVERR_OK);
    }
    return (ATVERR_INV_PARM);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpEnAutoDetect(RX_VBI_TYPE DataType, BOOL Enable)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);

    Enable &= 1;
    switch(DataType)
    {
        case RX_GEMS_1X:
        case RX_GEMS_2X:
            ATV_I2CWriteField8(MapAddr, 0x61, 0x10, 0x4, (UCHAR)Enable);
            break;
        default:
            return (ATVERR_INV_PARM);
    }
    return (ATVERR_OK);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpEnAutoUpdate(RX_VBI_TYPE DataType, BOOL Enable)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    
    
    Enable &= 1;
    switch(DataType)
    {
        case RX_WSS:
        case RX_CGMS:
            ATV_I2CWriteField8(MapAddr, 0x9C, 0x10, 0x4, (UCHAR)Enable);
            break;
        case RX_UTC:
        case RX_PDC:
        case RX_VPS:
            ATV_I2CWriteField8(MapAddr, 0x9C, 0x20, 0x5, (UCHAR)Enable);
            break;
        default:
            return (ATVERR_INV_PARM);
    }
    return (ATVERR_OK);
}
#if(RX_DEVICE!=7850)
/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpInsertDataIn656Stream(BOOL Enable)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    
    
    Enable &= 1;
    ATV_I2CWriteField8(MapAddr, 0x62, 0x80, 0x7, (UCHAR)Enable);
    return (ATVERR_OK);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpSet656Ident(UCHAR Identifier)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    
    
    ATV_I2CWriteField8(MapAddr, 0x62, 0x1F, 0, Identifier);
    return (ATVERR_OK);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpSetAux656Ident(UCHAR AuxIdentifier)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    
    
    ATV_I2CWriteField8(MapAddr, 0x63, 0x3F, 0, AuxIdentifier);
    return (ATVERR_OK);      
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpSpreadDataOnYAndC(BOOL SpreadData)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    
    
    SpreadData &= 1;
    ATV_I2CWriteField8(MapAddr, 0x63, 0x80, 7, (UCHAR)SpreadData);
    return (ATVERR_OK);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpCfg656Data(RX_ADF_MODE Mode)
{
    UCHAR Val;
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
        
        
    switch(Mode)
    {
        case RX_NIBBLE_MODE:
            Val = 0;
            break;
        case RX_BYTE_MODE_NO_RESTRICT:
            Val = 1;
            break;
        case RX_BYTE_MODE_WITH_RESTRICT:
            Val = 2;
            break;
        default:
            return (ATVERR_INV_PARM); 
    }
    ATV_I2CWriteField8(MapAddr, 0x62, 0x60, 5, Val);
    return (ATVERR_OK);
}

/*==========================================================================
 * 
 * 
 *
 *=========================================================================*/
ATV_ERR HAL_RxVdpEnSerialOutput(RX_VBI_TYPE DataType, BOOL Enable)
{
    UCHAR MapAddr = RxGetVdpMapAddr(RxOperatingMode);
    

    switch(DataType)
    {
        case RX_TTXT:
            if(Enable)
            {
                ATV_I2CWriteField8(MapAddr, 0x98, 0x80, 7, 1);
                ATV_I2CWriteField8(MapAddr, 0x98, 0x10, 4, 1);
            }
            else
            {
                ATV_I2CWriteField8(MapAddr, 0x98, 0x80, 7, 0);
                ATV_I2CWriteField8(MapAddr, 0x98, 0x10, 4, 0); 
            }
            break;          
        default:
            return (ATVERR_INV_PARM);
    }
    return (ATVERR_OK);
}
#endif
/*============================================================================
 * This function is used to enable or disable Vdp events
 *
 * Entry:   EventList: A pointer to a list of RX events that should be enabled
 *                     or disable
 *          ListSize:  The number of event in the listed pointed by EvntList
 *          Enable:    TRUE = To enable the events pointed by EventList
 *                     FALSE = To disable the events pointed by EventList
 *  
 * Return:  Number or enabled/disabled events processed
 *
 * Notes:   None
 *
 *===========================================================================*/
UCHAR HAL_RxVdpSetEnabledEvents (RX_EVENT *EventList, UCHAR ListSize, BOOL Enable)
{
    UINT8 i, j, IntIdx, IntCount=0, MaskVal[NOF_RX_MASK_REGS];
    IntIdx = HAL_RxGetIntIndex();

    /*=======================================
     * Get interrupt mask values
     *======================================*/
    for (i=0; RxVdpMaskRegs[i]; i+=3)
    {
        HAL_I2CReadByte(VRX_IO_MAP_ADDR, RxVdpMaskRegs[IntIdx+i], &(MaskVal[RxVdpMaskRegs[i+2]]));
    }

    /*====================================
     * Enable/disable required events
     *===================================*/
    for (i=0; i<ListSize; i++)
    {
        for (j=0; RxMaskInfo[j].Event != 0xff; j++)
        {
            if (RxMaskInfo[j].Event == EventList[i])
            {
                MaskVal[RxMaskInfo[j].Reg1] &= ~(RxMaskInfo[j].Mask1[IntIdx]);
                RX_EVENT_DISABLE(EventList[i]);
                if (Enable)
                {
                    MaskVal[RxMaskInfo[j].Reg1] |= RxMaskInfo[j].Mask1[IntIdx];
                    RX_EVENT_ENABLE(EventList[i]);
                }
                if (RxMaskInfo[j].Reg2 != 0xff)
                {
                    MaskVal[RxMaskInfo[j].Reg2] &= ~(RxMaskInfo[j].Mask2[IntIdx]);
                    if (Enable)
                    {
                        MaskVal[RxMaskInfo[j].Reg2] |= RxMaskInfo[j].Mask2[IntIdx];
                    }
                }
                IntCount++;
                break;
            }
        }
    }

    /*=======================================
     * Update interrupt masks
     *======================================*/
    for (i=0; RxVdpMaskRegs[i]; i+=3)
    {
        HAL_I2CWriteByte(VRX_IO_MAP_ADDR, RxVdpMaskRegs[IntIdx+i], MaskVal[RxVdpMaskRegs[i+2]]);
        /*HAL_I2CWriteByte(VRX_VDP_MAP_ADDR, RxVdpMaskRegs[IntIdx+i], MaskVal[RxVdpMaskRegs[i+2]]);*/
    }

    /*=======================================
     * Clear masks for unused interrupt
     *======================================*/
    IntIdx = (~IntIdx) & 1;
    for (i=0; RxVdpMaskRegs[i]; i+=3)
    {
        HAL_I2CWriteByte(VRX_IO_MAP_ADDR, RxVdpMaskRegs[IntIdx+i], 0);
    }
    
    return(IntCount);
}

/*==========================================================================
 * Finds out all the active nad unmasked CP interrupts. It clears all the 
 * active interupts on return.
 *
 * Entry:   RxCpInts = is a BOOL array that contains status of all the HDMI
 *                      interrupts
 * 
 * Notes:  None
 *=========================================================================*/
void HAL_RxVdpGetInterrupts(RX_VDP_INTERRUPTS *RxVdpInts)
{
    UINT8 IntStatus4;
        
    HAL_I2CReadByte(VRX_IO_MAP_ADDR, VRX_REG_INTERRUPT_STATUS_4, &IntStatus4);

    /*==================================================================
     * Acknowledge all interrupts immediately so we don't lose any
     *=================================================================*/
    HAL_I2CWriteByte(VRX_IO_MAP_ADDR, VRX_REG_INTERRUPT_CLEAR_4, IntStatus4);
  /*HAL_I2CWriteByte(VRX_VDP_MAP_ADDR, VRX_REG_VDP_STATUS_CLEAR, IntStatus4);*/     /*This line is not needed*/
    
     
    RxVdpInts->TtxtDet = (BOOL) (IntStatus4 & VRX_BIT_TTXT_AVL_ST);                               
    RxVdpInts->VitcDet = (BOOL) (IntStatus4 & VRX_BIT_VITC_AVL_ST);
    RxVdpInts->GsPdcVpsUtcDet = (BOOL) ((IntStatus4 & VRX_BIT_GS_PDC_VPS_UTC_AVL_ST) |
                                       (IntStatus4 & VRX_BIT_GS_DATA_TYPE_ST));
    RxVdpInts->FastDataRbReady = (BOOL) IntStatus4 & VRX_BIT_FASTVDP_DATA_AVL_ST;
    RxVdpInts->CgmsWssDetected = (BOOL) (IntStatus4 & VRX_BIT_CGMS_WSS_AVL_ST);
    RxVdpInts->CcapDet = (BOOL) ((IntStatus4 & VRX_BIT_CCAP_AVL_ST) |
                               (IntStatus4 & VRX_BIT_CCAP_EVEN_FIELD_ST));
}

/*==========================================================================
 * VDP ISR
 *
 * Entry:  None
 * 
 * Return:  ATVERR_OK
 *          ATVERR_FAILED
 * 
 * Notes:  None
 *=========================================================================*/
ATV_ERR HAL_RXLIB_VdpIsr (void)
{
    RX_VDP_INTERRUPTS VdpInts;
    /*UINT8 ByteX; */
    UINT8  PktBuf[2];
    ATV_ERR RetVal = ATVERR_FAILED;
   
    
    /*=======================================
     * Get asserted interrupts
     *======================================*/
    HAL_RxVdpGetInterrupts (&VdpInts);
    
    /*==================================================
     * Teletext detection
     *=================================================*/
    if (VdpInts.TtxtDet)
    {
       
        RX_NOTIFY_USER(RX_EVENT_VDP_TTXT, 0, NULL);
        RetVal = ATVERR_OK;
    }
    if (VdpInts.VitcDet)
    {
       
        RX_NOTIFY_USER(RX_EVENT_VDP_VITC, 0, NULL);
        RetVal = ATVERR_OK;
    }
    if (VdpInts.GsPdcVpsUtcDet)
    {
        PktBuf[0] = (ADIAPI_RxVdpIsTypeDetected(RX_GEMS_1X) == (ATVERR_TRUE) ? 1 : 0);
        PktBuf[1] = (ADIAPI_RxVdpIsTypeDetected(RX_GEMS_2X) == (ATVERR_TRUE) ? 1 : 0);
        /*EventList = RX_EVENT_VDP_GPVUC;*/
        RX_NOTIFY_USER(RX_EVENT_VDP_GPVUC, 0, PktBuf);
        RetVal = ATVERR_OK;
    }
    if (VdpInts.CgmsWssDetected)
    {
        
        RX_NOTIFY_USER(RX_EVENT_VDP_CGMS_WSS, 0, NULL);
        RetVal = ATVERR_OK;
    }
    if (VdpInts.CcapDet)
    {
       /* ByteX = ((ADIAPI_RxVdpIsDataOnEvenField(RX_CCAP) == ATVERR_TRUE) ? 1 : 0) ;*/
       
        RX_NOTIFY_USER(RX_EVENT_VDP_CCAP, 0, NULL);
        RetVal = ATVERR_OK;
    }
    if (VdpInts.FastDataRbReady)
    {
    	RX_NOTIFY_USER(RX_EVENT_VDP_FAST_DATARB_RDY, 0, NULL);
        RetVal = ATVERR_OK;
    		
    }
    return (RetVal);
}

#endif
#endif
