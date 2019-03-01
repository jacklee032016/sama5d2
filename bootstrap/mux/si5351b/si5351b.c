#include "common.h"
#include "hardware.h"
#include "board.h"
#include "twi.h"
#include "debug.h"

#include "register_map_500768.h"

#include "si5351b.h"


static unsigned int si5351b_get_twi_bus(void)
{
	unsigned int bus = 0;

#if defined(CONFIG_TWI0)
	bus = 0;
#elif defined(CONFIG_TWI1)
	bus = 1;
#elif defined(CONFIG_TWI2)
	bus = 2;
#elif defined(CONFIG_TWI3)
	bus = 3;
#endif
	return bus;
}

void  init_si5351b(void)
{
	int i;
	unsigned char	buffer[2];
	unsigned int bus;


	bus = si5351b_get_twi_bus();
	buffer[0] = 0x04 | SI5351B_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buffer, 1);


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


	for (i=0; i<NUM_REGS_MAX; i++) {
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
	
	return;
}

