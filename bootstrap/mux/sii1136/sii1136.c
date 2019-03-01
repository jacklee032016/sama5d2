//#include "GlobalDefinitions.h"
//#include "compiler.h"

#include "debug.h"
#include "twi.h"


#include "sii1136.h"
#include "wrapper.h"


Sil9136_env	Sil9136;

Sil9136_env	*pSil9136;


//------------------------------------------------------------------------------
/*! \name Private Functions
 */
//! @{

//static unsigned char	Sil7136_chan = TWI_NONE;

void Sil9136_init()
{
//char  buff[12];
//char  resp[4];

unsigned char  bus = 0;
unsigned char  twi_data;
unsigned char  twi_addr;
unsigned char  twi_dev;
unsigned char	data[16];


	data[0] = 0x04 | SII1136_BUS;	// set mux for SII1136
	twi_write(bus, PCA9544_ADDR, 0, 0, data, 1);

// Init SI9136

//	Sil9136_Select_Chan(chan);
	// Enter TPI mode
	twi_dev = SIL9136_TPI_ADDR>>1;
	twi_addr = 0xc7;
	twi_data = 0;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);

	// Soft reset
	twi_addr = 0x40;
	ATV_I2CWriteField8(twi_dev<<1, twi_addr, 0x01, 0, 1);
	ATV_I2CWriteField8(twi_dev<<1, twi_addr, 0x01, 0, 0);

	twi_addr = 0x1B;
	twi_read(bus, twi_dev, twi_addr, 1, data, 3);
	dbg_info("HDMI resp %x %x %x \n", data[0], data[1], data[2]);

	twi_addr = 0x1E;
	ATV_I2CWriteField8(twi_dev<<1, twi_addr, 0x03, 0, 0);


	twi_addr = 0x08;
	twi_data = 0x60;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x0B;
	twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x60;
	twi_data = 0x04;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x61;
	twi_data = 0x03;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);

	twi_addr = 0x3C;
	twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);

	// Set TMDS PLL
#if 0
	twi_addr = 0xBC;  twi_data = 0x01;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0xBD;  twi_data = 0x90;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0xBE;  twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
#endif

	Sil9136_write_idx(0x01, 0x4b, 0x00);
	Sil9136_write_idx(0x01, 0x4c, 0x00);
	Sil9136_write_idx(0x01, 0x4d, 0x00);

// 	Sil9136_set_video_info(chan, 7424, 24, 2750, 1125);

#if 0
	twi_addr = 0x00; twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x01; twi_data = 0x1D;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);	// 0x1D00 = 7424 ( *10000 pixel clock )
	twi_addr = 0x02; twi_data = 0x18;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x03; twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);	// 0x0018 = 24 Hz ( frame rate )
	twi_addr = 0x04; twi_data = 0xBE;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x05; twi_data = 0x0A;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);// 0x0ABE = 2750 pix per lines
	twi_addr = 0x06; twi_data = 0x65;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x07; twi_data = 0x04;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);	// 0x0465 = 1125 lines
#else
	Sil9136_set_video_info(14850, 60, (1920+280), (1080+8) );
#endif

	twi_addr = 0x09; twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x0A; twi_data = 0x10;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);	// bt.601/bt.709

	twi_addr = 0x60; twi_data = 0x04;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);	// YC mux mode
	twi_addr = 0x63; twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);


	twi_addr = 0x1A; twi_data = 0x11;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x19; twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_addr = 0x1E; twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);

	twi_addr = 0x26; twi_data = 0xA1;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1); // audio 8 ch PCM
	twi_addr = 0x25; twi_data = 0x03;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);	// audio do not check
	twi_addr = 0x27; twi_data = 0xD8;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);


	twi_addr = 0x1A; twi_data = 0x00;
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);


	// For test
	// Enable all interrupt on SIL9136
//	Sil9136_SetIRQStatus(0x00ffffff);
//	Sil9136_SetIRQEnable(0xC3);	 // HCDP, RX Sense, HotPlug

	return;
}

void Sil9136_Select_Chan(unsigned char chan)
{
//	Sil7136_chan = chan;
//	pSil9136 = Sil9136_var[chan];

//	twi_select_bus(chan);
}

static char dbgstr[32];

#if 0
void Sil9136_Hotplug()
{
int i;
	// read EDID memory to local memory
	Sil9136_ReadEDID(DDC_EPROM, 0x00, 0x00, 256, pSil9136->EDID);

	DEBUGSTR("EDID\r\n");
	for (i=0; i<256; i++) {
		itoa(pSil9136->EDID[i], dbgstr, 16);
		DEBUGSTR(dbgstr);
		DEBUGSTR(" ");
		if ((i & 0x0f) == 0x0f) DEBUGSTR("\r\n");
	}
	DEBUGSTR("\r\n");

	if (Sil7136_chan == 0) {
		Base_LED_Set( LED_HDMI_OUT1_GRN, 1);
		Base_LED_Set( LED_HDMI_OUT1_RED, 0);
	}

	if (Sil7136_chan == 1) {
		Base_LED_Set( LED_HDMI_OUT2_GRN, 1);
		Base_LED_Set( LED_HDMI_OUT2_RED, 0);
	}

	return;
}

void Sil9136_Unplug()
{
	if (Sil7136_chan == 0) {
		Base_LED_Set( LED_HDMI_OUT1_GRN, 0);
		Base_LED_Set( LED_HDMI_OUT1_RED, 1);
	}

	if (Sil7136_chan == 1) {
		Base_LED_Set( LED_HDMI_OUT2_GRN, 0);
		Base_LED_Set( LED_HDMI_OUT2_RED, 1);
	}

	return;
}

unsigned char  Sil9136_SetIRQEnable(unsigned char set)
{
unsigned char  tx[2];

	tx[0] = (unsigned char)SIL9136_TPI_IRQ_ENABLE;	//
	tx[1] = (unsigned char)(set);
	return (twi_mux_master_transfer(Sil7136_chan, SIL9136_TPI_ADDR, 2, tx, 0, tx ) );
}

unsigned char  Sil9136_GetIRQEnable()
{
unsigned char  tx[1];
unsigned char  stat;

	stat = 0;
	tx[0] = (unsigned char)SIL9136_TPI_IRQ_ENABLE;	//
	twi_mux_master_transfer(Sil7136_chan, SIL9136_TPI_ADDR, 1, tx, 1, &stat );

	return(stat);
}


unsigned char  Sil9136_SetIRQStatus(unsigned int stat)
{
unsigned char  tx[4];

	tx[0] = (unsigned char)SIL9136_TPI_IRQ_STATUS;	//
	tx[1] = (unsigned char)(stat);
	tx[2] = (unsigned char)(stat>>8);
	tx[3] = (unsigned char)(stat>>16);
	return (twi_mux_master_transfer(Sil7136_chan, SIL9136_TPI_ADDR, 4, tx, 0, tx ) );
}

unsigned int  Sil9136_GetIRQStatus()
{
unsigned char  tx[4];
unsigned int stat;

	stat = 0; 
	tx[0] = (unsigned char)SIL9136_TPI_IRQ_STATUS;	//
	twi_mux_master_transfer(Sil7136_chan, SIL9136_TPI_ADDR, 1, tx, 3, &stat );
	return(stat);
}
#endif


// ------------------------------------------------------------
// Write indexe reg ( Page, Reg , data)
//
void Sil9136_write_idx(unsigned char page, unsigned char index, unsigned char data)
{
unsigned char  tx[4];
unsigned char  bus = 0;
unsigned char  twi_addr;
unsigned char  twi_dev;

	bus = 0;
	twi_dev = SIL9136_TPI_ADDR>>1;
	twi_addr = SIL9136_PAGE_REG;
	tx[0] = (unsigned char)page;
	tx[1] = (unsigned char)index;
	tx[2] = (unsigned char)data;

//	twi_write(bus, twi_dev, twi_addr, 1, &tx, 3);
	return;
}

#if 0
// ------------------------------------------------------------
// Read indexe reg ( Page, Reg )
//
unsigned char Sil9136_read_idx(unsigned char page, unsigned char index)
{
	twi_mux_Write8(Sil7136_chan, SIL9136_TPI_ADDR, SIL9136_PAGE_REG, page);
	twi_mux_Write8(Sil7136_chan, SIL9136_TPI_ADDR, SIL9136_ADDR_REG, index);
	return( twi_mux_Read8(Sil7136_chan, SIL9136_TPI_ADDR, SIL9136_DATA_REG) );
}
#endif

void Sil9136_set_video_info(unsigned int pclk, unsigned int V_Freq, unsigned int PixPerLine, unsigned int NbLines)
{
unsigned char  bus = 0;
unsigned char  twi_addr;
unsigned char  twi_dev;
unsigned char  tx[9];

	twi_dev = SIL9136_TPI_ADDR>>1;
	twi_addr = 0x00;

	tx[0] = (unsigned char)pclk;
	tx[1] = (unsigned char)(pclk>>8);
	tx[2] = (unsigned char)V_Freq;
	tx[3] = (unsigned char)(V_Freq>>8);
	tx[4] = (unsigned char)PixPerLine;
	tx[5] = (unsigned char)(PixPerLine>>8);
	tx[6] = (unsigned char)NbLines;
	tx[7] = (unsigned char)(NbLines>>8);

	twi_write(bus, twi_dev, twi_addr, 1, tx, 8);

	return;
}


void Sil9136_set_TMDS(unsigned char  set)
{
unsigned char  twi_addr;
unsigned char  twi_dev;

	twi_dev = SIL9136_TPI_ADDR>>1;
	twi_addr = 0x1A;

	ATV_I2CWriteField8(twi_dev<<1, twi_addr, 0x10, 4, set);
}

unsigned char  Sil9136_get_TMDS()
{
unsigned char  twi_addr;
unsigned char  twi_dev;

	twi_dev = SIL9136_TPI_ADDR>>1;

	twi_addr = 0x1A;
	return( ATV_I2CReadField8(twi_dev<<1, twi_addr, 0x10, 4) );
}

#if 0

void Sil9136_set_AVMute(unsigned char  set)
{
	twi_mux_WriteField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x08, 3, set);
}

unsigned char  Sil9136_get_AVMute()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x08, 3) );
}

void Sil9136_set_HDMIMode( unsigned char  set)
{
	twi_mux_WriteField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x01, 0, set);
}

unsigned char  Sil9136_get_HDMIMode()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x01, 0) );
}


void Sil9136_set_IrqEna(unsigned char  Irq)
{
	twi_mux_Write8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x3C, Irq);
}

unsigned char  Sil9136_get_IrqEna()
{
	return( twi_mux_Read8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x3C) );
}

// ------------------------------------------------------------
// Request DDC bus from host
//
void Sil9136_set_ReqDDC(unsigned char  set)
{
int timeout = 100;

	if (set != 0) {
		twi_mux_WriteField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x04, 2, 0x01);

		do {
			if (twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x02, 1) != 0) break;
			msDelay(1);
		} while (--timeout);

		twi_mux_WriteField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x06, 1, 0x03);

	} else {

		do {
			twi_mux_WriteField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x06, 1, 0x00);
			if (twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x06, 1) == 0) break;;
			msDelay(1);
		} while (--timeout);
	}
}

// ------------------------------------------------------------
// Check if request DDC is active
//
unsigned char  Sil9136_get_ReqDDC()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x04, 2) );
}

// ------------------------------------------------------------
// Check if DDC is granted
//
unsigned char  Sil9136_get_GntDDC()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0x1A, 0x02, 1) );
}


unsigned char Sil9136_DDC_idx(unsigned char Dev, unsigned char Seg, unsigned char Off)
{
unsigned char  tx[4];
unsigned char  bRet;

	tx[0] = (unsigned char)0xED;	// register 0
	tx[1] = (unsigned char)(Dev<<1);
	tx[2] = (unsigned char)Seg;
	tx[3] = (unsigned char)Off;

	bRet = twi_mux_master_transfer(Sil7136_chan, SIL9136_TPI_ADDR, 4, tx, 0, tx );
	return bRet;
}

unsigned char Sil9136_DDC_count(unsigned int count)
{
unsigned char  tx[3];
unsigned char  bRet;

	tx[0] = (unsigned char)0xF0;	// register 0
	tx[1] = (unsigned char)count;
	tx[2] = (unsigned char)((count>>8) & 0x03);

	bRet = twi_mux_master_transfer(Sil7136_chan, SIL9136_TPI_ADDR, 3, tx, 0, tx );
	return bRet;
}

unsigned char Sil9136_DDC_cmd(unsigned char cmd)
{
unsigned char  tx[2];
unsigned char  bRet;

	tx[0] = (unsigned char)0xF3;	// register 0
	tx[1] = (unsigned char)cmd;

	bRet = twi_mux_master_transfer(Sil7136_chan, SIL9136_TPI_ADDR, 2, tx, 0, tx );
	return bRet;
}

unsigned char Sil9136_DDC_GetStat()
{
unsigned char  tx[1];
unsigned char  rx[1];

	tx[0] = (unsigned char)0xF2;	// register 0

	twi_mux_master_transfer(Sil7136_chan, SIL9136_TPI_ADDR, 1, tx, 1, rx );
	return( rx[0] );
}

unsigned char  Sil9136_DDC_IsBusLow()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF2, 0x40, 6) );
}

unsigned char  Sil9136_DDC_IsNoAck()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF2, 0x20, 5) );
}

unsigned char  Sil9136_DDC_IsInProgress()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF2, 0x10, 4) );
}

unsigned char  Sil9136_DDC_IsFifoFull()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF2, 0x08, 3) );
}

unsigned char  Sil9136_DDC_IsFifoEmpty()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF2, 0x04, 2) );
}

unsigned char  Sil9136_DDC_IsReadUse()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF2, 0x02, 1) );
}

unsigned char  Sil9136_DDC_IsWriteUse()
{
	return( twi_mux_ReadField8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF2, 0x01, 0) );
}

unsigned char  Sil9136_DDC_RdFifoCnt()
{
	return( twi_mux_Read8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF5) & 0x1F );
}

void  Sil9136_DDC_WrFifoData(unsigned char data)
{
	return( twi_mux_Write8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF4, data) );
}

unsigned char  Sil9136_DDC_RdFifoData()
{
	return( twi_mux_Read8 (Sil7136_chan, SIL9136_TPI_ADDR, 0xF4) );
}

// ------------------------------------------------------------
// Read the EDID from EDID prom
//
// need to exit TPI mode
// do EDID transaction
// Re-enter TPI mode
//
unsigned char  Sil9136_ReadEDID(unsigned char Dev, unsigned char Seg, unsigned char Off, unsigned int NofBytes, unsigned char  *data)
{
int  i, j, idx, cnt;
unsigned char   nb;
unsigned char  RdCnt;
unsigned char  prevHDCP;

	// Exit HDCP mode
	prevHDCP = twi_mux_Read8(Sil7136_chan, SIL9136_TPI_ADDR, 0x2A);
	twi_mux_Write8(Sil7136_chan, SIL9136_TPI_ADDR, 0x2A, 0x20);

	// Exit TPI mode
	twi_mux_Write8(Sil7136_chan, SIL9136_TPI_ADDR, 0xBC, 0x80);	// Exit TPI mode

	Sil9136_DDC_cmd(SIL9136_DDC_DELAY_ENA | SIL9136_DDC_GLITCH_ENA | SIL9136_CMD_CLK_SCL );	// init
	while (Sil9136_DDC_IsInProgress()!=0);

	idx = 0;
	cnt = NofBytes;
	for (i=0; i<NofBytes; i+=16) {
		nb = (cnt > 16) ? 16 : cnt;

		Sil9136_DDC_cmd(SIL9136_DDC_DELAY_ENA | SIL9136_DDC_GLITCH_ENA | SIL9136_CMD_CLRFIFO );	// Clear fifo for next transaction
		while (Sil9136_DDC_IsInProgress()!=0);

		Sil9136_DDC_idx(Dev, Seg, (Off+idx) );
		Sil9136_DDC_count(nb);
		Sil9136_DDC_cmd(SIL9136_DDC_DELAY_ENA | SIL9136_DDC_GLITCH_ENA | SIL9136_CMD_SEQ_RD_NOACK );

		while (Sil9136_DDC_IsInProgress()!=0);

		RdCnt = Sil9136_DDC_RdFifoCnt();
		for (j=0; j<RdCnt; j++) data[idx+j] = Sil9136_DDC_RdFifoData();

		idx += 16;
		cnt -= 16;
		msDelay(0);
	}

	// Enable TPI mode
	twi_mux_Write8(Sil7136_chan, SIL9136_TPI_ADDR, 0xC7, 0x00);
	twi_mux_Write8(Sil7136_chan, SIL9136_TPI_ADDR, 0x2A, prevHDCP);

	return NofBytes;
}


void Sil9136_ProcIrqStat(unsigned int Stat)
{
	if ((Stat & SIL9136_IRQS_HPEVENT) != 0) { // Hotplug
		if ((Stat & SIL9136_IRQS_HPSTATE) != 0) { // hotplug is high
			DEBUGSTR( "Hotplug\n\r");
			Sil9136_Hotplug();
		} else { // Unplug
			DEBUGSTR( "Unplug\n\r");
			Sil9136_Unplug();
		}
	}

	if ((Stat & SIL9136_IRQS_RXEVENT) != 0) { // Receiver Sense
		DEBUGSTR( "Rx sense\n\r");
	}

	if ((Stat & SIL9136_IRQS_HDCPEVENT) != 0) { // HDCP
		DEBUGSTR( "HDCP sense\n\r");
	}

}

void  Sil9136_Proc_Isr()
{
static unsigned int  IrqStat;
unsigned char	IrqFlag;

	if   ((ch2_int_Mutex==NULL)
		|| (ch3_int_Mutex==NULL)
		|| (ch4_int_Mutex==NULL) ) return;

#if 1
	if (xSemaphoreTake( ch2_int_Mutex, 0 ) ) {
		Sil9136_Select_Chan(2);
		IrqStat = Sil9136_GetIRQStatus();
		Sil9136_ProcIrqStat(IrqStat);

		IrqFlag = Sil9136_GetIRQEnable();
		Sil9136_SetIRQStatus(IrqFlag);

	}

	if (xSemaphoreTake( ch3_int_Mutex, 0 ) ) {
		Sil9136_Select_Chan(0);
		IrqStat = Sil9136_GetIRQStatus();
		Sil9136_ProcIrqStat(IrqStat);

		IrqFlag = Sil9136_GetIRQEnable();
		Sil9136_SetIRQStatus(IrqFlag);
	}

	if (xSemaphoreTake( ch4_int_Mutex, 0 ) ) {
		Sil9136_Select_Chan(1);
		IrqStat = Sil9136_GetIRQStatus();
		Sil9136_ProcIrqStat(IrqStat);

		IrqFlag = Sil9136_GetIRQEnable();
		Sil9136_SetIRQStatus(IrqFlag);
	}
#endif

	// Recheck if IRq lines still low
	if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, 0x07, 4) == 0)  xSemaphoreGive( ch2_int_Mutex);
	if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, 0x07, 14) == 0) xSemaphoreGive( ch3_int_Mutex);
	if (Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, 0x07, 15) == 0) xSemaphoreGive( ch4_int_Mutex);
}


#endif


