
#ifndef __MUX_7XX_H__
#define __MUX_7XX_H__

#define	WITH_MEM_TEST				0

#include "mux7xxHw.h"


#define TEST_SYS_SDRAM_BASE 0x20000000
#define TEST_SYS_SDRAM_SIZE 0x10000000
#define TEST_SYS_SDRAM_PTRN 0x5AA53CC3
//#define TEST_SYS_SDRAM_PTRN 0x00000000




#define	MDIO_OPS_ADDR			0x00
#define	MDIO_OPS_WRITE		0x01
#define	MDIO_OPS_READ_INC		0x02
#define	MDIO_OPS_READ			0x03

#define MDIO_OPS				0x00
#define MDIO_PRTAD				0x01
#define FPGA_DEVAD				0x02
#define FPGA_DATA				0x03
#define FPGA_DATA_MSB			0x04


#define TLK_PRTAD				0x00
#define AQR105_PRTAD			0x00
//#define TLK_DEVAD	0x1E


int  mdioInit(void);

int  muxSi5351bHwInit(void);
int  si5351bTxCfg(void);

short extSensorGetTemperatureCelsius(void);

//#include <rtk_api.h>
//#include <rtk_api_ext.h>
//#include <rtl8307h_types.h> 

//#include <rx_lib.h> 

void fpag_init_tx(void);
void fpag_init_rx(void);

void  dac_param(int  dac);
#if WITH_MEM_TEST
void  ddr_memory_test(void);
#endif

int  muxRxInitVideoParam(void);
int muxTxInitHdmi(void);


unsigned int muxHwTwiBus(void);
unsigned int muxHwTwiSetMuxChannel(unsigned char channo);

int muxFpgaReset(FPGA_CTRL *fpgaCtrl);



#endif

