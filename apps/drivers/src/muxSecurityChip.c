
#include "muxHwDrv.h"
#include "muxDrv.h"

#include "mux7xxCompact.h"

#include "sha256_software.h"

#if	(MUX_BOARD == MUX_BOARD_768)
#define	_REG_SET_IO				S_PIO_SIONR2
#define	_REG_MASK					PIO_MSKR2
#define	_REG_CFG					PIO_CFGR2
#define	_REG_SET_OUT_DATA			PIO_SODR2
#define	_REG_CLEAR_OUT_DATA		PIO_CODR2
#define	_REG_PIN_DATA_STATUS		PIO_PDSR2

#define	_SC_PIN						0x02000000	/* PC25 */
#elif (MUX_BOARD == MUX_BOARD_774)
#define	_REG_SET_IO				S_PIO_SIONR3
#define	_REG_MASK					PIO_MSKR3
#define	_REG_CFG					PIO_CFGR3
#define	_REG_SET_OUT_DATA			PIO_SODR3
#define	_REG_CLEAR_OUT_DATA		PIO_CODR3
#define	_REG_PIN_DATA_STATUS		PIO_PDSR3

#define	_SC_PIN						0x00000400	/* PD10. p.482 of datasheet */
#elif (MUX_BOARD == MUX_BOARD_767)
#else
#error 	Not support board definition
#endif

#define	_SC_PIN_MODE_OUT			0x00000100	/* p454, CFG register */
#define	_SC_PIN_MODE_IN			0x00000000	/* p454, CFG register */

#define	_SC_MAP(addr, _register)	\
			(addr) = ioremap((_register), 4)


#define	_SC_UNMAP(addr)	\
			iounmap((addr))

#define	_SC_WRITE(addr, value)		\
			writel_relaxed((value), (addr) )

#define	_SC_READ(addr)		\
			readl_relaxed((addr) )


// delay durations
#define EEPROM_WRITE_DELAY       10  	//tprd unit:ms
#define SHA_COMPUTATION_DELAY    3   	//unit:ms
#define SECRET_PROGRAM_DELAY	 100	//tprs unit:ms

#define CMD_READ_ROM			 	0x33	//Read rom ID
#define CMD_READ_WRITE_SCRATCHPAD   0x0F	//Read/write scratchpad 
#define CMD_ROM_SKIP             	0xCC
#define CMD_WRITE_MEMORY         	0x55	//Write to memory
#define CMD_READ_MEMORY         	0xF0	
#define CMD_READ_STATUS	         	0xAA
#define CMD_COMPUTE_READ_PAGE_MAC	0xA5
#define CMD_LOAD_LOCK_SECRET		0x33

static DEFINE_SPINLOCK(ds_lock);
static unsigned long ds_spin_flags;

static struct SHA256_ds_msg	chip_ds_msg;

static unsigned short CRC16;
//--------------------------------------------------------------------------
// Calculate a new CRC16 from the input data shorteger.  Return the current
// CRC16 and also update the global variable CRC16.
//
static short oddparity[16] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

static unsigned short docrc16(unsigned short data)
{
	data = (data ^ (CRC16 & 0xff)) & 0xff;
	CRC16 >>= 8;

	if (oddparity[data & 0xf] ^ oddparity[data >> 4])
		CRC16 ^= 0xc001;

	data <<= 6;
	CRC16  ^= data;
	data <<= 1;
	CRC16   ^= data;

	return CRC16;
}

//set 1 wire bus high
static void enable_ds_pull_up(void)
{
	void *s_pio_sionr2;	
	void *pio_msk2;	
	void *pio_cfgr2;	
	void *pio_sodr2;		

#if 1
	_SC_MAP(s_pio_sionr2, _REG_SET_IO);
	_SC_MAP(pio_msk2, _REG_MASK);
	_SC_MAP(pio_cfgr2, _REG_CFG);
	_SC_MAP(pio_sodr2, _REG_SET_OUT_DATA);

	//avoid glitch on output
	_SC_WRITE(pio_sodr2, _SC_PIN);
	
	//init registers for PIN as output, non security,	
	_SC_WRITE(s_pio_sionr2, _SC_PIN);
	_SC_WRITE(pio_msk2, _SC_PIN);
	_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_OUT);

	//output HIGH
	_SC_WRITE(pio_sodr2, _SC_PIN);

	_SC_UNMAP(s_pio_sionr2);
	_SC_UNMAP(pio_msk2);
	_SC_UNMAP(pio_cfgr2);
	_SC_UNMAP(pio_sodr2);

#else	
	pio_msk2 = ioremap(PIO_MSKR2, 4);	
	pio_cfgr2 = ioremap(PIO_CFGR2, 4);	
	pio_sodr2 = ioremap(PIO_SODR2, 4);	

	//avoid glitch on output
	writel_relaxed(0x02000000, pio_sodr2);
	
	//init registers for PC25 as output, non security,	
	writel_relaxed(0x02000000, s_pio_sionr2);
	writel_relaxed(0x02000000, pio_msk2);
	writel_relaxed(0x00000100, pio_cfgr2);	
	
	//output HIGH
	writel_relaxed(0x02000000, pio_sodr2);	
	
	iounmap(s_pio_sionr2);
	iounmap(pio_msk2);
	iounmap(pio_cfgr2);
	iounmap(pio_sodr2);
#endif

}

//set 1 wire bus as input
static void disable_ds_pull_up(void)
{
	void *s_pio_sionr2;	
	void *pio_msk2;	
	void *pio_cfgr2;	
#if 1
	_SC_MAP(s_pio_sionr2, _REG_SET_IO);
	_SC_MAP(pio_msk2, _REG_MASK);
	_SC_MAP(pio_cfgr2, _REG_CFG);

	//init registers for PIN as input, non security,	
	_SC_WRITE(s_pio_sionr2, _SC_PIN);
	_SC_WRITE(pio_msk2, _SC_PIN);
	_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_IN);


	_SC_UNMAP(s_pio_sionr2);
	_SC_UNMAP(pio_msk2);
	_SC_UNMAP(pio_cfgr2);
#else	
	s_pio_sionr2 = ioremap(S_PIO_SIONR2, 4);	
	pio_msk2 = ioremap(PIO_MSKR2, 4);	
	pio_cfgr2 = ioremap(PIO_CFGR2, 4);		
	//init registers for PC25 as input, non security,	
	writel_relaxed(0x02000000, s_pio_sionr2);
	writel_relaxed(0x02000000, pio_msk2);
	writel_relaxed(0x00000000, pio_cfgr2);		
	iounmap(s_pio_sionr2);
	iounmap(pio_msk2);
	iounmap(pio_cfgr2);
#endif	
}

static int ds_reset(void)
{
	unsigned int u32value;
	int i;
	void *s_pio_sionr2;	
	void *pio_msk2;	
	void *pio_cfgr2;		
	void *pio_codr2;		
	void *pio_pdsr2;
	int ret = 0;	

#if 1
	_SC_MAP(s_pio_sionr2, _REG_SET_IO);
	_SC_MAP(pio_msk2, _REG_MASK);
	_SC_MAP(pio_cfgr2, _REG_CFG);
	_SC_MAP(pio_codr2, _REG_CLEAR_OUT_DATA);
	_SC_MAP(pio_pdsr2, _REG_PIN_DATA_STATUS);

	//avoid glitch
	_SC_WRITE( pio_codr2, _SC_PIN);	
	//init registers for PC25 as output, non security,	
	_SC_WRITE(s_pio_sionr2, _SC_PIN);
	_SC_WRITE(pio_msk2, _SC_PIN);
	_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_OUT);	
	//set to low and keep it about 60us
	_SC_WRITE(pio_codr2, _SC_PIN);		
	udelay(55);
	
	//set PC25 as input to read DS28E15 response
	_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_IN);	
	udelay(3);	//tMSP
	for(i = 0; i < 200; i++)
	{
		udelay(1);
		u32value = _SC_READ(pio_pdsr2) & _SC_PIN;			
		if(u32value == 0)
		{
			ret = 1;							
			break;
		}		
	}

	_SC_UNMAP(s_pio_sionr2);
	_SC_UNMAP(pio_msk2);
	_SC_UNMAP(pio_cfgr2);

	_SC_UNMAP(pio_codr2);
	_SC_UNMAP(pio_pdsr2);
#else	
	s_pio_sionr2 = ioremap(S_PIO_SIONR2, 4);	
	pio_msk2 = ioremap(PIO_MSKR2, 4);	
	pio_cfgr2 = ioremap(PIO_CFGR2, 4);	
	pio_codr2 = ioremap(PIO_CODR2, 4);	
	pio_pdsr2 = ioremap(PIO_PDSR2, 4);
	
	//avoid glitch
	writel_relaxed(0x02000000, pio_codr2);	
	//init registers for PC25 as output, non security,	
	writel_relaxed(0x02000000, s_pio_sionr2);
	writel_relaxed(0x02000000, pio_msk2);
	writel_relaxed(0x00000100, pio_cfgr2);	
	//set to low and keep it about 60us
	writel_relaxed(0x02000000, pio_codr2);		
	udelay(55);	
	//set PC25 as input to read DS28E15 response
	writel_relaxed(0x00000000, pio_cfgr2);	
	udelay(3);	//tMSP
	for(i = 0; i < 200; i++)
	{
		udelay(1);
		u32value = readl_relaxed(pio_pdsr2) & 0x02000000;			
		if(u32value == 0)
		{
			ret = 1;							
			break;
		}		
	}
	
	iounmap(s_pio_sionr2);
	iounmap(pio_msk2);
	iounmap(pio_cfgr2);
	iounmap(pio_codr2);
	iounmap(pio_pdsr2);
#endif	
	udelay(60);			//trsth min is 48us
	return ret;	
}

static void ds_write_byte(unsigned char data)
{
	unsigned char i;
	unsigned char temp;
	void *s_pio_sionr2;	
	void *pio_msk2;	
	void *pio_cfgr2;		
	void *pio_codr2;		
	void *pio_pdsr2;

#if 1
	_SC_MAP(s_pio_sionr2, _REG_SET_IO);
	_SC_MAP(pio_msk2, _REG_MASK);
	_SC_MAP(pio_cfgr2, _REG_CFG);
	_SC_MAP(pio_codr2, _REG_CLEAR_OUT_DATA);
	_SC_MAP(pio_pdsr2, _REG_PIN_DATA_STATUS);

	_SC_WRITE(s_pio_sionr2, _SC_PIN);
	_SC_WRITE(pio_msk2, _SC_PIN);
	//avoid glitch
	_SC_WRITE(pio_codr2, _SC_PIN);
	
	//to protect the ds critical section
	spin_lock_irqsave(&ds_lock, ds_spin_flags);
	for(i = 0; i < 8; i ++) 
	{
		udelay(6);			//tREC, recovery time between write		
		//set for PC25 as output, non security,	
		_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_OUT);	

		temp = data >> i; 	//shift right		
		temp &= 0x01; 		//start from LSB				
		//all communication begins with the master start from pulling data line low
		//write 0
		_SC_WRITE(pio_codr2, _SC_PIN);				
		udelay(1);      	//tW1L 
		ndelay(100);
		
		if(temp)
		{			
			//set PC25 as input, default pull up resistor set signal to 1
			_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_IN);	
		}
		
		udelay(10);			//keep slot time				
		//set PC25 as input
		_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_IN);	
	}

	spin_unlock_irqrestore(&ds_lock, ds_spin_flags);
	
	_SC_UNMAP(s_pio_sionr2);
	_SC_UNMAP(pio_msk2);
	_SC_UNMAP(pio_cfgr2);

	_SC_UNMAP(pio_codr2);
	_SC_UNMAP(pio_pdsr2);
#else

	s_pio_sionr2 = ioremap(S_PIO_SIONR2, 4);	
	pio_msk2  = ioremap(PIO_MSKR2, 4);	
	pio_cfgr2 = ioremap(PIO_CFGR2, 4);	
	pio_codr2 = ioremap(PIO_CODR2, 4);	
	pio_pdsr2 = ioremap(PIO_PDSR2, 4);
	
	writel_relaxed(0x02000000, s_pio_sionr2);
	writel_relaxed(0x02000000, pio_msk2);
	//avoid glitch
	writel_relaxed(0x02000000, pio_codr2);
	//to protect the ds critical section
	spin_lock_irqsave(&ds_lock, ds_spin_flags);
	for(i = 0; i < 8; i ++) 
    {
		udelay(6);			//tREC, recovery time between write		
		//set for PC25 as output, non security,	
		writel_relaxed(0x00000100, pio_cfgr2);	
		
		temp = data >> i; 	//shift right		
		temp &= 0x01; 		//start from LSB				
		//all communication begins with the master start from pulling data line low
		//write 0
		writel_relaxed(0x02000000, pio_codr2);				
		udelay(1);      	//tW1L 
		ndelay(100);
		if(temp)
		{			
			//set PC25 as input, default pull up resistor set signal to 1
			writel_relaxed(0x00000000, pio_cfgr2);	
		}
		udelay(10);			//keep slot time				
		//set PC25 as input
		writel_relaxed(0x00000000, pio_cfgr2);	
    }
	
	spin_unlock_irqrestore(&ds_lock, ds_spin_flags);			  	
	iounmap(s_pio_sionr2);
	iounmap(pio_msk2);
	iounmap(pio_cfgr2);
	iounmap(pio_codr2);
	iounmap(pio_pdsr2);
#endif

}

static unsigned char ds_read_byte(void)
{
	unsigned char i, data = 0x0;
	unsigned int temp;	
	void *s_pio_sionr2;	
	void *pio_msk2;	
	void *pio_cfgr2;	
	void *pio_codr2;		
	void *pio_pdsr2;
#if 1
	_SC_MAP(s_pio_sionr2, _REG_SET_IO);
	_SC_MAP(pio_msk2, _REG_MASK);
	_SC_MAP(pio_cfgr2, _REG_CFG);
	_SC_MAP(pio_codr2, _REG_CLEAR_OUT_DATA);
	_SC_MAP(pio_pdsr2, _REG_PIN_DATA_STATUS);

	_SC_WRITE(s_pio_sionr2, _SC_PIN);
	_SC_WRITE(pio_msk2, _SC_PIN);
	//avoid glitch
	_SC_WRITE(pio_codr2, _SC_PIN);
	
	//to protect the ds critical section
	spin_lock_irqsave(&ds_lock, ds_spin_flags);
	for(i = 0; i < 8; i ++) 
	{			
		udelay(6);			//tREC, recovery time between write, 
		//set PC25 as output
		//init registers for PC25 as output, non security,	
		_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_OUT);	
		//all communication begins with the master start from pulling data line low
		//write 0		
		_SC_WRITE( pio_codr2, _SC_PIN);			
		udelay(1);      	//tRL     //	
		
		//set PC25 as input		
		_SC_WRITE(pio_cfgr2, _SC_PIN_MODE_IN);

		ndelay(500);

		temp = _SC_READ(pio_pdsr2) & _SC_PIN;		
		if(temp)	//bit '1'
		{			
			data |= (0x1 << i);
		}	
		udelay(10);			//tslot minimum is 13us
	}
	
	spin_unlock_irqrestore(&ds_lock, ds_spin_flags);	

	_SC_UNMAP(s_pio_sionr2);
	_SC_UNMAP(pio_msk2);
	_SC_UNMAP(pio_cfgr2);

	_SC_UNMAP(pio_codr2);
	_SC_UNMAP(pio_pdsr2);

#else	
	s_pio_sionr2 = ioremap(S_PIO_SIONR2, 4);	
	pio_msk2 = ioremap(PIO_MSKR2, 4);	
	pio_cfgr2 = ioremap(PIO_CFGR2, 4);	
	pio_codr2 = ioremap(PIO_CODR2, 4);	
	pio_pdsr2 = ioremap(PIO_PDSR2, 4);
	
	writel_relaxed(0x02000000, s_pio_sionr2);
	writel_relaxed(0x02000000, pio_msk2);
	//avoid glitch
	writel_relaxed(0x02000000, pio_codr2);
	//to protect the ds critical section
	spin_lock_irqsave(&ds_lock, ds_spin_flags);
	for(i = 0; i < 8; i ++) 
    {			
		udelay(6);			//tREC, recovery time between write, 
		//set PC25 as output
		//init registers for PC25 as output, non security,	
		writel_relaxed(0x00000100, pio_cfgr2);	
		//all communication begins with the master start from pulling data line low
		//write 0		
		writel_relaxed(0x02000000, pio_codr2);			
		udelay(1);      	//tRL     //		
		//set PC25 as input		
		writel_relaxed(0x00000000, pio_cfgr2);
		
		ndelay(500);
		
		temp = readl_relaxed(pio_pdsr2) & 0x02000000;		
		if(temp)	//bit '1'
		{			
			data |= (0x1 << i);
		}	
		udelay(10);			//tslot minimum is 13us
    }
	
	spin_unlock_irqrestore(&ds_lock, ds_spin_flags);	
	iounmap(s_pio_sionr2);
	iounmap(pio_msk2);
	iounmap(pio_cfgr2);
	iounmap(pio_codr2);
	iounmap(pio_pdsr2);

#endif	
	return data;
}

//compute rom_id crc8
int calc_rom_id_crc(int data_byte, int crc)
{
	int bit_mask = 0, carry_check = 0, temp_data = 0;
	temp_data = data_byte;
	for ( bit_mask = 0; bit_mask <= 7; bit_mask ++)
	{
		data_byte = data_byte ^ crc;
		crc = crc / 2;
		temp_data = temp_data / 2;
		carry_check = data_byte & 0x01;
		if (carry_check)
		{
			crc = crc ^ 0x8C;
		}
		data_byte = temp_data;
	}
	return ( crc );
}

static int ds_read_rom_id(unsigned char *rom_id)
{
	int ret, i;
	int crc_tmp, data_byte;
	memset(rom_id, 0, sizeof(rom_id));
	ret = ds_reset();
	if(ret == 0)				
	{
		printk(KERN_ERR"reset fail");
		return 0;
	}										
	ds_write_byte(CMD_READ_ROM);	
	for(i = 0; i < 8; i++)
		rom_id[i] = ds_read_byte();	
	crc_tmp = 0;
	for(i = 0; i < 7; i++)
	{
		data_byte = rom_id[i];			
		crc_tmp = calc_rom_id_crc(data_byte, crc_tmp);
	}
	
	if((crc_tmp == rom_id[7]) && (rom_id[0] == 0x17))	//check device family code 0x17 for DS28E15
		return 1;
	else
	{
		printk(KERN_ERR"Read rom id error!\n");
		for(i = 0; i < 8; i++)
			printk(KERN_ERR" rom_id[%d] = 0x%x\n", i, rom_id[i]);	
		memset(rom_id, 0, sizeof(rom_id));
		return 0;
	}		
}

//----------------------------------------------------------------------
// Write the 256 bit(32 byte) data to scratchpad with CRC16 verification for DS28E15.  
// 'data'- data to write
//
// Return: 1 - write to scratch verified
//         0 - error writing scratch

static int write_scratch_pad(unsigned char *data)
{
	int i;
	unsigned short read_crc;	

	CRC16 = 0;
	//CMD code
	docrc16(CMD_READ_WRITE_SCRATCHPAD);
	////PB--parameter byte bitmap, BIT3-0:0000 write, 1111 read
	docrc16(0x0);
	
	ds_reset();
	//SKIP_ROM
	ds_write_byte(CMD_ROM_SKIP);	
	ds_write_byte(CMD_READ_WRITE_SCRATCHPAD);		
	ds_write_byte(0x0);	
	
	//first crc	
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);		
		
	if( CRC16 != ( (~read_crc) & 0xffff ) )
	{
		printk(KERN_ERR"write_scratch first CRC error!\n");
		return 0;
	}		
		
	read_crc = 0;
	//data			
	for(i = 0; i < 32; i++ )
		ds_write_byte(data[i]);		
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);		
	CRC16 = 0;		
	for(i = 0; i < 32; i++ )		
		docrc16(data[i]);		
	
	if( CRC16 != ( (~read_crc) & 0xffff ) )	
	{
		printk(KERN_ERR"write_scratch second CRC error!\n");
		return 0;
	}				
	return 1;
}

//----------------------------------------------------------------------
// Read the 256 bit(32 byte) data from scratchpad with CRC16 verification for DS28E15.  
// 'data'- data from scratchpad
//
// Return: 1 - read scratch verified
//         0 - read error 
/*
static int read_scratch_pad(unsigned char *data)
{
	int i;
	unsigned short read_crc;

	CRC16 = 0;
	//CMD code
	docrc16(CMD_READ_WRITE_SCRATCHPAD);
	////PB--parameter byte bitmap, BIT3-0:0000 write, 1111 read
	docrc16(0x0F);
	
	ds_reset();
	//SKIP_ROM
	ds_write_byte(CMD_ROM_SKIP);	
	ds_write_byte(CMD_READ_WRITE_SCRATCHPAD);		
	ds_write_byte(0x0F);	
	
	//first crc	
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);		
		
	if( CRC16 != ( (~read_crc) & 0xffff ) )
	{
		printk(KERN_ERR"read_scratch first CRC error!\n");
		return 0;
	}		
		
	read_crc = 0;
	//data			
	for(i = 0; i < 32; i++ )
		data[i] = ds_read_byte();		
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);		
	CRC16 = 0;		
	for(i = 0; i < 32; i++ )		
		docrc16(data[i]);		
	
	if( CRC16 != ( (~read_crc) & 0xffff ) )	
	{
		printk(KERN_ERR"read_scratch second CRC error!\n");
		return 0;
	}				
	return 1;
}
*/

//--------------------------------------------------------------------------
//  Write a 4 byte memory block. The block location is selected by the 
//  page number and offset blcok within the page. Multiple blocks can
//  be programmed without re-selecting the device using the continue flag. 
//  This function does not use the Authenticated Write operation. 
// 
//  Parameters
//     page - page number, 0 or 1
//	   seg_num seg number, 0 to 7
//     data - 4 byte buffer containing the data to write, B0 B1 B2 B3
//
//  Returns: TRUE - block written
//           FALSE - Failed to write block (no presence or invalid CRC16)
//
int write_block(unsigned char page, unsigned char seg_num, unsigned char *data)
{
	int i;
	unsigned short read_crc;
	unsigned char PB;

	if(page >= 2)
		return 0;
	if(seg_num > 7)
		return 0;
	
	PB = ((seg_num & 0x7) << 5) + (page & 0x1);
	CRC16 = 0;
	//CMD code
	docrc16(CMD_WRITE_MEMORY);
	////PB--parameter byte bitmap, seg 0, page 0
	docrc16(PB);	//get first CRC 
	
	ds_reset();
	//SKIP_ROM
	ds_write_byte(CMD_ROM_SKIP);	
	ds_write_byte(CMD_WRITE_MEMORY);		
	ds_write_byte(PB);	
	
	//first crc	
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);   	
		
	if(CRC16 != ((~read_crc) & 0xffff))
	{
		printk(KERN_ERR"write_block first CRC error! page = %d, block = %d\n", page, seg_num);
		return 0;
	}		
		
	//write 4 byte data
	for(i = 0; i < 4; i++)
		ds_write_byte(data[i]);	
	//second crc
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);   	
	
	CRC16 = 0;
	for(i = 0; i < 4; i++)
		docrc16(data[i]);
	if(CRC16 != ((~read_crc) & 0xffff))
	{
		printk(KERN_ERR"write_block second CRC error! page = %d, block = %d\n", page, seg_num);
		return 0;
	}	
	//send release
	ds_write_byte(0xAA);
	//strong pull up
	enable_ds_pull_up();				
	mdelay(EEPROM_WRITE_DELAY);	
	disable_ds_pull_up();
	read_crc = ds_read_byte();	
	if(read_crc != 0xAA)
	{
		printk(KERN_ERR"write_block CS error!\n");
		return 0;		
	}
	else
		return 1;
}

//write 32 bytes to one page
int write_page(unsigned char page_num, unsigned char *data)
{
	unsigned char block_data[4];
	int i;
	for(i = 0; i < 8; i++)
	{
		memcpy(block_data, &data[i*4], 4);
		if(!write_block(page_num, i, block_data))
		{
			printk(KERN_ERR"Write page %d, block %d error!\n", page_num, i);
			return 0;			
		}
	}
	return 1;
}

//read one page data, from seg0 to seg7, B0 to B3
int read_page(unsigned char page_num, unsigned char *data)
{
	int i;
	unsigned short read_crc;
	unsigned char PB;
	
	if(page_num > 1)
		return 0;

	PB = page_num & 0x1;
	
	CRC16 = 0;
	//CMD code
	docrc16(CMD_READ_MEMORY);
	////PB--parameter byte bitmap, seg 0, page 0
	docrc16(PB);	//get first CRC 
	
	ds_reset();
	//SKIP_ROM
	ds_write_byte(CMD_ROM_SKIP);	
	ds_write_byte(CMD_READ_MEMORY);		
	ds_write_byte(PB);	//PB seg 0, page 0
	
	//first crc	
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + (ds_read_byte() << 8);   	
		
	if(CRC16 != ((~read_crc) & 0xffff))
	{
		printk(KERN_ERR"read_page first CRC error!\n");
		return 0;
	}		
			
	//read one page data, 32 byte
	for(i = 0; i < 32; i++)
		data[i] = ds_read_byte();	
	//second crc
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);   	
	
	CRC16 = 0;
	for(i = 0; i < 32; i++)
		docrc16(data[i]);
	if(CRC16 != ((~read_crc) & 0xffff))
	{
		printk(KERN_ERR"read_page second CRC error!\n");
		return 0;
	}		
	return 1;
}


static int read_status_personality(unsigned char *status)
{
	int i;
	unsigned short read_crc;
	
	CRC16 = 0;
	//CMD code
	docrc16(CMD_READ_STATUS);
	////PB--parameter byte bitmap, read personality 4 bytes
	docrc16(0xE0);	
	
	ds_reset();
	//SKIP_ROM
	ds_write_byte(CMD_ROM_SKIP);	
	ds_write_byte(CMD_READ_STATUS);		
	ds_write_byte(0xE0);	//PB seg 0, page 0
	
	//first crc	
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + (ds_read_byte() << 8);   	
		
	if(CRC16 != ((~read_crc) & 0xffff))
	{
		printk(KERN_ERR"CMD_READ_STATUS first CRC error!\n");
		return 0;
	}		
			
	//read one page data, 32 byte
	for(i = 0; i < 4; i++)
		status[i] = ds_read_byte();	
	//second crc
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);   	
	
	CRC16 = 0;
	for(i = 0; i < 4; i++)
		docrc16(status[i]);
	if(CRC16 != ((~read_crc) & 0xffff))
	{
		printk(KERN_ERR"CMD_READ_STATUS second CRC error!\n");
		return 0;
	}	
	return 1;
}


//--------------------------------------------------------------------------
//  Do Compute Page MAC command and return MAC. Optionally do
//  anonymous mode (anon != 0). 
// 
//  Parameters
//     page_num - page number to read 0 - 16
//     challange - 32 byte buffer containing the challenge
//     mac - 32 byte buffer for page data read
//     anon - Flag to indicate Anonymous mode if (anon != 0)
//
//  Returns: TRUE - page read has correct MAC
//           FALSE - Failed to read page or incorrect MAC
//
int compute_read_page_mac(unsigned char page_num, unsigned char *challenge, unsigned char *mac, unsigned char anon)
{
	unsigned char buf[32];
	int i;
	unsigned short read_crc;	
	unsigned char PB;
	
	if(anon == 1)	//anonymous 
		PB = 0xE0;
	else
		PB = 0x00;	//use ROM ID
	
	PB |= (page_num & 0x1);
	
	// write the challenge 32 bytes to the scratchpad
	if(!write_scratch_pad(challenge))
	{
		printk(KERN_ERR"compute_read_page_mac write_scratch_pad fail.\n");
		return 0;
	}
	
	CRC16 = 0;
	//CMD code
	docrc16(CMD_COMPUTE_READ_PAGE_MAC);
	////PB--parameter byte bitmap, BIT3-0:0000 write, 1111 read
	docrc16(PB);
	
	ds_reset();
	//SKIP_ROM
	ds_write_byte(CMD_ROM_SKIP);	
	ds_write_byte(CMD_COMPUTE_READ_PAGE_MAC);		
	ds_write_byte(PB);	
	
	//first crc	
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);		
		
	if( CRC16 != ( (~read_crc) & 0xffff ) )
	{
		printk(KERN_ERR"CMD_COMPUTE_READ_PAGE_MAC first CRC error!\n");
		return 0;
	}		

	//wait for 2*tcsha
	mdelay(2*SHA_COMPUTATION_DELAY);
	
	//read cs
	read_crc = ds_read_byte();
	if(read_crc != 0xAA)
	{
		printk(KERN_ERR"read cs error! 0x%x\n", read_crc);
		return 0;
	}
	
	//read 32 byte MAC 
	for(i = 0; i < 32; i++ )
		buf[i] = ds_read_byte();		
	
	read_crc = ds_read_byte();
	read_crc = read_crc + (ds_read_byte() << 8);		
	CRC16 = 0;		
	for(i = 0; i < 32; i++ )		
		docrc16(buf[i]);		
	
	if(CRC16 != ((~read_crc) & 0xffff ))	
	{
		printk(KERN_ERR"CMD_COMPUTE_READ_PAGE_MAC second CRC error!\n");
		return 0;
	}
	else
		memcpy(mac, buf, 32);	
	return 1;	
}

//----------------------------------------------------------------------
// load and lock the 256 bit(32 byte) secret to chip
// 'secret'- secret to load
// 'lock_flag' - 1: lock , 0: no lock
// 'magic - 0x5A, protect lock_flag
//
// Return: 1 - load&lock secret ok
//         0 - error load&lock secret

static int load_lock_secret(unsigned char *secret, unsigned lock_flag, unsigned char lock_magic)
{
	unsigned short read_crc;	
	unsigned char PB;
	
	if(!write_scratch_pad(secret))
	{
		printk(KERN_ERR"CMD_LOAD_LOCK_SECRET write_scratch_pad error!\n");
		return 0;
	}	
		
	if((lock_flag == 1)&&(lock_magic == 0x5A))
		PB = 0xE0;
	else
		PB = 0x00;

	CRC16 = 0;
	//CMD code
	docrc16(CMD_LOAD_LOCK_SECRET);
	////PB--parameter byte bitmap, BIT3-0:0000 write, 1111 read
	docrc16(PB);
	
	ds_reset();
	//SKIP_ROM
	ds_write_byte(CMD_ROM_SKIP);	
	ds_write_byte(CMD_LOAD_LOCK_SECRET);		
	ds_write_byte(PB);	
	
	//first crc	
	read_crc = 0;
	read_crc = ds_read_byte();
	read_crc = read_crc + ( ds_read_byte() << 8);		
		
	if( CRC16 != ( (~read_crc) & 0xffff ) )
	{
		printk(KERN_ERR"CMD_LOAD_LOCK_SECRET first CRC error!\n");
		return 0;
	}		
	
	//send release
	ds_write_byte(0xAA);
	//strong pull up
	enable_ds_pull_up();				
	//delay tprs
	mdelay(SECRET_PROGRAM_DELAY);	
	disable_ds_pull_up();
	read_crc = ds_read_byte();	
	if(read_crc == 0xAA)
		return 1;
	else if(read_crc == 0x55)
	{
		printk(KERN_ERR"CMD_LOAD_LOCK_SECRET secret was locked before.\n");
		return 0;
	}
	else
	{
		printk(KERN_ERR"CMD_LOAD_LOCK_SECRET fail.\n");
		return 0;
	}			
}

static int ds_open(struct inode * inode, struct file * file)
{	
	return 0;
}

static int ds_close(struct inode * inode, struct file * file)
{
    return 0;
}

static long ds_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	unsigned char	rom_id[8];	
	unsigned char	secret[8];			
	unsigned char 	challenge[32];
	unsigned char	page_data[32];		//memory page data, 32 byte
	unsigned char 	personality_byte[4];//
	unsigned char   read_mac[32];		//MAC from chip
	unsigned char 	page_num;			//page number
	unsigned char 	lock_flag;			//1--lock chip secret forever
	unsigned char 	lock_magic;				
	unsigned char   anon;				//anonymous flag. 1--anonymous mode, 0-NOT anonymous mode
	int i;
	int ret;
	
	memset(page_data, 0, sizeof(page_data));
	ret = 0;	
	switch(cmd)  
    {  
		case	SHA256_CHIP_TIMING_INIT:  					
				for(i = 0; i < 100; i++)
					ds_read_rom_id(rom_id);
				udelay(10);
				read_page(0, page_data);
				udelay(10);
				read_page(1, page_data);				
				break;
				
		case	SHA256_READ_ROM_ID:  					
				ds_read_rom_id(rom_id);
				memcpy(&chip_ds_msg.rom_id[0], &rom_id[0], sizeof(rom_id));
				ret = copy_to_user((struct ds_msg __user *)arg, &chip_ds_msg.rom_id[0], sizeof(chip_ds_msg));
				if(ret != 0)
					return -EACCES;				
				break;
				
		case 	SHA256_INIT_CHIP_PRODUCTION_KEY:				
				ret = copy_from_user(&chip_ds_msg, (struct ds_msg __user *)arg, sizeof(chip_ds_msg));
				if(ret != 0)			
				{
					printk(KERN_ERR"INIT_CHIP_PRODUCTION_KEY copy from user error! ret = %d\n", ret);
					return -EACCES;				
				}				
				ds_read_rom_id(rom_id);
				//ds_reset();
				memset(page_data, 0, sizeof(page_data));				
				//write_page(0, page_data);
				udelay(10);
				//write_page(1, page_data);
				ds_reset();
				memcpy(page_data, &chip_ds_msg.page_0_data[0], 32);				
				if(!write_page(0, page_data))
				{
					printk(KERN_ERR"INIT_CHIP_PRODUCTION_KEY Write page 0 data to chip FAIL!\n");
					return -EACCES;						
				}					
				
				//write page 1 data to chip, page 1 data has MAC address inside, it is written in PRODUCTION_MAC command
				//memcpy(page_data, &chip_ds_msg.page_1_data[0], 32);
				//if(!write_page(1, page_data))
				//{
				//	printk(KERN_ERR"INIT_CHIP_PRODUCTION Write page data to chip FAIL!\n");
				//	return -EACCES;						
				//}	
				
				//load and lock secret		
				lock_flag = chip_ds_msg.lock_flag;
				lock_magic = chip_ds_msg.lock_magic;
				memcpy(secret, &chip_ds_msg.secret[0], 32);				
				if(!load_lock_secret(secret, lock_flag, lock_magic))
				{
					printk(KERN_ERR"INIT_CHIP_PRODUCTION_KEY load lock secret to chip FAIL!\n");
					return -EACCES;						
				}												
				break;
				

		case 	SHA256_INIT_CHIP_PRODUCTION_MAC:				
				ret = copy_from_user(&chip_ds_msg, (struct ds_msg __user *)arg, sizeof(chip_ds_msg));
				if(ret != 0)			
				{
					printk(KERN_ERR"INIT_CHIP_PRODUCTION_MAC copy from user error! ret = %d\n", ret);
					return -EACCES;				
				}				
				ds_read_rom_id(rom_id);
				//ds_reset();
				memset(page_data, 0, sizeof(page_data));				
				//write_page(0, page_data);
				udelay(10);
				//write_page(1, page_data);
				ds_reset();							
				//PAGE 1 structure	
				//[0-5] 6 byte MAC address, [10]-- model_id, [20]--production_name_id, [31]--CRC8 of [0]~[30])				
				memcpy(page_data, &chip_ds_msg.page_1_data[0], 32);
				if(!write_page(1, page_data))
				{
					printk(KERN_ERR"INIT_CHIP_PRODUCTION_MAC Write page 1 data to chip FAIL!\n");
					return -EACCES;						
				}						
				break;				
				
		case	SHA256_READ_CHIP_MEM:  				
				ret = copy_from_user(&chip_ds_msg, (struct ds_msg __user *)arg, sizeof(chip_ds_msg));
				if(ret != 0)			
				{
					printk(KERN_ERR"READ_CHIP_MEM copy from user FAIL!\n");
					return -EACCES;				
				}						
				memset(page_data, 0, sizeof(page_data));				
				ds_read_rom_id(rom_id);
				if(!read_page(0, page_data))
				{
					printk(KERN_ERR"READ_CHIP_MEM read page 0 FAIL!\n");
					return -EACCES;				
				}	
				memcpy(&chip_ds_msg.page_0_data[0], page_data, 32);
				if(!read_page(1, page_data))
				{
					printk(KERN_ERR"READ_CHIP_MEM read page 1 FAIL!\n");
					return -EACCES;				
				}	
				memcpy(&chip_ds_msg.page_1_data[0], page_data, 32);
				if(!read_status_personality(personality_byte))
				{
					printk(KERN_ERR"READ_CHIP_MEM read personality FAIL!\n");
					return -EACCES;				
				}
				chip_ds_msg.man_id[0] = personality_byte[2];
				chip_ds_msg.man_id[1] = personality_byte[3];				
				
				ret = copy_to_user((struct ds_msg __user *)arg, &chip_ds_msg.rom_id[0], sizeof(chip_ds_msg));
				if(ret != 0)			
				{
					printk(KERN_ERR"READ_CHIP_MEM copy to user FAIL!\n");
					return -EACCES;				
				}			
				break;					
				
				
		case	SHA256_COMPUTE_READ_MAC:				
				ret = copy_from_user(&chip_ds_msg, (struct ds_msg __user *)arg, sizeof(chip_ds_msg));
				if(ret != 0)			
				{
					printk(KERN_ERR"COMPUTE_READ_MAC copy from user FAIL!\n");
					return -EACCES;				
				}								
				page_num = chip_ds_msg.page_num;
				anon = chip_ds_msg.anon;
				memcpy(challenge, &chip_ds_msg.challenge[0], 32);				
				ret = compute_read_page_mac(page_num, challenge, read_mac, anon);				
				if(ret == 0)
				{
					printk(KERN_ERR"COMPUTE_READ_MAC read_auth_page FAIL!\n");
					return -EACCES;
				}
				memcpy(&chip_ds_msg.mac[0], read_mac, 32);				
				ret = copy_to_user((struct ds_msg __user *)arg, &chip_ds_msg.rom_id[0], sizeof(chip_ds_msg));
				if(ret != 0)			
				{
					printk(KERN_ERR"COMPUTE_READ_MAC copy to user FAIL!\n");
					return -EACCES;				
				}			
				break;										
			
		default: 
				return -EINVAL;  					
    } 
	return 0;	
}

static struct file_operations ds_fops = 
{
    .owner      = THIS_MODULE,
    .open       = ds_open,
    .release    = ds_close,
    .unlocked_ioctl = ds_ioctl,
};

static struct miscdevice ds_dev = 
{   
    .minor		= MISC_DYNAMIC_MINOR,
    .name		= MUX_DRV_SECURE_CHIP_NAME,
    .fops  		= &ds_fops,
};

static int __init ds_init(void)
{		
	if (misc_register(&ds_dev))
	{
		printk(KERN_ERR"ERROR: could not register muxds devices\n");
		return -1;
	}

	EXT_INFOF(EXT_SYSTEM" DS28E15 initlializing...");
	return 0;
}
static void __exit ds_exit(void)
{
	misc_deregister(&ds_dev);	
}

module_init(ds_init);
module_exit(ds_exit);
MODULE_LICENSE("GPL");
