
#ifndef SIL9136_H_
#define SIL9136_H_

//#define SIL9136_MAX_DEV		3

#define PCA9544_ADDR	(0xE0>>1)


#define	SIL9136_TPI_ADDR	0x72
#define	SIL9136_CFG_ADDR	0x7A
#define	SIL9136_CPI_ADDR	0xC0

#define SII1136_BUS		2

#define SIL9136_PAGE_REG	0xBC
#define SIL9136_ADDR_REG	0xBD
#define SIL9136_DATA_REG	0xBE


#define	SIL9136_DDC_DELAY_ENA		0x00
#define	SIL9136_DDC_DELAY_DIS		0x20

#define	SIL9136_DDC_GLITCH_ENA		0x00
#define	SIL9136_DDC_GLITCH_DIS		0x10

#define	SIL9136_CMD_ABORT		0x0F
#define	SIL9136_CMD_CLRFIFO		0x09
#define	SIL9136_CMD_CLK_SCL		0x0A

#define	SIL9136_CMD_ADDR_RD_NOACK		0x00
#define	SIL9136_CMD_SEQ_RD_NOACK		0x02
#define	SIL9136_CMD_ENH_DDC_NOACK		0x04
#define	SIL9136_CMD_SEQ_WR_NOACK		0x06
#define	SIL9136_CMD_SEQ_WR_ACK		0x06


#define SIL9136_TPI_IRQ_ENABLE		0x3C
#define	SIL9136_TPI_IRQ_STATUS		0x3D


#define	SIL9136_IRQS_HPEVENT		0x01
#define	SIL9136_IRQS_RXEVENT		0x02
#define	SIL9136_IRQS_HPSTATE		0x04
#define	SIL9136_IRQS_RXSTATE		0x08

#define SIL9136_IRQS_AUDIO			0x10
#define SIL9136_IRQS_SECURITY		0x20
#define SIL9136_IRQS_HDCPSTATE		0x40
#define SIL9136_IRQS_HDCPEVENT		0x80


typedef struct sSil9136_env {

	unsigned char  EDID[256];
} Sil9136_env, *pSil9136_env;


void Sil9136_init();
void Sil9136_Select_Chan(unsigned char chan);
void Sil9136_Proc_Isr(void);


void Sil9136_Hotplug(void);
void Sil9136_Unplug(void);

unsigned char  Sil9136_SetIRQEnable(unsigned char set);
unsigned char  Sil9136_GetIRQEnable(void);
unsigned char  Sil9136_SetIRQStatus(unsigned int stat);
unsigned int  Sil9136_GetIRQStatus(void);


void Sil9136_set_TMDS(unsigned char  set);
unsigned char  Sil9136_get_TMDS(void);

void Sil9136_set_AVMute(unsigned char  set);
unsigned char  Sil9136_get_AVMute(void);

void Sil9136_set_HDMIMode(unsigned char  set);
unsigned char  Sil9136_get_HDMIMode(void);

void Sil9136_set_IrqEna(unsigned char  Irq);
unsigned char  Sil9136_get_IrqEna(void);

void Sil9136_set_ReqDDC(unsigned char  set);
unsigned char  Sil9136_get_GntDDC(void);


void Sil9136_set_video_info(unsigned int pclk, unsigned int V_Freq, unsigned int PixPerLine, unsigned int NbLines);



void Sil9136_write_idx(unsigned char page, unsigned char index, unsigned char data);
unsigned char Sil9136_read_idx(unsigned char page, unsigned char index);

unsigned char Sil9136_DDC_idx(unsigned char Dev, unsigned char Seg, unsigned char Off);
unsigned char Sil9136_DDC_count(unsigned int count);
unsigned char Sil9136_DDC_cmd(unsigned char cmd);


unsigned char 	Sil9136_DDC_GetStat(void);
unsigned char  Sil9136_DDC_IsBusLow(void);
unsigned char  Sil9136_DDC_IsNoAck(void);
unsigned char  Sil9136_DDC_IsInProgress(void);
unsigned char  Sil9136_DDC_IsFifoFull(void);
unsigned char  Sil9136_DDC_IsFifoEmpty(void);
unsigned char  Sil9136_DDC_IsReadUse(void);
unsigned char  Sil9136_DDC_IsWriteUse(void);

unsigned char  Sil9136_DDC_RdFifoCnt(void);
void  Sil9136_DDC_WrFifoData(unsigned char data);
unsigned char  Sil9136_DDC_RdFifoData(void);

unsigned char  Sil9136_ReadEDID(unsigned char Dev, unsigned char Seg, unsigned char Off, unsigned int NofBytes, unsigned char  *data);

void Sil9136_ProcIrqStat(unsigned int Stat);




#endif

