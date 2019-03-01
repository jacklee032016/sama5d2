/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : I2C sample code
* Abstract : 
* Author : Zhenyang_jin (zhenyang_jin@realsil.com.cn)                
*  $Id: I2c,v 1.0 2010-04-22 13:04:19 zhenyang Exp $
*/


#include <rtk_types.h>
#include <rtk_error.h>

#include <rtk_i2c.h>

//#include "TWI_Master_Helper_Funcs.h"

//extern  struct _twi_desc twid0;



/* Sample Code will be embraced by this macro.
    when porting to new platform, the sample code will be rewritten */     
//#define SAMPLE_CODE     

#ifdef SAMPLE_CODE 
    #include "stdafx.h"
    #include "io.h"     /*Provide IO Read/Write API: PortOut() & PortIn() */

    #define DATA_PORT       (0x378) /* Temporary port number in sample code */
    #define STATUS_PORT     (0x379) /* Temporary port number in sample code */

    /* user doesn't need to take care of magic bits */
    #define MAGIC_BITS0     (0x1c)  
    #define MAGIC_BITS1     (0x18)
#endif

#define RTL8305			0x50
#define DEVICE_ID       (4)

#define SET_SCL_0_SDA_0  _SCL_SDA_set(0, 0)
#define SET_SCL_0_SDA_1  _SCL_SDA_set(0, 1)
#define SET_SCL_1_SDA_0  _SCL_SDA_set(1, 0)
#define SET_SCL_1_SDA_1  _SCL_SDA_set(1, 1)

#define SET_SCL_0        _SCL_set(0)
#define SET_SCL_1        _SCL_set(1)

#define __REV(a)   (((a>>24) & 0xff) | \
					((a>>8)  & 0xff00) | \
					((a<<8)  & 0xff0000) | \
					((a<<24) & 0xff000000) )

typedef enum I2C_Oper_e {
    I2C_WRITE_CMD,
    I2C_READ_CMD,
}I2C_Oper_t;

typedef enum GPIO_Mode_e {
    GPIO_INPUT,
    GPIO_OUTPUT,
}GPIO_Mode_t;

/*
    Low level APIs. rewrite the API if porting to other platform.
    The APIs that user need to rewritten are:
        _gpio_sda_mode_set();
        _SCL_SDA_set();
        _SCL_set();
        _SDA_get();
        _wait_half_CLK();
        RTL8307H_I2C_init();
            
    By the way, RTL8307H_I2C_READ()/RTL8307H_I2C_WRITE() may need 
    synchronization mechanism when they are used by several kernel
    control path in the OS.
*/

#if 0


/* Function Name:
 *      _wait_half_CLK
 * Description:
 *      Keep the signal for a moment to make sure the frequency of the SCL should be no more than 8MHZ.
 * Input:
 *      None
 * Output:     
 *      None
 * Return: 
 *      None  
 * Note:
 *      None
 */
void _wait_half_CLK(void)
{
#ifdef SAMPLE_CODE 
#endif
} 

/* Function Name:
 *      _gpio_sda_mode_set
 * Description:
 *      Change the mode of GPIO used as SDA:
 *          GPIO_INPUT,
 *          GPIO_OUTPUT
 * Input:
 *      mode                    -  GPIO_INPUT or GPIO_OUTPUT
 * Output:     
 *      None
 * Return: 
 *      None  
 * Note:
 *      None
 */
void _gpio_sda_mode_set(GPIO_Mode_t mode) 
{
#ifdef SAMPLE_CODE 
#endif    
}

/* Function Name:
 *      _SCL_SDA_set
 * Description:
 *      Set the level of GPIOs used as SCL & SDA, keep the signal for a monment.
 *      The frequency of the SCL should be no more than 8MHZ.
 * Input:
 *      scl                    -  output level low/high on scl(0:low, 1:high)
 *      sda                    -  output level low/high on sda(0:low, 1:high)
 * Output:     
 *      None
 * Return: 
 *      None  
 * Note:
 *      None
 */
void _SCL_SDA_set(char scl, char sda)
{
#ifdef SAMPLE_CODE
    char val;
    val = scl | (sda << 1) | MAGIC_BITS0;
    PortOut(DATA_PORT,val);

    //printf("set %08x\n", val);
    /* delay about holdtime */
    _wait_half_CLK();
#endif    
}

/* Function Name:
 *      _SCL_set
 * Description:
 *     This API is used to output the SCL signal when reading SDA. keep the signal for a monment.
 *      The frequency of the SCL should be no more than 8MHZ.
 * Input:
 *      scl                    -  output level low/high on scl(0:low, 1:high)
 * Output:     
 *      None
 * Return: 
 *      None  
 * Note:
 *      None
 */
void _SCL_set(char scl)
{
#ifdef SAMPLE_CODE
    char val;
    val = scl | MAGIC_BITS1;
    PortOut(DATA_PORT,val);

    /* delay about holdtime */
    _wait_half_CLK();
#endif    
}

/* Function Name:
 *      _SDA_get
 * Description:
 *     This API is used to get the SDA
 * Input:
 *      None
 * Output:     
 *      the level on the GPIO used as SDA. (0:low level, 1:high level)
 * Return: 
 *      None  
 * Note:
 *      None
 */
unsigned char _SDA_get(void)
{
    unsigned char val;
#ifdef SAMPLE_CODE

    val = ((unsigned char)PortIn(STATUS_PORT) >> 3) & 0x01;

    /* delay about holdtime */
    _wait_half_CLK();
#endif    
    return val;
}
#endif

/* Function Name:
 *     RTL8307H_I2C_init
 * Description:
 *     Initializtion. Normally, the GPIO of SDA and SCL in the master 
 *     should be configured as output
 * Input:
 *      None
 * Output:     
 *      None
 * Return: 
 *      None  
 * Note:
 *      None
 */
void RTL8307H_I2C_init(void)
{ 
#ifdef SAMPLE_CODE
    PortOut(DATA_PORT,0); /* set I2C power disable */
#endif
}

#if 0

/*  
    _________________
SDA         1
         _______     
SCL ____|       |____    

*/
void out_bit_1(void)
{
    SET_SCL_0_SDA_1;                       
    SET_SCL_1_SDA_1;      
    SET_SCL_1_SDA_1;  
    SET_SCL_0_SDA_1;    
}

/*  
    
SDA ________0________
         _______     
SCL ____|       |____    

*/
void out_bit_0(void)
{
    SET_SCL_0_SDA_0;                       
    SET_SCL_1_SDA_0;      
    SET_SCL_1_SDA_0;  
    SET_SCL_0_SDA_0;    
}

/*  Start Command 
    ________
SDA         |________
         _______     
SCL ____|       |____    

*/
void RTL8307H_I2C_START(void)
{
    SET_SCL_0_SDA_1;                       
    SET_SCL_1_SDA_1;      
    SET_SCL_1_SDA_0;  
    SET_SCL_0_SDA_0;
}

/*  Stop Command
            ________
SDA  ______|

         _______     
SCL ____|       |____         
 */
void RTL8307H_I2C_STOP(void)
{
    SET_SCL_0_SDA_0;                       
    SET_SCL_1_SDA_0;      
    SET_SCL_1_SDA_1;  
    SET_SCL_0_SDA_1;

    // Add one clock cycle
    SET_SCL_0_SDA_1;                       
    SET_SCL_1_SDA_1;      
    SET_SCL_1_SDA_1;  
    SET_SCL_0_SDA_1;
}
/*
    write the control byte 
 */
void RTL8307H_I2C_WR_Ctrl(I2C_Oper_t rw)
{
    out_bit_1();
    out_bit_0();
    out_bit_1();
    out_bit_0();

    /* output device id */
    if (DEVICE_ID&0x4)
        out_bit_1();
    else
        out_bit_0();

    if (DEVICE_ID&0x2)
        out_bit_1();
    else
        out_bit_0();        
    
    if (DEVICE_ID&0x1)
        out_bit_1();
    else
        out_bit_0(); 

    if (rw == I2C_WRITE_CMD)
        out_bit_0();
    else
        out_bit_1();    
}

void RTL8307H_I2C_WR_8BIT(unsigned char data)
{
    if (data & 0x80)    // MSB
        out_bit_1();
    else
        out_bit_0();

    if (data & 0x40)
        out_bit_1();
    else
        out_bit_0();

    if (data & 0x20)
        out_bit_1();
    else
        out_bit_0();  

    if (data & 0x10)
        out_bit_1();
    else
        out_bit_0();

    if (data & 0x08)
        out_bit_1();
    else
        out_bit_0();

    if (data & 0x04)
        out_bit_1();
    else
        out_bit_0(); 

    if (data & 0x02)
        out_bit_1();
    else
        out_bit_0();

    if (data & 0x01)    // LSB
        out_bit_1();
    else
        out_bit_0();   
}

unsigned char RTL8307H_I2C_RD_DATA(void)
{
    int bit_count;
    unsigned char InData = 0;
    unsigned char sda;

    _gpio_sda_mode_set(GPIO_INPUT);
    
    for(bit_count = 8; bit_count > 0; bit_count--) // from MSB to LSB
    {
        SET_SCL_0;                       
        SET_SCL_1; 
        
        sda = _SDA_get();
        
        InData = (unsigned char)((InData<<1) + sda);

        SET_SCL_1;                       
        SET_SCL_0; 
    }

    _gpio_sda_mode_set(GPIO_OUTPUT);
    
    return InData;
}

/*
    check ack signal from eeprom 
 */
int ack_check(void)
{
    unsigned char sda;
    int time_out;

    time_out = 0;

    _gpio_sda_mode_set(GPIO_INPUT);
    
    SET_SCL_0;                       
    SET_SCL_1; 

    do {
        sda = _SDA_get();

        if(time_out++ > 8)          /*  time out    */
        {
            SET_SCL_0;   
            _gpio_sda_mode_set(GPIO_OUTPUT);
			RTL8307H_I2C_STOP();    /*  stop if no ACK    */ 
			return RT_ERR_NO_ACK;
        }
    }while(sda != 0);   /*  if sda = 0, get ACK    */  

    SET_SCL_0;   
    _gpio_sda_mode_set(GPIO_OUTPUT);
    
	return RT_ERR_OK;
}


#endif


/* Function Name:
 *      RTL8307H_I2C_READ
 * Description:
 *      Read the data with address switch_addr. 
 *      The MSB of *reg_val (i.e. (*reg_val >> 31) & 0x1) corresponds to bits 31:31
 *      in the register datasheet
 * Input:
 *      switch_addr               -  switch address
 * Output:     
 *      reg_val                   -  Register value
 * Return: 
 *      RT_ERR_OK  
 *      RT_ERR_NO_ACK
 * Note:
 *      None
 */
int32 RTL8307H_I2C_READ(uint32 switch_addr, uint32 *reg_val)
{    
	*reg_val = 0;
#if 0
uint8_t	data[4] = {0xde, 0xad, 0xbe, 0xef};
struct 	_buffer in;

	twi_select_bus(TWI_CH3);

	twid0.slave_addr = (RTL8305 | DEVICE_ID);;
	twid0.iaddr = (__REV(switch_addr)>>8);
	twid0.isize = 3;
	twid0.mutex = 0;
	
	in.data = (uint8_t*)data;
	in.size = 4;
	
	twid_transfert(&twid0, &in, NULL, &twid0_free_mutex, (void *)&twid0.mutex); 

	if (reg_val != NULL) {
		*reg_val = 0;
		for (int i = 0; i < 4; i++)  { *reg_val = (data[i]<<(i*8)) | *reg_val; } 
	}
#endif	
	return RT_ERR_OK;
}

/* Function Name:
 *      RTL8307H_I2C_WRITE
 * Description:
 *      Write the data at address switch_addr. 
 *      The MSB of reg_val (i.e. (*reg_val >> 31) & 0x1) corresponds to bits 31:31
 *      in the register datasheet
 * Input:
 *      switch_addr               -  switch address   
 *      reg_val                   -  Register value
 * Output:  
 *      None
 * Return: 
 *      RT_ERR_OK  
 *      RT_ERR_NO_ACK
 * Note:
 *      None
 */
int32 RTL8307H_I2C_WRITE(uint32 switch_addr, uint32 reg_val)
{
#if 0	
uint8_t	data[4];
struct 	_buffer out;

	twi_select_bus(TWI_CH3);

	twid0.slave_addr = (RTL8305 | DEVICE_ID);;
	twid0.iaddr = (__REV(switch_addr)>>8);
	twid0.isize = 3;
	twid0.mutex = 0;
	data[0] = reg_val;
	data[1] = reg_val>>8;
	data[2] = reg_val>>16;
	data[3] = reg_val>>24;
	
	out.data = (uint8_t*)data;
	out.size = 4;
	
	twid_transfert(&twid0, NULL, &out, &twid0_free_mutex, (void *)&twid0.mutex); 
#endif		
	return RT_ERR_OK; 
}








