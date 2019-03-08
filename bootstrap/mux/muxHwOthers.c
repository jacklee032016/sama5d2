
#include "muxStrapCmn.h"
#include "gpio.h"
#include "arch/at91_pio.h"


#include <adv7619_init.h> 

#define  ADV7619_BUS	2

void Sil9136_set_video_info(unsigned int pclk, unsigned int V_Freq, unsigned int PixPerLine, unsigned int NbLines);

//rtk_port_link_ability_t		P4_ability;
//rtk_port_link_ability_t		P5_ability;
//rtk_port_link_ability_t		P6_ability;

//rtk_port_link_ability_t	heac0;
//rtk_port_link_ability_t	heac1; 

const RX_EVENT	EventList[] = {RX_EVENT_HDMI_CABLE_DETECT, RX_EVENT_HDMI_MODE_CHNG };

static const RX_PIN RxHdmlPin[] = 
{
    RX_LLC_OUT_PIN,     RX_SYNC_OUT_PINS,  
    RX_PIX_OUT_PINS,    RX_AUD_OUT_PINS,
    RX_INT2_OUT_PIN,
    (RX_PIN)0xff 
};



/*============================================================================
 * Initialize RX device for HDMI operation
 *
 * 
 * 
 *===========================================================================*/
void RxApi_HdmiHardwareInit (void)
{
    UCHAR i=0;
    RX_OP_MODE OpMode = RX_OPMODE_HDMI;
    RX_PIX_BUS_INFO PixBusInfo;

    /*==========================
     * Reset RX device
     *=========================*/
#if ((RX_DEVICE != 7840) && (RX_DEVICE != 7604) && (RX_DEVICE != 7614) && (RX_DEVICE != 7180))
    VRX_set_MAIN_RESET(1);
    HAL_DelayMs(10);
#endif
#if ((RX_DEVICE == 7844) || (RX_DEVICE == 7842))
    VRX_set_HDCP_REPT_EDID_RESET(1);
    HAL_DelayMs(5);
#endif

    /*==========================
     * Turn on RX device 
     *=========================*/
    ADIAPI_RxInit(RX_OPMODE_HDMI);

    /*===============================
     * Disable all ports
     *==============================*/
    ADIAPI_RxHdmiSetManualTmdsTerm(PORT_ALL, FALSE);

    /*===============================
     * Set BCAPS and Bstatus
     *==============================*/
    ADIAPI_RxHdmiSetVisibleRepMap(PORT_A);
//    ADIAPI_RxHdmiSetHdcpBcaps(PORT_ALL, HDCP_REC_BCAPS);
//    ADIAPI_RxHdmiSetHdcpBstatus(PORT_ALL, HDCP_REC_BSTATUS);

    /*================================================
     * Set manual freerun mode and colors
     * Configure pixel port
     *===============================================*/
    VRX_set_CP_DEF_COL_MAN_VAL(1);
    PixBusInfo.Format    = REP_RX_PIX_BUS_FORMAT_444;
    PixBusInfo.ChOrder   = REP_RX_PIX_BUS_CH_ORDER_444;
    PixBusInfo.BitWidth  = REP_RX_PIX_BUS_WIDTH_444;
    PixBusInfo.Mode      = REP_RX_PIX_BUS_MODE_444;
    PixBusInfo.CrCbSwap  = REP_RX_PIX_BUS_CRCB_SWAP_444;
    PixBusInfo.BitReorder= REP_RX_PIX_BUS_REORDER_444;
    ADIAPI_RxSetOpPixFormat (&PixBusInfo, &OpMode);

    /*============================================================
     * Interrupts setup. Set masks and clear all interrupts
     * Enable both Audio Mute and Main Interrupts
     *===========================================================*/
    ADIAPI_RxSetEnabledEvents ((RX_EVENT *)EventList, sizeof(EventList)/sizeof(RX_EVENT), TRUE);

#if (RX_DEVICE == 7612) || (RX_DEVICE == 7611) || (RX_DEVICE == 7619)
    /* We use MCLK_PIN for AUDIO_MUTE, INT1 pin for RX_IRQ */
    ADIAPI_RxSetPinFunction(RX_MCLK_PIN, RX_AUDIO_MUTE_SIG);
    ADIAPI_RxSetPinFunction(RX_INT1_OUT_PIN, RX_INT_SIG);
#else
    ADIAPI_RxSetPinFunction(((RX_HW_INT_ID==2)?RX_INT1_OUT_PIN:RX_INT2_OUT_PIN), RX_AUDIO_MUTE_SIG);
    ADIAPI_RxSetPinFunction(((RX_HW_INT_ID==2)?RX_INT2_OUT_PIN:RX_INT1_OUT_PIN), RX_INT_SIG);
#endif
#if (RX_DEVICE == 7623) || (RX_DEVICE == 7622) || (RX_DEVICE == 76221)
    ADIAPI_RxConfigInt(RX_INT2_OUT_PIN, RX_ACT_UNTIL_CLR, RX_OPEN_DRAIN);
    ADIAPI_RxConfigInt(RX_INT1_OUT_PIN, RX_ACT_UNTIL_CLR, RX_OPEN_DRAIN);
#else
    ADIAPI_RxConfigInt(RX_INT2_OUT_PIN, RX_ACT_UNTIL_CLR, RX_LOW_LEVEL);
    ADIAPI_RxConfigInt(RX_INT1_OUT_PIN, RX_ACT_UNTIL_CLR, RX_LOW_LEVEL);
#endif

    /*==============================================================
     * Initialize colour space converter to DVI
     * RGB in => RGB out, offset registers for RGB (unipolar data)
     *=============================================================*/
    i = 0;
#if (RX_DEVICE == 7623) || (RX_DEVICE == 7622) || (RX_DEVICE == 76221)
#elif ((RX_DEVICE == 7844) || (RX_DEVICE == 7842) || (RX_DEVICE == 7612) \
    || (RX_DEVICE == 7611) || (RX_DEVICE == 7619))
#if ATV_MOTHERBOARD
    i = (HAL_GetTxBoardRevision() == TX_7510_BOARD_REV_C)? 1: 0;
#endif
#else
    i = 1;
#endif
    ADIAPI_RxHdmiSetSyncPolarity(0xFF, 0xFF, i);
    VRX_set_RGB_OUT(1);                 /* output RGB */
    VRX_set_INP_COLOR_SPACE(0x0f);      /* input auto CS (from AVI) */
    VRX_set_UP_CONVERSION_MODE(0);      /* do not interpolate */
    VRX_set_QZERO_ITC_DIS(1);
    VRX_set_QZERO_RGB_FULL(1);

    /*=======================================================
     * - Set NOT_AUTO_UNMUTE to 0
     *======================================================*/
    VRX_set_WAIT_UNMUTE(RX_AUDIO_UNMUTE_DELAY);
    VRX_set_MT_MSK_COMPRS_AUD(0);
    VRX_set_MUX_HBR_OUT(0);
    VRX_set_OVR_MUX_HBR(1);

    /*=======================================================
     * Remove Tristate on all pins
     *======================================================*/
    for (i=0; RxHdmlPin[i] != 0xff; i++)
    {
        ADIAPI_RxTristatePin(RxHdmlPin[i], FALSE);
    }
}

ATV_ERR ADIAPI_RepRxShutdown (void)
{
    ADIAPI_RxSetOperatingMode (RX_OPMODE_PWRDWN, NULL);
    ADIAPI_RxHdmiSetManualTmdsTerm(PORT_ALL, FALSE);
    return (ATVERR_OK);
}

void REP_RxSoftwareInit (void)
{
#if 0
    UCHAR i;
    /*====================================
     * Read h/w state
     *===================================*/
    RxReadStateFromHw();
    for (i=0; i<NOF_RX_PORTS; i++)
    {
        VidPllReset[i] = FALSE;
    }
    AudioChanged            = FALSE;
    VidDetected             = FALSE;
    VidStable               = FALSE;
    TimeAtVidOk             = 0;
    TimeAtMuteEnd           = 0;
    HpdUserDelay            = 0;
    CurrHdcpMode            = HDCP_INVALID;
    CurrOperMode            = MODE_INVALID;
    AutoHdcpSetting         = 2;
    StblTimeStarted         = 0;
    FreeRunState            = 0;                        /* RX Free Run State */
    FreeRunCs               = 0;
    RxStatPkt.FreeRunOn     = FreeRunState? TRUE: FALSE;
    RxStatPkt.WaitingForBksv= 0;
    RxStatPkt.AvailPkts     = 0;
    DstCounter              = 0;
    CheckDvi                = TRUE;
    LastRxHkTime            = 0;
    UnmuteDelayStarted      = FALSE;
    LastPortRx              = PORT_NONE;
    ATV_DUMMY_SET(DstCounter);
    InitTxVars();
    ADIAPI_MwRxSetConfig (MWRX_ENABLE_DBG, FALSE);
#endif
}

void REP_RxHardwareInit (void)
{
//    ProcessRxHdcp();
//    ConfigureRxCSC (Y1Y0_RGB, 0, 0);
    ADIAPI_RxCpEnHdmiFreeRun(TRUE);
    ADIAPI_RxCpEnManFreeRunColor(TRUE);
//    SetManualFRunMode(FALSE);
#if RX_INCLUDE_CEC
    ADIAPI_RxCecReset();
    ADIAPI_RxCecEnable(FALSE);
#endif
}

void HAL_PlatformInit (void)
{
}

void    HAL_PlatformRXInit (void)
{
}



void fpag_init_tx(void)
{
unsigned char  bus;
unsigned char  twi_data;
unsigned char  twi_addr;
unsigned char  twi_dev;
unsigned char	data[16];

	bus = 0;
	twi_dev = (0x60>>1);
	twi_addr = 1;
	twi_data = (7<<2) | (0<<0);
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);

	twi_addr = 0;
	data[0] = 0xff;
	data[1] = 0xfe;
	data[2] = 0xfd;
	data[3] = 0xfc;
	twi_read(bus, twi_dev, twi_addr, 1, data, 4);
	dbg_info("FPGA(60.0) %x %x %x %x \n", data[0],data[1],data[2], data[3] );

	twi_dev = (0x68>>1);
	twi_addr = 4;
	data[0] = 0x10;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0xE5;
	data[4] = 0xA0;
	data[5] = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, data, 6);

	twi_addr = 10;
	data[0] = 0x11;
	twi_write(bus, twi_dev, twi_addr, 1, data, 6);

	twi_addr = 16;
	data[0] = 210;
	data[1] = 4;
	data[2] = 0;
	data[3] = 10;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);
#if 1
// video dest ip
	twi_addr = 20;
	data[0] = 211;
	data[1] = 4;
	data[2] = 0;
	data[3] = 10;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);
#endif

#if 0
// audio dest ip
	twi_addr = 24;
	data[0] = 211;
	data[1] = 4;
	data[2] = 0;
	data[3] = 10;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);
#endif

	twi_addr = 32;
	data[0] = 0x30;
	data[1] = 0xc2;
	data[2] = 0x20;
	data[3] = 0xc2;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);

	twi_addr = 36;
	data[0] = 0x50;
	data[1] = 0xc2;
	data[2] = 0x40;
	data[3] = 0xc2;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);

	twi_addr = 40;
	data[0] = 0x70;
	data[1] = 0xc2;
	data[2] = 0x60;
	data[3] = 0xc2;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);

	twi_addr = 1;
	data[0] = 0x06;  // enable audio + video
//	twi_write(bus, twi_dev, twi_addr, 1, &data, 1);


}

void fpag_init_rx(void)
{
unsigned char  bus;
unsigned char  twi_data;
unsigned char  twi_addr;
unsigned char  twi_dev;
unsigned char	data[16];

	bus = 0;
	twi_dev = (0x60>>1);

	twi_addr = 1;
	twi_data = (7<<2) | (0<<0);
	// Set the video format as: bits(4:2) = RGB Raw, bits(1:0) = 8-bits per pixel
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);

  // for debug
	twi_addr = 0;
	data[0] = 0xff;
	data[1] = 0xfe;
	data[2] = 0xfd;
	data[3] = 0xfc;
	twi_read(bus, twi_dev, twi_addr, 1, data, 4);
	dbg_info("FPGA(60.0) %x %x %x %x \n", data[0],data[1],data[2], data[3] );

	twi_dev = (0x68>>1);
	twi_addr = 4;
	data[0] = 0x11; //10;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0xE5;
	data[4] = 0xA0;
	data[5] = 0x00;
	// Set the 48-bit Local MAC address 
	twi_write(bus, twi_dev, twi_addr, 1, data, 6);

	twi_addr = 10;
	data[0] = 0x10; //0x11;
	// Set the 48-bit Destination MAC address
	twi_write(bus, twi_dev, twi_addr, 1, data, 6);

	twi_addr = 16;
	data[0] = 211; //210;
	data[1] = 4;
	data[2] = 0;
	data[3] = 10;
	// Set the 32-bit Local Video IP address 
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);
	
#if 1
// video dest ip
	twi_addr = 20;
	data[0] = 210; // 211;
	data[1] = 4;
	data[2] = 0;
	data[3] = 10;
	// Set the 32-bit Destination Video IP address  
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);
#endif

#if 0
// audio dest ip
	twi_addr = 24;
	data[0] = 211;
	data[1] = 4;
	data[2] = 0;
	data[3] = 10;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);
#endif

	twi_addr = 32;
	// Set the 16-bit Local UDP Video Port 
	data[0] = 0x30;
	data[1] = 0xc2;
	// Set the 16-bit Destination UDP Video Port
	data[2] = 0x20;
	data[3] = 0xc2;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);

	twi_addr = 36;
	// Set the 16-bit Local UDP Audio Port 
	data[0] = 0x50;
	data[1] = 0xc2;
	// Set the 16-bit Destination UDP Audio Port
	data[2] = 0x40;
	data[3] = 0xc2;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);

	twi_addr = 40;
	// Set the 16-bit Local UDP Ancillary Port 
	data[0] = 0x70;
	data[1] = 0xc2;
	// Set the 16-bit Destination UDP Ancillary Port
	data[2] = 0x60;
	data[3] = 0xc2;
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);

	twi_addr = 1;
	data[0] = 0x06;  // enable audio + video
//	twi_write(bus, twi_dev, twi_addr, 1, &data, 1);


}



unsigned char  init_4k[] =
// ##03 HDMI Input to ADV7619, Input Pixel Frequency > 170MHz ##
// 03-01 RGB 444 In - 2x24-bit RGB 444 Out - For use up to 4k2k:
{
//0x98, 0xFF, 0x80, // I2C reset
// set I2c address
0x98, 0xF4, 0x80, // CEC
0x98, 0xF5, 0x7C, // INFOFRAME
0x98, 0xF8, 0x4C, // DPLL
0x98, 0xF9, 0x64, // KSV
0x98, 0xFA, 0x6C, // EDID
0x98, 0xFB, 0x68, // HDMI
0x98, 0xFD, 0x44, // CP

0x68, 0xC0, 0x03, // ADI Required Write
0x68, 0x00, 0x09, // Set HDMI Input Port B (BG_MEAS_PORT_SEL = 001b)
0x68, 0x02, 0x03, // ALL BG Ports enabled

0x68, 0x6C, 0xA3, // Enable clock terminators for port A & B 
0x68, 0x83, 0xFC, // Enable clock terminators for port A & B 

// set equaliser
0x68, 0x85, 0x10, // ADI Required Write 
0x68, 0x86, 0x9B, // ADI Required Write 
0x68, 0x89, 0x03, // HF Gain
0x68, 0x9B, 0x03, // ADI Required Write
0x68, 0x93, 0x03, // ADI Required Write
0x68, 0x5A, 0x80, // ADI Required Write
0x68, 0x9C, 0x80, // ADI Required Write
0x68, 0x9C, 0xC0, // ADI Required Write
0x68, 0x9C, 0x00, // ADI Required Write

0x98, 0x02, 0xF2, // Auto CSC, RGB out, Set op_656 bit
0x98, 0x03, 0x54, // 2x24 bit SDR 444 interleaved mode 0
0x98, 0x05, 0x28, // AV Codes Off
0x98, 0x06, 0xA0, // No inversion on VS,HS pins
0x98, 0x0C, 0x42, // Power up part
0x98, 0x15, 0x80, // Disable Tristate of Pins
0x98, 0x19, 0x83, // LLC DLL phase
0x98, 0x33, 0x40, // LLC DLL MUX enable
0x4C, 0xB5, 0x01, // Setting MCLK to 256Fs


0x98, 0x00, 0x19, // Set VID_STD
0x98, 0x01, 0x05, // Prim_Mode =101b HDMI-Comp
0x98, 0xDD, 0x00, // ADI Required Write
0x98, 0xE7, 0x04, // ADI Required Write 
0x4C, 0xC3, 0x80, // ADI Required Write
0x4C, 0xCF, 0x03, // ADI Required Write
0x4C, 0xDB, 0x80, // ADI Required Write
0x68, 0xC0, 0x03, // ADI Required write
0x68, 0x03, 0x98, // ADI Required Write
0x68, 0x10, 0xA5, // ADI Required Write
0x68, 0x1B, 0x00, // ADI Required Write
0x68, 0x45, 0x04, // ADI Required Write
0x68, 0x97, 0xC0, // ADI Required Write
0x68, 0x3E, 0x69, // ADI Required Write
0x68, 0x3F, 0x46, // ADI Required Write
0x68, 0x4E, 0xFE, // ADI Required Write 
0x68, 0x4F, 0x08, // ADI Required Write
0x68, 0x50, 0x00, // ADI Required Write
0x68, 0x57, 0xA3, // ADI Required Write
0x68, 0x58, 0x07, // ADI Required Write
0x68, 0x6F, 0x08, // ADI Required Write
0x68, 0x84, 0x03, // FP MODE

0x00, 0, 0 };

const unsigned char _acComplete_EDID_SEIKI_4K[256UL + 1] = {
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4C, 0xAB, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x23, 0x17, 0x01, 0x03, 0x81, 0x56, 0x30, 0x78, 0x8A, 0xA5, 0x8E, 0xA6, 0x54, 0x4A, 0x9C, 0x26, 0x12, 0x45, 0x46, 0xAD, 0xCE, 0x00, 0x81, 0x40,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58, 0x8A, 0x00, 0x56, 0xE1, 0x31, 0x00, 0x00, 0x1E, 0x9A, 0x29, 0xA0, 0xD0, 0x51, 0x84, 0x22, 0x30,
  0x50, 0x98, 0x36, 0x00, 0x60, 0xE1, 0x31, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x32, 0x4B, 0x18, 0x3C, 0x0B, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x53, 0x45, 0x33, 0x39, 0x55, 0x59, 0x30,
  0x34, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x01, 0x65, 0x02, 0x03, 0x2A, 0x71, 0x4F, 0x06, 0x07, 0x02, 0x03, 0x15, 0x96, 0x11, 0x12, 0x13, 0x04, 0x14, 0x05, 0x1F, 0x90, 0x20, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x6D, 0x03, 0x0C, 0x00,
  0x20, 0x00, 0x08, 0x3C, 0x20, 0x40, 0x68, 0x01, 0x02, 0x03, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55, 0x00, 0x56, 0xE1, 0x31, 0x00, 0x00, 0x18, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58, 0x2C, 0x25, 0x00,
  0x56, 0xE1, 0x31, 0x00, 0x00, 0x9E, 0x01, 0x1D, 0x80, 0xD0, 0x72, 0x1C, 0x16, 0x20, 0x10, 0x2C, 0x25, 0x80, 0x56, 0xE1, 0x31, 0x00, 0x00, 0x9E, 0x01, 0x1D, 0x00, 0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8, 0x28, 0x55, 0x40, 0x56, 0xE1, 0x31, 0x00,
  0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00
};


void  init_adv7619_B(void)
{
unsigned char	buffer[4];
unsigned int bus;

	bus = 0;
	buffer[0] = 0x04 | ADV7619_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buffer, 1);


	dbg_info("init ADV7619 \n");


	ATV_I2CWriteTable (init_4k, 0);
}


void  init_adv7619(void)
{
//unsigned char	version[32];
unsigned char	buffer[4];
//unsigned char	data[16];
unsigned int bus;
unsigned int chan = 2;

//unsigned int	RxRev;
RX_OP_MODE 	OpMode = RX_OPMODE_HDMI;
//RX_PIX_BUS_INFO PixBusInfo;
unsigned char  bRet;

	bus = 0;
	buffer[0] = 0x04 | ADV7619_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buffer, 1);


	dbg_info("init ADV7619 \n");
	//ADIAPI_RxGetChipRevision (version);
//	dbg_info(version);

	ADIAPI_RxSetDeviceIndex(chan);
	ADIAPI_RxInit(OpMode);
	ADIAPI_RxHdmiAssertHpd(PORT_ALL, FALSE);
	ADIAPI_RepRxShutdown();     /* Turn off RX device */
	REP_RxSoftwareInit();       /* Initialize Repeater software (RX part) */
	RxApi_HdmiHardwareInit();   /* Initialize RX hw for HDMI operation */
	REP_RxHardwareInit();       /* Initialize RX hw for repeater operation */

	HAL_PlatformRXInit();      /* Initialize platform-dependent RX settings */

//	ATV_I2CWriteTable (init_tbl_4k, 0);
                                        
	bRet = ADIAPI_RxHdmiSetSelectedPort ( PORT_B , TRUE);
	ADIAPI_RxSetAudioInterface(RX_AUD_IF_AUTO );

	bRet = ADIAPI_RxHdmiSetInternalEdid((UCHAR *) _acComplete_EDID_SEIKI_4K , 0x0020 , NULL , 0 , FALSE);
//	bRet = ADIAPI_RxHdmiSetInternalEdid( _acsamsung_edid , 0 , NULL , 0 , TRUE);

	bRet = ADIAPI_RxHdmiEnableInternalEdid(PORT_ALL , FALSE);
	ADIAPI_RxHdmiAssertHpd ( PORT_B , TRUE);


#if 0
	ADIAPI_RxSetDeviceIndex(chan);
	ADIAPI_RxInit( OpMode);

		//Rx4K2KResSettingTable
//	ATV_I2CWriteTable (init_tbl_4k, 0);
//	ATV_I2CWriteTable (init_tbl_HD, 0);


	ADIAPI_RxHdmiAssertHpd ( PORT_B , FALSE);
	ADIAPI_RxManagePower( RX_ALL_SECTIONS, TRUE);
	ADIAPI_RxGetChipRevision(&RxRev);
	ADIAPI_RxChipReset( RX_SDP_MAIN);
	ADIAPI_RxChipReset( RX_EDID_REP_MPU);

	bRet = ADIAPI_RxHdmiSetSelectedPort ( PORT_B , TRUE);
	ADIAPI_RxSetAudioInterface(RX_AUD_IF_AUTO );

	bRet = ADIAPI_RxHdmiEnableInternalEdid(PORT_ALL , FALSE);

//	bRet = ADIAPI_RxHdmiSetInternalEdid( _acComplete_EDID_SEIKI_4K , 0x0020 , NULL , 0 , FALSE);
	bRet = ADIAPI_RxHdmiSetInternalEdid( _acsamsung_edid , 0 , NULL , 0 , TRUE);

	bRet = ADIAPI_RxHdmiSetHdcpI2CBase ( 0x74 ) ;

#if 0
	ATV_I2CWriteField8(VRX_REPEATER_MAP_ADDR, 0x77, 0xFF, 0x0, 0x00);
	ATV_I2CWriteField8(VRX_REPEATER_MAP_ADDR, 0x52, 0xFF, 0x0, 0x20);
	ATV_I2CWriteField8(VRX_REPEATER_MAP_ADDR, 0x53, 0xFF, 0x0, 0x00);
	ATV_I2CWriteField8(VRX_REPEATER_MAP_ADDR, 0x70, 0xFF, 0x0, 0x9E);
#endif

//	bRet = ADIAPI_RxHdmiEnableInternalEdidN(chan, PORT_ALL , TRUE);

//	ADIAPI_RxSetEnabledEvents (EventList, sizeof(EventList)/sizeof(RX_EVENT), TRUE);

	//RX_3_XTAL_CLK , RX_15_XTAL_CLK,  RX_63_XTAL_CLK, 	RX_ACT_UNTIL_CLR
//	ADIAPI_RxConfigInt( RX_INT1_OUT_PIN , RX_ACT_UNTIL_CLR /*RX_63_XTAL_CLK*/ , RX_LOW_LEVEL );

	// Enable Hotplug signal
//	msDelay(50);

//	ADIAPI_RxSetDeviceIndex(chan);
	ADIAPI_RxHdmiAssertHpd ( PORT_B , TRUE);

	PixBusInfo.BitWidth = RX_BUS_BITS_8;
	PixBusInfo.Format = RX_BUS_MODE5; // RX_BUS_MODE0;
	PixBusInfo.Mode = RX_BUS_OP_MODE_444SDR;
	PixBusInfo.ChOrder = RX_BUS_OP_CH_GBR;
	PixBusInfo.CrCbSwap = 0;
	PixBusInfo.BitReorder = 0;
//	ADIAPI_RxSetOpPixFormat( &PixBusInfo, &OpMode);

#endif

		//Rx4K2KResSettingTable
//	ATV_I2CWriteTable (init_tbl_4k, 0);
//	ATV_I2CWriteTable (init_tbl_HD, 0);

//	ADIAPI_RxSetInpVid4K2K(TRUE);

	//ADIAPI_RxCpSetManGainFilter(RX_4K_LINES);
	//ADIAPI_RxCpConfigHsyncLckMsrmt(100, RX_256_LINES);
	//ADIAPI_RxCpSetNoiseCalibWindow (10, RX_4K_LINES);

	ATV_I2CWriteTable((UCHAR *)init_clk_gt170, 0);

#if 1
	VRX_set_OP_FORMAT_SEL(0x54);
	VRX_set_VIDEO_CORE_ENABLE_MAN_EN(1);
	VRX_set_VIDEO_FREQ_LIMIT_DISABLE(1);
	VRX_set_AUDIO_FREQ_LIMIT_DISABLE(1);

	VRX_set_MAN_OP_CLK_SEL(0x2);
	VRX_set_MAN_OP_CLK_SEL_EN(1);
#endif
	return;
}

#define MCP4706_BUS   1


void  dac_param(int  dac)
{
unsigned char	twi_dev;
//unsigned char	twi_addr;
unsigned char	data[16];
unsigned int bus = 0;
unsigned char	cmd;
unsigned char	pd;
unsigned int  udac;

	udac = 0x7ff + dac;
	bus = 0;
	data[0] = 0x04 | MCP4706_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, data, 1);

	twi_dev = (0XC0>>1);
	cmd = 0;
	pd = 0;
	data[0] = ((cmd&0X03) << 6) | ((pd&0X03)<<4) | ((udac>>8)&0x0f);
	data[1] = udac;
	twi_write(bus, twi_dev, 0, 0, data, 2);
}


void  init_video_param(void)
{
unsigned char	twi_dev;
unsigned char	twi_addr;
unsigned char	data[16];
unsigned int bus = 0;
//unsigned int chan = 0;

unsigned int x, y;
unsigned int sync_x, sync_y;

unsigned int sx, sy;

#if 1
	x = 3840;
	y = 2160;
	sync_x = 560;
	sync_y = 90; //48;

	Sil9136_set_video_info(29700, 30, (x+sync_x), (y+sync_y-1));

	init_pll_drp(0);	// X 2 

#else
	x = 1920;
	y = 1080;
	sync_x = 280;
	sync_y = 45;

	Sil9136_set_video_info(14850, 60, (x+sync_x), (y+sync_y-1));

	init_pll_drp(1);	// X 1

#endif


	twi_dev = (0x60>>1);

	twi_addr = 6;
	// video active
	// Number of active pixels 
	data[0] = (x & 0xff);
	data[1] = (x >> 8);
	// Number of active rows 
	data[2] = (y & 0xff);
	data[3] = (y >> 8);
	// Setting the active horizontal and vertical video size
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);


	twi_addr = 10;
	sx = x + sync_x -1;
	sy = y + sync_y -1;
	// video size total
	// Total number of cycles per row 
#if	(MUX_BOARD == MUX_BOARD_768)
	data[0] = (sx & 0xff);
	data[1] = (sx >> 8);
	data[2] = (sy & 0xff);
	data[3] = (sy >> 8);
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);

	twi_addr = 14;
	sx = sync_x-1;
	sy = sync_y-1;
	// video offset
	data[0] = (sx & 0xff);
	data[1] = (sx >> 8);
	data[2] = (sy & 0xff);
	data[3] = (sy >> 8);
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);

#elif (MUX_BOARD == MUX_BOARD_774)
	data[0] = (sx & 0xff);
	data[1] = (sx >> 8);
	data[2] = 0;
	data[3] = 0;
	// Total number of rows per frame
	data[4] = (sy & 0xff);
	data[5] = (sy >> 8);
	data[6] = 0;
	data[7] = 0;
	twi_write(bus, twi_dev, twi_addr, 1, data, 8);

	// FPGA does not have video offset registers ???
	/*
	twi_addr = 14;
	sx = sync_x-1;
	sy = sync_y-1;
	// video offset
	data[0] = (sx & 0xff);
	data[1] = (sx >> 8);
	data[2] = (sy & 0xff);
	data[3] = (sy >> 8);
	twi_write(bus, twi_dev, twi_addr, 1, data, 4);
	*/
#endif
	return;
}


void  init_pll_drp(unsigned char mode)
{
unsigned char	twi_dev;
unsigned char	twi_addr;
unsigned char	data[16];
unsigned int bus = 0;
//unsigned int chan = 0;

unsigned char	drp_daddr;
unsigned drp_data;

unsigned char	cycle;

	switch (mode) {
	case 0: cycle = 2;  break;	// x2, 297 mhz
	case 1: cycle = 4;  break;	// x1, 148.5 mhz
	case 2: cycle = 8;  break;	// /2, 74.25 mhz
	default: cycle = 16;  break;	// /4, 37.125 mhz
	}

	twi_dev = (0x6A>>1);
	twi_addr = 0;
	drp_daddr = 0x08; 	// clk0, reg 1
	// see xapp888 for ext info on drp prog of pll module
	drp_data = cycle | (cycle<<6) | (1<<12) | (0 << 13);	// div by (cycle+cycle)

	data[0] = (drp_data & 0xff);
	data[1] = (drp_data >> 8);
	data[2] = drp_daddr;
	data[3] = 0x03;  // drp rd/wr cycle on this i2c write
	twi_write(bus, twi_dev, twi_addr, 1, data, 5);

	twi_addr = 4;
	twi_read(bus, twi_dev, twi_addr, 1, data, 3);

	dbg_info("DRP read %x %x %x\n", data[0], data[1], data[2] );


	return;
}


