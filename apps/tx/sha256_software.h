/*---------------------------------------------------------------------------
 *  Copyright (C) 2011 Maxim Integrated Products, All Rights Reserved.
 * 
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 * 
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY CLAIM, DAMAGES
 *  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 * 
 *  Except as contained in this notice, the name of Maxim Integrated Products
 *  shall not be used except as stated in the Maxim Integrated Products
 *  Branding Policy.
 * ---------------------------------------------------------------------------
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include "system_info.h"

#ifndef __SHA_H_
#define __SHA_H_



#ifndef uchar
   typedef unsigned char uchar;
#endif

#ifndef ushort
   typedef unsigned short ushort;
#endif

#ifndef ulong
   typedef unsigned long ulong;
#endif

#define DS_MAGIC 'D'
#define SHA256_READ_ROM_ID				_IOWR(DS_MAGIC, 1, unsigned int *)  
#define SHA256_INIT_CHIP_PRODUCTION_KEY	_IOWR(DS_MAGIC, 2, unsigned int *)  
#define SHA256_INIT_CHIP_PRODUCTION_MAC	_IOWR(DS_MAGIC, 3, unsigned int *)  
#define SHA256_READ_CHIP_MEM			_IOWR(DS_MAGIC, 4, unsigned int *)  
#define SHA256_COMPUTE_READ_MAC			_IOWR(DS_MAGIC, 5, unsigned int *)  
#define SHA256_CHIP_TIMING_INIT			_IOWR(DS_MAGIC, 6, unsigned int *)  

struct SHA256_DS_MSG 
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




int ComputeSHA256(uchar* message, short length, ushort skipconst, ushort reverse, uchar* digest);
int VerifyMAC256(uchar* MT, short lenght, uchar* compare_MAC);
int ComputeMAC256(uchar* message, short length, uchar* MAC);
int CalculateNextSecret256(uchar* binding, uchar* partial, int page_num, uchar* man_id, uchar* rom_no);
void SHA256_Set_Secret(uchar *secret);
int SHA256_Compute_Software_Mac(unsigned char page_num, unsigned char *page_data, unsigned char *challenge, unsigned char *rom_id, unsigned char *secret, unsigned char *man_id, unsigned char anon, unsigned char *chip_mac);
int SHA256_Calc_Rom_Id_Crc(int data_byte, int crc);
int SHA256_Compute_Software_Mac(unsigned char page_num, unsigned char *page_data, unsigned char *challenge, unsigned char *rom_id, unsigned char *secret, unsigned char *man_id, unsigned char anon, unsigned char *chip_mac);
int SHA256_App_Check(void);
int SHA256_Read_Rom_Id(unsigned char *rom_id);
int SHA256_Write_Key(unsigned char *key);	
int SHA256_Read_MacAddr(char *mac_addr_str);
int SHA256_Write_MacAddr_Once(char *mac_addr_str);
int SHA256_Write_ModelID_Once(char *modelidstr);
void SHA256_StrToHex(char *pbDest, char *pbSrc, int nLen);
void SHA256_HexToStr(char *pbDest, char *pbSrc, int nLen);
int SHA256_Read_Mac_ModelID_Once(char *mac_addr_str, char *modelidstr);
int SHA256_Write_Mac_ModelID_Once(char *mac_addr_str, char *modelidstr);


#endif
