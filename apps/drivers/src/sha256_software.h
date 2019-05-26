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


int ComputeSHA256(uchar* message, short length, ushort skipconst, ushort reverse, uchar* digest);
int VerifyMAC256(uchar* MT, short lenght, uchar* compare_MAC);
int ComputeMAC256(uchar* message, short length, uchar* MAC);
int CalculateNextSecret256(uchar* binding, uchar* partial, int page_num, uchar* man_id, uchar* rom_no);
void set_secret(uchar *secret);
int Compute_Software_Mac(unsigned char page_num, unsigned char *page_data, unsigned char *challenge, unsigned char *rom_id, unsigned char *secret, unsigned char *man_id, unsigned char anon, unsigned char *chip_mac);


#endif
