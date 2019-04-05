#include <stdio.h>
#include "controler.h"
#include "multithread.h"

#define _mdio_drv_h_
#define I2C_ADDR_PCS_PMA    (0x6E)                          // 8 bits addr format

// Control
#define PCS_PMA_RESET       0x00

// Status
#define PMA_STATUS_0        0x20
#define PMA_STATUS_1        0x21
#define PMA_VERSION_0       0x22
#define PMA_VERSION_1       0x23
#define PCS_STATUS_0        0x24
#define PCS_STATUS_1        0x25
#define PCS_PMA_LOCK        0x2B

#define port_name "eth0"

typedef unsigned char		bool;
typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;

#ifdef MDIO_DRV_ENABLE
unsigned char  mdio_write_addr(unsigned char  prtad, unsigned char  devad, unsigned int  data);
unsigned char  mdio_read_data(unsigned char  prtad, unsigned char  devad, unsigned int  *data);
#endif
void checkColorSpace(void);
void checkStatusPCS_PMA(void);
void * link_10G_routine();