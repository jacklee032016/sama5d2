

#ifndef _mdio_drv_h_
#define _mdio_drv_h_


#define I2C_ADDR_MDIO	(0x64 >> 1)		// 8 bits addr format
#define I2C_MDIO_MUX	TWI_NONE

#define	MDIO_OPS_ADDR	0x00
#define	MDIO_OPS_WRITE	0x01
#define	MDIO_OPS_READ_INC	0x02
#define	MDIO_OPS_READ	0x03

#define MDIO_OPS		0x00
#define MDIO_PRTAD		0x01
#define FPGA_DEVAD		0x02
#define FPGA_DATA		0x03
#define FPGA_DATA_MSB	0x04
#define FPGA_CONTROL	0x05
#define FPGA_STATUS		0x06


#define TLK_PRTAD	0x00
#define AQR105_PRTAD	0x00
//#define TLK_DEVAD	0x1E


unsigned char  mdio_init(void);

unsigned char  mdio_write_addr(unsigned char  prtad, unsigned char  devad,  unsigned int  data);
unsigned char  mdio_write_data(unsigned char  prtad, unsigned char  devad,  unsigned int  data);
unsigned char  mdio_read_data(unsigned char  prtad,  unsigned char  devad,  unsigned int  *data);
unsigned char  mdio_read_data_incr(unsigned char  prtad, unsigned char  devad, unsigned int  *data);
unsigned char  mdio_read_data_buf(unsigned int  *data);



#endif
