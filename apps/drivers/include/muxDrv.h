/*
* driver header for user space applications
*/

#ifndef __MUX_DRV_H__
#define __MUX_DRV_H__

#define	DEV_ROOT_NAME							"/dev/"

#define	MUX_DRV_DIP_SWITCH_MAGIC			'S'
#define	MUX_DRV_DIP_SWITCH_READ				_IOR(MUX_DRV_DIP_SWITCH_MAGIC, 1, unsigned char *)  

#define	MUX_DRV_LED_VIDEO_MAGIC				'V'
#define	MUX_DRV_LED_VIDEO_OP_OFF  			_IO(MUX_DRV_LED_VIDEO_MAGIC, 0)  
#define	MUX_DRV_LED_VIDEO_OP_ON 			_IO(MUX_DRV_LED_VIDEO_MAGIC, 1)


//PB0 Video LED
//PB4 Act LED
//PB5 Power LED
//PB7 Link LED

#define DS_MAGIC 'D'
#define SHA256_READ_ROM_ID				_IOWR(DS_MAGIC, 1, unsigned int *)  
#define SHA256_INIT_CHIP_PRODUCTION_KEY	_IOWR(DS_MAGIC, 2, unsigned int *)  
#define SHA256_INIT_CHIP_PRODUCTION_MAC	_IOWR(DS_MAGIC, 3, unsigned int *)  
#define SHA256_READ_CHIP_MEM				_IOWR(DS_MAGIC, 4, unsigned int *)  
#define SHA256_COMPUTE_READ_MAC			_IOWR(DS_MAGIC, 5, unsigned int *)  
#define SHA256_CHIP_TIMING_INIT			_IOWR(DS_MAGIC, 6, unsigned int *)  

struct SHA256_ds_msg 
{
	unsigned char rom_id[8];		//7 byte id(start from 0x17) + 1 byte CRC
	unsigned char secret[32];		//secret write to chip
	unsigned char challenge[32];	//random data write to chip
	unsigned char mac[32];			//mac data read from chip
	unsigned char page_0_data[32];	//page 0 data write to/read from chip
	unsigned char page_1_data[32];	//page 1 data write to/read from chip
	unsigned char man_id[2];		//manufacturer ID
	unsigned char page_num;			//page number, which page is used for MAC 
	unsigned char anon;				//Anonymous Indicator(ANON). These bits specify whether the device’s ROM ID is used for the MAC computation
									//1--anonymous mode, 0--NOT anonymous mode.
	unsigned char lock_flag;		//1-lock secret, secret is fixed for ever	
	unsigned char lock_magic;		//when (logic_magic == 0x5A)&&(lock_flag == 1), lock the secret
};


#define	MUX_DRV_DIP_SWITCH_NAME				"muxDip"
#define	MUX_DRV_DIP_SWITCH_DEV_NAME		DEV_ROOT_NAME MUX_DRV_DIP_SWITCH_NAME


#define	MUX_DRV_LED_VIDEO_NAME				"muxLedVideo"
#define	MUX_DRV_LED_VIDEO_DEV_NAME			DEV_ROOT_NAME MUX_DRV_LED_VIDEO_NAME


#define	MUX_DRV_SECURE_CHIP_NAME				"muxds"
#define	MUX_DRV_SECURE_CHIP_DEV_NAME			DEV_ROOT_NAME MUX_DRV_SECURE_CHIP_NAME


#endif

