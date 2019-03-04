
#include "muxStrapCmn.h"

#define NUM_REGS_MAX 100

typedef struct Reg_Data{
   unsigned char Reg_Addr;
   unsigned char Reg_Val;
} Reg_Data;

Reg_Data const Reg_Store[NUM_REGS_MAX] = {
{ 15,0x00},
{ 16,0x0F},
{ 17,0x0F},
{ 18,0x0F},
{ 19,0x0F},
{ 20,0x0F},
{ 21,0x4F},
{ 22,0x2F},
{ 23,0x0F},
{ 24,0x00},
{ 25,0x00},
{ 26,0x0C},
{ 27,0x35},
{ 28,0x00},
{ 29,0x0E},
{ 30,0x38},
{ 31,0x00},
{ 32,0x04},
{ 33,0x68},
{ 34,0x31},
{ 35,0x2D},
{ 36,0x00},
{ 37,0x0F},
{ 38,0xD1},
{ 39,0x11},
{ 40,0x18},
{ 41,0xC3},
{ 42,0xEC},
{ 43,0x85},
{ 44,0x00},
{ 45,0x0C},
{ 46,0x29},
{ 47,0xA0},
{ 48,0xD8},
{ 49,0xB3},
{ 50,0x00},
{ 51,0x7D},
{ 52,0x00},
{ 53,0x0E},
{ 54,0xE5},
{ 55,0x00},
{ 56,0x00},
{ 57,0x2F},
{ 58,0x01},
{ 59,0x77},
{ 60,0x00},
{ 61,0x0D},
{ 62,0x04},
{ 63,0x00},
{ 64,0x01},
{ 65,0x24},
{ 66,0x00},
{ 67,0x7D},
{ 68,0x00},
{ 69,0x0E},
{ 70,0xE5},
{ 71,0x00},
{ 72,0x00},
{ 73,0x2F},
{ 74,0x0C},
{ 75,0x35},
{ 76,0x00},
{ 77,0x0E},
{ 78,0x38},
{ 79,0x00},
{ 80,0x04},
{ 81,0x68},
{ 82,0x00},
{ 83,0x01},
{ 84,0x00},
{ 85,0x1F},
{ 86,0x00},
{ 87,0x00},
{ 88,0x00},
{ 89,0x00},
{ 90,0x06},
{ 91,0x42},
{ 92,0x00},
{149,0x00},
{150,0x00},
{151,0x00},
{152,0x00},
{153,0x00},
{154,0x00},
{155,0x00},
{156,0x00},
{157,0x00},
{158,0x00},
{159,0x00},
{160,0x00},
{161,0x00},
{162,0x26},
{163,0x58},
{164,0x01},
{165,0x00},
{166,0x00},
{167,0x00},
{168,0x00},
{169,0x00},
{170,0x00},
};

#define  ADV7619_BUS	2


#define	SI5351B_BUS	0

#define	MDIO_BUS		0xFF


#define	SI5351B_ADDR	(0xC0>>1)


int  muxSi5351bInit(void)
{
	int i;
	unsigned char	buffer[2];
	unsigned int bus;

	dbg_info("si5351b init....\n");

	if (!twi_init_done)
		twi_init();

	bus = muxHwGetTwiBus();
	buffer[0] = 0x04 | SI5351B_BUS;	// set mux for si5351b 
	if(twi_write(bus, PCA9544_ADDR, 0, 0, buffer, 1))
	{
		dbg_info("PCA init failed\n");
		return -1;
	}


	buffer[0] = 0xff;
	twi_write(bus, SI5351B_ADDR, 3, 1, buffer, 1);

	buffer[0] = 0x80;
	twi_write(bus, SI5351B_ADDR, 16, 1, buffer, 1);
	twi_write(bus, SI5351B_ADDR, 17, 1, buffer, 1);
	twi_write(bus, SI5351B_ADDR, 18, 1, buffer, 1);
	twi_write(bus, SI5351B_ADDR, 19, 1, buffer, 1);
	twi_write(bus, SI5351B_ADDR, 20, 1, buffer, 1);
	twi_write(bus, SI5351B_ADDR, 21, 1, buffer, 1);
	twi_write(bus, SI5351B_ADDR, 22, 1, buffer, 1);
	twi_write(bus, SI5351B_ADDR, 23, 1, buffer, 1);


	for (i=0; i<NUM_REGS_MAX; i++)
	{
		buffer[0] = Reg_Store[i].Reg_Val;
		twi_write(bus, SI5351B_ADDR, Reg_Store[i].Reg_Addr, 1, buffer, 1);
	}

	// adjust crystal capacitante register
	buffer[0] = 0x12;
	twi_write(bus, SI5351B_ADDR, 183, 1, buffer, 1);
	// reset PLL
	buffer[0] = 0xAC;
	twi_write(bus, SI5351B_ADDR, 177, 1, buffer, 1);
	buffer[0] = 0x00;
	twi_write(bus, SI5351B_ADDR, 3, 1, buffer, 1);
	

	if(twi_read(bus, SI5351B_ADDR, 0, 1, buffer, 1))
	{
		dbg_info("SI5351B read failed\n");
		return -1;
	}
	dbg_info("SI5351B status: %x, %s\n", buffer[0], (buffer[0]==0x11)?"OK":"Failed");
	
	dbg_info("si5351b end!");
	return 0;
}

