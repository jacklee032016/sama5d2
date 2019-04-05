//------------Copyright (C) 2013 Maxim Integrated Products --------------
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL MAXIM INTEGRATED PRODCUTS BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Maxim Integrated Products
// shall not be used except as stated in the Maxim Integrated Products
// Branding Policy.
// ---------------------------------------------------------------------------
//
// sha256_software.c - SHA-256 implemenation in software for Maxim devices.
//



#include "sha256_software.h"

#define SHA_256_INITIAL_LENGTH    8

#define TRUE 1
#define FALSE 0

/*
// SHA-256 Functions
int ComputeSHA256(uchar* message, short length, ushort skipconst, ushort reverse, uchar* digest);
int ComputeMAC256(uchar* message, short length, uchar* MAC);
int VerifyMAC256(uchar* message, short length, uchar* compare_MAC);
int CalculateNextSecret256(uchar* binding, uchar* partial, int page_num, uchar* manid, uchar* rom_no);
void set_secret(uchar *secret);
*/

// Utility Functions
ulong sha_ch(ulong x, ulong y, ulong z);
ulong sha_maj(ulong x, ulong y, ulong z);
ulong sha_rotr_32(ulong val, ushort r);
ulong sha_shr_32(ulong val, ushort r);
ulong sha_bigsigma256_0(ulong x);
ulong sha_littlesigma256_0(ulong x);
ulong sha_littlesigma256_1(ulong x);
void sha_copy32(ulong* p1, ulong* p2, ushort length);
void sha_copyWordsToBytes32(ulong* input, uchar* output, ushort numwords);
void sha_writeResult(ushort reverse, uchar* outpointer);
ulong sha_getW(int index);
void sha_prepareSchedule(uchar* message);
void sha256_hashblock(uchar* message, ushort lastblock);

//extern int dprintf(char *format, ...);

// hold secret for creating a 
static uchar SECRET[32];

// SHA-256 globals values
ulong SHA_256_Initial[] = 
{
   0x6a09e667,
   0xbb67ae85,
   0x3c6ef372,
   0xa54ff53a,
   0x510e527f,
   0x9b05688c,
   0x1f83d9ab,
   0x5be0cd19
};

ulong SHA_CONSTANTS[] =  
{
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
  0xca273ece, 0xd186b8c7, 0xeada7dd6, 0xf57d4f7f, 0x06f067aa, 0x0a637dc5, 0x113f9804, 0x1b710b35,
  0x28db77f5, 0x32caab7b, 0x3c9ebe0a, 0x431d67c4, 0x4cc5d4be, 0x597f299c, 0x5fcb6fab, 0x6c44198c
};

uchar workbuffer[128];

ulong a32, b32, c32, d32, e32, f32, g32, h32; // SHA working variables
ulong W32[16];                                // SHA message schedule
ulong H32[8];                                 // last SHA result variables
 
int reverse_endian=1; 
int sha_debug=0;

//----------------------------------------------------------------------
// Set the Secret to be used for calculating MAC's in the ComputeMAC
// function.
//
void set_secret(uchar *secret)
{ 
   int i;

   for (i = 0; i < 32; i++)
      SECRET[i] = secret[i];   
}

//----------------------------------------------------------------------
// Computes SHA-256 given the MT digest buffer after first iserting
// the secret at the correct location in the array defined by all Maxim
// devices. Since part of the input is secret it is called a Message
// Autnentication Code (MAC).
//
// 'MT'       - buffer containing the message digest
// 'length'   - Length of block to digest
// 'MAC'      - result MAC in byte order used by 1-Wire device
//
int ComputeMAC256(uchar* MT, short length, uchar* MAC)
{
   int i,j;  
   uchar tmp[4]; 

   // check for two block format
   if (length == 119)
   {
      // insert secret
      memcpy(&MT[64], &SECRET[0], 32);

      // change to little endian for A1 devices
      if (reverse_endian)
      {
         for (i = 0; i < 108; i+=4)
         {
            for (j = 0; j < 4; j++)
               tmp[3 - j] = MT[i + j];
      
            for (j = 0; j < 4; j++)
               MT[i + j] = tmp[j];
         }
      }
   
      ComputeSHA256(MT, 119, TRUE, TRUE, MAC);
   }
   // one block format
   else
   {
      // insert secret
      memcpy(&MT[0], &SECRET[0], 32);

      // change to little endian for A1 devices
      if (reverse_endian)
      {
         for (i = 0; i < 56; i+=4)
         {
            for (j = 0; j < 4; j++)
               tmp[3 - j] = MT[i + j];
      
            for (j = 0; j < 4; j++)
               MT[i + j] = tmp[j];
         }
      }

      ComputeSHA256(MT, 55, TRUE, TRUE, MAC);
   }

   return TRUE;
}

//----------------------------------------------------------------------
// Computes SHA-256 given the MT digest buffer after first iserting
// the secret at the correct location in the array defined by all Maxim
// devices. Since part of the input is secret it is called a Message
// Autnentication Code (MAC).
//
// 'MT'           - buffer containing the message digest
// 'length'       - Length of block to digest
// 'compare_MAC'  - MAC in byte order used by 1-Wire device to compare
//                 with calculate MAC.
//
int VerifyMAC256(uchar* MT, short length, uchar* compare_MAC)
{
   uchar calc_mac[32];
   int i;

   // calculate the MAC
   ComputeMAC256(MT, length, calc_mac);

   // Compare calculated mac with one read from device
   for (i = 0; i < 32; i++)
   {
      if (compare_MAC[i] != calc_mac[i])
         return FALSE;
   }
   return TRUE;
}

//----------------------------------------------------------------------
// Performs a Compute Next SHA-256 calculation given the provided 32-bytes
// of binding data and 8 byte partial secret. The first 8 bytes of the
// resulting MAC is set as the new secret.
// 
// 'binding'  - 32 byte buffer containing the binding data
// 'partial'  - 8 byte buffer with new partial secret
// 'page_num'  - page number that the compute is calculated on
// 'manid'  - manufacturer ID
//
// Globals used : SECRET used in calculation and set to new secret
//
// Returns: TRUE if compute successful
//          FALSE failed to do compute
//
int CalculateNextSecret256(uchar* binding, uchar* partial, int page_num, uchar* manid, uchar* rom_no)
{
   uchar MT[128];
   uchar MAC[64];

   // clear 
   memset(MT,0,128);

   // insert page data
   memcpy(&MT[0],binding,32);

   // insert challenge
   memcpy(&MT[32],partial,32);

   // insert ROM number or FF
   //Muxlab
   //memcpy(&MT[96],ROM_NO,8);   
   memcpy(&MT[96],rom_no,8);

   MT[106] = page_num;
   MT[105] = manid[0];
   MT[104] = manid[1];

   ComputeMAC256(MT, 119, MAC);

   // set the new secret to the first 32 bytes of MAC
   set_secret(MAC);

   return TRUE;
}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_ch(ulong x, ulong y, ulong z)
{
   return (x & y) ^ ((~x) & z);
}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_maj(ulong x, ulong y, ulong z)
{
   ulong temp = x & y;
   temp ^= (x & z);
   temp ^= (y & z);
   return temp;  //(x & y) ^ (x & z) ^ (y & z);
}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_rotr_32(ulong val, ushort r)
{
   val = val & 0xFFFFFFFFL;
   return ((val >> r) | (val << (32 - r))) & 0xFFFFFFFFL;
}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_shr_32(ulong val, ushort r)
{
   val = val & 0xFFFFFFFFL;
   return val >> r;
}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_bigsigma256_0(ulong x)
{
   return sha_rotr_32(x,2) ^ sha_rotr_32(x,13) ^ sha_rotr_32(x,22);
}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_bigsigma256_1(ulong x)
{
   return sha_rotr_32(x,6) ^ sha_rotr_32(x,11) ^ sha_rotr_32(x,25);
}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_littlesigma256_0(ulong x)
{
   return sha_rotr_32(x,7) ^ sha_rotr_32(x,18) ^ sha_shr_32(x,3);
}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_littlesigma256_1(ulong x)
{
   return sha_rotr_32(x,17) ^ sha_rotr_32(x,19) ^ sha_shr_32(x,10);
}

//----------------------------------------------------------------------
// SHA-256 support function
//
void sha_copy32(ulong* p1, ulong* p2, ushort length)
{
   while (length > 0)
   {
      *p2++ = *p1++;
      length--;
   }
}

//----------------------------------------------------------------------
// SHA-256 support function
//
void sha_copyWordsToBytes32(ulong* input, uchar* output, ushort numwords)
{
    ulong temp;
    ushort i;

    for (i=0;i<numwords;i++)
    {
        temp = *input++;
        *output++ = (uchar)(temp >> 24);
        *output++ = (uchar)(temp >> 16);
        *output++ = (uchar)(temp >> 8);
        *output++ = (uchar)(temp);
    }
}

//----------------------------------------------------------------------
// SHA-256 support function
//
void sha_writeResult(ushort reverse, uchar* outpointer)
{
   int i;
   uchar tmp;

   sha_copyWordsToBytes32(H32, outpointer, 8); 

   if (reverse)
   {
      for (i = 0; i < 16; i++)
      {  
         tmp = outpointer[i];
         outpointer[i] = outpointer[31-i];
         outpointer[31-i] = tmp;
      }
   }

}

//----------------------------------------------------------------------
// SHA-256 support function
//
ulong sha_getW(int index)
{
   ulong newW;
   if (index < 16)
   {
      return W32[index];
   }

   newW = sha_littlesigma256_1(W32[(index-2)&0x0f]) + 
            W32[(index-7)&0x0f] + 
          sha_littlesigma256_0(W32[(index-15)&0x0f]) + 
            W32[(index-16)&0x0f];
   W32[index & 0x0f] = newW & 0xFFFFFFFFL;  // just in case...

   return newW;
}

//----------------------------------------------------------------------
// Prepair the block for hashing
//
void sha_prepareSchedule(uchar* message)
{
   // we need to copy the initial message into the 16 W registers
   ushort i,j;
   ulong temp;
   for (i = 0; i < 16; i++)
   {
      temp = 0;
      for (j = 0; j < 4;j++)
      {
         temp = temp << 8;
         temp = temp | (*message & 0xff);
         message++;
      }

      W32[i] = temp;
   }
}

//----------------------------------------------------------------------
// Hash a single block of data. 
//
void sha256_hashblock(uchar* message, ushort lastblock)
{
   ushort sha1counter = 0;
   ushort sha1functionselect = 0;
   ushort i;
   ulong nodeT1, nodeT2;

   ulong Wt, Kt;

   // chunk the original message into the working schedule
   sha_prepareSchedule(message);

   a32 = H32[0];
   b32 = H32[1];
   c32 = H32[2];
   d32 = H32[3];
   e32 = H32[4];
   f32 = H32[5];
   g32 = H32[6];
   h32 = H32[7];

   // rounds
   for (i = 0; i < 64; i++)
   {
      Wt = sha_getW(i);
      Kt = SHA_CONSTANTS[i]; 

      nodeT1 = (h32 + sha_bigsigma256_1(e32) + sha_ch(e32,f32,g32) + Kt + Wt); // & 0xFFFFFFFFL;
      nodeT2 = (sha_bigsigma256_0(a32) + sha_maj(a32,b32,c32)); // & 0xFFFFFFFFL;
      h32 = g32;
      g32 = f32;
      f32 = e32;
      e32 = d32 + nodeT1;
      d32 = c32;
      c32 = b32;
      b32 = a32;
      a32 = nodeT1 + nodeT2;

      sha1counter++;
      if (sha1counter==20)
      {
         sha1functionselect++;
         sha1counter = 0;
      }			

   }

   if (!lastblock)
   {
      // now fix up our H array
      H32[0] += a32;
      H32[1] += b32;
      H32[2] += c32;
      H32[3] += d32;
      H32[4] += e32;
      H32[5] += f32;
      H32[6] += g32;
      H32[7] += h32;
   }
   else
   {
      // now fix up our H array
      H32[0] = a32;
      H32[1] = b32;
      H32[2] = c32;
      H32[3] = d32;
      H32[4] = e32;
      H32[5] = f32;
      H32[6] = g32;
      H32[7] = h32;
   }
}

//----------------------------------------------------------------------
// Computes SHA-256 given the data block 'message' with no padding. 
// The result is returned in 'digest'.   
//
// 'message'  - buffer containing the message 
// 'skipconst' - skip adding constant on last block (skipconst=1)
// 'reverse' - reverse order of digest (reverse=1, MSWord first, LSByte first)
// 'digest'   - result hash digest in byte order used by 1-Wire device
//
int ComputeSHA256(uchar* message, short length, ushort skipconst, ushort reverse, uchar* digest)
{
   ushort bytes_per_block;
   ushort nonpaddedlength;
   ushort numblocks;
   ushort i,j;
   ulong bitlength;
   ushort markerwritten;
   ushort lastblock;

   ushort wordsize = 32;

   /*
   if (sha_debug)
   {
      dprintf("\nSHA-256 INPUT:\n");
      for (i = 0; i < length; i+=4)
      {
         for (j = 0; j < 4; j++)
            dprintf("%02X ",message[i+j]);
         dprintf("\n");
      }
      dprintf("\n");
   }
   */

   // if wordsize is 32 bits, we need 512 bit blocks.  else 1024 bit blocks.
   // that means 16 words are in one message.
   bytes_per_block = 16 * (wordsize / 8);
   // 1 byte for the '80' that follows the message, 8 or 16 bytes of length
   nonpaddedlength = length + 1 + (wordsize/4);
   numblocks = nonpaddedlength / bytes_per_block;
   if ((nonpaddedlength % bytes_per_block) != 0) 
   {
      // then there is some remainder we need to pad
      numblocks++;
   }

   sha_copy32(SHA_256_Initial, H32, SHA_256_INITIAL_LENGTH); 

   bitlength = 8 * length;
   markerwritten = 0;
   // 'length' is our number of bytes remaining.
   for (i = 0; i < numblocks; i++)
   {
      if (length > bytes_per_block)
      {
         memcpy(workbuffer, message, bytes_per_block);
         length -= bytes_per_block;
      }
      else if (length==bytes_per_block)
      {
         memcpy(workbuffer, message, length);
         length = 0;
      }
      else // length is less than number of bytes in a block
      {
         memcpy(workbuffer, message, length);
         // message is now used for temporary space
         message = workbuffer + length;     
         if (markerwritten == 0)
         {
            *message++ = 0x80;
            length++;
         }

         while (length < bytes_per_block)
         {
            // this loop is inserting padding, in this case all zeroes
            *message++ = 0;
            length++;
         }
         length = 0;
         // signify that we have already written the 80h
         markerwritten = 1;
      }

      // on the last block, put the bit length at the very end
      lastblock = (i == (numblocks - 1));
      if (lastblock)
      {
         // point at the last byte in the block
         message = workbuffer + bytes_per_block - 1;
         for (j = 0; j < wordsize/4; j++)
         {
            *message-- = (uchar)bitlength;
            bitlength = bitlength >> 8;
         }
      }

      // SHA in software 
      sha256_hashblock(workbuffer, (ushort)(lastblock && skipconst));
      message += bytes_per_block;
   }

   sha_writeResult(reverse, digest);

   /*
   if (sha_debug)
   {
      dprintf("\nSHA-256 Result:\n");
      for (i = 0; i < 32; i++)
         dprintf("%02X ",digest[i]);
      dprintf("\n");
   }
   */
   return TRUE;
}

//software calculate MAC
int SHA256_Compute_Software_Mac(unsigned char page_num, unsigned char *page_data, unsigned char *challenge, unsigned char *rom_id, unsigned char *secret, unsigned char *man_id, unsigned char anon, unsigned char *chip_mac)
{
	int i,j;
	uchar calc_mac[32];
	uchar tmp[4]; 
	uchar MT[128];
	// create buffer to compute and verify mac
	// clear 
	memset(MT,0,128);

	// insert page data
	memcpy(&MT[0],page_data,32);

	// insert challenge
	memcpy(&MT[32],challenge,32);
   
	// insert secret
    memcpy(&MT[64], &secret[0], 32);
	// insert ROM number or FF
	if (anon)
		memset(&MT[96],0xFF,8);
	else
		memcpy(&MT[96],rom_id,8);

	MT[106] = page_num;
	MT[105] = man_id[0];
	MT[104] = man_id[1];
	

	// change to little endian for A1 devices    
	for (i = 0; i < 108; i+=4)
	{
		for (j = 0; j < 4; j++)
			tmp[3 - j] = MT[i + j];
     
        for (j = 0; j < 4; j++)
			MT[i + j] = tmp[j];
    }
    
	ComputeSHA256(MT, 119, TRUE, TRUE, calc_mac);
	// Compare calculated mac with one read from device
	for (i = 0; i < 32; i++)
	{
		if (chip_mac[i] != calc_mac[i])
			return FALSE;
	}
	return TRUE; 
}

//compute rom_id crc8
int SHA256_Calc_Rom_Id_Crc(int data_byte, int crc)
{
	int bit_mask = 0, carry_check = 0, temp_data = 0;
	temp_data = data_byte;
	for (bit_mask = 0; bit_mask <= 7; bit_mask++)
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
	return (crc);
}

int SHA256_App_Check(void)
{
	int fd;
	int ret, i;
	int testcrc, data_byte;
	unsigned char challenge[32];
	unsigned char secret[32];
	unsigned char chip_mac[32];
	unsigned char page_data[32];
	unsigned char rom_id[8];
	unsigned char man_id[2];
	unsigned char personality_data[4];
	unsigned char page_num;
	unsigned char anon;
	unsigned char lock_magic;
	struct SHA256_DS_MSG	chip_ds_msg;
	
	//init chip_ds_msg
	for (i = 0; i < 8; i++)
	{
		chip_ds_msg.rom_id[i] = 0;
		chip_ds_msg.secret[i] = 0x5a;
		chip_ds_msg.challenge[i] = 0xa9;
	}		
	for (i = 0; i < 20; i++)
		chip_ds_msg.mac[i] = 0x0;
	for (i = 0; i < 32; i++)
		chip_ds_msg.page_0_data[i] = 0x0;	
	for (i = 0; i < 32; i++)
		chip_ds_msg.page_1_data[i] = 0x0;		
	chip_ds_msg.lock_flag = 0;
	chip_ds_msg.lock_magic = 0;
		
	anon = 0;
	testcrc = 0x00;	
	fd = open("/dev/muxds", O_RDWR);
	if (fd < 0)
	{
		printf("can not open /dev/muxds!\n");
		return -1;
	}	
	if (ioctl(fd, SHA256_READ_ROM_ID, &chip_ds_msg) == -1)
	{
		printf("DS IOCTL READ_ROM read ds error!\n");
		return -2;
	}	
	memcpy(rom_id, &chip_ds_msg.rom_id[0], 8);	
	
	testcrc = 0x00;		
	for (i = 0; i < 7; i++)
	{
		data_byte = rom_id[i];			
		testcrc = SHA256_Calc_Rom_Id_Crc(data_byte, testcrc);
	}
	
	if ((testcrc != rom_id[7]) || (rom_id[0] != 0x17))
	{
		printf("read ROM ID FAIL! Exit!\n");	
		return -3;
	}	
	ret = ioctl(fd, SHA256_READ_CHIP_MEM, &chip_ds_msg);
	if (ret == -1)
	{
		printf("DS IOCTL READ_CHIP_MEM FAIL!\n");
		return -4;
	}		
	srand((unsigned)time(NULL));	//init random seed	
	for (i = 0; i < 32; i++)
	{	
		chip_ds_msg.challenge[i] = rand() % 256;		
	}
	
	//set page_num	
	chip_ds_msg.page_num = 0;
	//set no anonymous mode
	chip_ds_msg.anon = 0;		
	
	//Read MAC from chip, this step does not need to send secret to chip. ONLY 32 byte challenge data is used.
	if (ioctl(fd, SHA256_COMPUTE_READ_MAC, &chip_ds_msg) == -1)
	{
		printf("DS IOCTL COMPUTE_READ_MAC FAIL!\n");
		return -5;
	}
	
	anon = chip_ds_msg.anon;
	
	page_num = chip_ds_msg.page_num;
	if (page_num == 0)
		memcpy(page_data, chip_ds_msg.page_0_data, 32);
	else
		memcpy(page_data, chip_ds_msg.page_1_data, 32);	
	
	memcpy(challenge, chip_ds_msg.challenge, 32);	
	memcpy(man_id, chip_ds_msg.man_id, 2);
	memcpy(chip_mac, chip_ds_msg.mac, 32);
	
	//set key *MARBOHJOCAUBPETJIARODFANSANPAT*	
	secret[0] = (rom_id[0] + 0x2A + 0x2) & 0xFF;
	secret[1] = (rom_id[1] + 0x4D + 0x2) & 0xFF;
	secret[2] = (rom_id[2] + 0x41 + 0x2) & 0xFF;
	secret[3] = (rom_id[3] + 0x52 + 0x2) & 0xFF;
	
	secret[4] = (rom_id[4] + 0x42 + 0x2) & 0xFF;
	secret[5] = (rom_id[5] + 0x4F + 0x2) & 0xFF;
	secret[6] = (rom_id[6] + 0x48 + 0x2) & 0xFF;
	secret[7] = (rom_id[7] + 0x4A + 0x2) & 0xFF;
	
	secret[8]  = (rom_id[0] + 0x4F + 0x4) & 0xFF;
	secret[9]  = (rom_id[1] + 0x43 + 0x4) & 0xFF;
	secret[10] = (rom_id[2] + 0x41 + 0x4) & 0xFF;
	secret[11] = (rom_id[3] + 0x55 + 0x4) & 0xFF;
	
	secret[12] = (rom_id[4] + 0x42 + 0x4) & 0xFF;
	secret[13] = (rom_id[5] + 0x50 + 0x4) & 0xFF;
	secret[14] = (rom_id[6] + 0x45 + 0x4) & 0xFF;
	secret[15] = (rom_id[7] + 0x54 + 0x4) & 0xFF;
	
	secret[16] = (rom_id[0] + 0x4A + 0x6) & 0xFF;	
	secret[17] = (rom_id[1] + 0x49 + 0x6) & 0xFF;
	secret[18] = (rom_id[2] + 0x41 + 0x6) & 0xFF;
	secret[19] = (rom_id[3] + 0x52 + 0x6) & 0xFF;
	
	secret[20] = (rom_id[4] + 0x4F + 0x6) & 0xFF;
	secret[21] = (rom_id[5] + 0x44 + 0x6) & 0xFF;
	secret[22] = (rom_id[6] + 0x46 + 0x6) & 0xFF;
	secret[23] = (rom_id[7] + 0x41 + 0x6) & 0xFF;
	
	secret[24] = (rom_id[0] + 0x4E + 0x8) & 0xFF;	
	secret[25] = (rom_id[1] + 0x53 + 0x8) & 0xFF;
	secret[26] = (rom_id[2] + 0x41 + 0x8) & 0xFF;
	secret[27] = (rom_id[3] + 0x4E + 0x8) & 0xFF;
	
	secret[28] = (rom_id[4] + 0x50 + 0x8) & 0xFF;
	secret[29] = (rom_id[5] + 0x41 + 0x8) & 0xFF;
	secret[30] = (rom_id[6] + 0x54 + 0x8) & 0xFF;
	secret[31] = (rom_id[7] + 0x2A + 0x8) & 0xFF;
	
	//printf("secret key is : \n");
	//for(i = 0; i < 32; i++)
	//	printf("%02x", secret[i]);
	
	if (!SHA256_Compute_Software_Mac(page_num, page_data, challenge, rom_id, secret, man_id, anon, chip_mac))
	{
		printf("MAC compare ERROR!\n");
		close(fd);
		return -6;
	}
	else
		printf("MAC compare OK!\n");	
		
	close(fd);
	return 0;		
}

int SHA256_Read_Rom_Id(unsigned char *rom_id)
{
	int fd, i, testcrc, data_byte;
	unsigned char dev_info[8];	
	struct SHA256_DS_MSG	chip_ds_msg;
	fd = open(SECURITY_DEVICE, O_RDWR);
	if (fd < 0)
	{
		printf("\ncan not open SECURITY_DEVICE!");
		return -1;
	}	
	memset(dev_info, 0, sizeof(dev_info));
	if (ioctl(fd, SHA256_READ_ROM_ID, &chip_ds_msg) == -1)
	{
		printf("DS IOCTL READ_ROM read ds error!\n");
		return -2;
	}	
	for (i = 0; i < 8; i++)
	{
		dev_info[i] = chip_ds_msg.rom_id[i];				
	}	
	testcrc = 0x00;	
	for (i = 0; i < 7; i++)
	{
		data_byte = dev_info[i];			
		testcrc = SHA256_Calc_Rom_Id_Crc(data_byte, testcrc);
	}		
	if ((testcrc != dev_info[7]) || (dev_info[0] != 0x17))
	{		
		return -3;
	}	
	memcpy(rom_id, dev_info, 8);
	return 0;
}

int SHA256_Write_Key(unsigned char *key)		
{
	int fd;
	int ret, i;
	int testcrc, data_byte;
	unsigned char challenge[32];
	unsigned char secret[32];
	unsigned char chip_mac[32];
	unsigned char page_data[32];
	unsigned char rom_id[8];
	unsigned char man_id[2];
	unsigned char personality_data[4];
	unsigned char page_num;
	unsigned char anon;
	struct SHA256_DS_MSG	chip_ds_msg;
	
	unsigned char ds_page_0_data[32] = { 'M', 'U', 'X', 'L', 'A', 'B', ' ', 'H', 'D', 'M', 'I', 'O', 'V', 'E', 'R', 'I', 'P', 'R', 'A', 'W', 'N', 'C', 'M', 'P',
		' ', '5', '0', '0', '7', '6', '8', '\0' };

	unsigned char ds_page_1_data[32] = { 'L', 'E', 'S', 'S', 'D', 'E', 'L', 'A', 'Y', ' ', ' ', 'O', 'V', 'E', 'R', 'I', 'P', 'R', 'A', 'W', 'N', 'C', 'M', 'P',
		' ', '5', '0', '0', '7', '6', '8', '\0' };
	
	//init chip_ds_msg
	memset(&chip_ds_msg.rom_id[0], 0, sizeof(chip_ds_msg));
	
	anon = 0;
	testcrc = 0x00;	
	fd = open("/dev/muxds", O_RDWR);
	if (fd < 0)
	{
		printf("can not open /dev/muxds!\n");
		return -1;
	}
	
	if (ioctl(fd, SHA256_CHIP_TIMING_INIT, &chip_ds_msg) == -1)
	{
		printf("DS IOCTL CHIP_TIMING_INIT error!\n");
		return -2;
	}
		
	if (ioctl(fd, SHA256_READ_ROM_ID, &chip_ds_msg) == -1)
	{
		printf("DS IOCTL READ_ROM read ds error!\n");
		return -3;
	}
	
	memcpy(rom_id, &chip_ds_msg.rom_id[0], 8);	
	
	testcrc = 0x00;		
	for (i = 0; i < 7; i++)
	{
		data_byte = rom_id[i];			
		testcrc = SHA256_Calc_Rom_Id_Crc(data_byte, testcrc);
	}	
	if ((testcrc != rom_id[7]) || (rom_id[0] != 0x17))			
	{
		printf("read ROM ID FAIL! Exit!\n");	
		return -4;
	}	
	
	//for (i = 0; i < 8; i++)		
	//	printf(" 0x%x ", rom_id[i]);
	//printf("\n");
	
	//for (i = 0; i < 32; i++)		
	//	printf(" key[%d] = 0x%x ", i, key[i]);
	//printf("\n");
	
	//copy setcret key
	memcpy(&chip_ds_msg.secret[0], key, 32);
	memcpy(&chip_ds_msg.page_0_data, ds_page_0_data, 32);
	memcpy(&chip_ds_msg.page_1_data, ds_page_1_data, 32);			
	
	chip_ds_msg.page_num = 0;
	chip_ds_msg.anon = 0;
	chip_ds_msg.lock_flag = 0;	//Do not lock secret
	chip_ds_msg.lock_magic = 0;
	
	ret = ioctl(fd, SHA256_INIT_CHIP_PRODUCTION_KEY, &chip_ds_msg);
	if (ret == -1)
	{
		printf("DS IOCTL INIT_CHIP_PRODUCTION FAIL!\n");
		return -5;
	}	

	ret = ioctl(fd, SHA256_READ_CHIP_MEM, &chip_ds_msg);
	if (ret == -1)
	{
		printf("DS IOCTL READ_CHIP_MEM FAIL!\n");
		return -6;
	}		
	srand((unsigned)time(NULL));	//init random seed	
	for (i = 0; i < 32; i++)
	{	
		chip_ds_msg.challenge[i] = rand() % 256;		
	}

	//set page_num	
	chip_ds_msg.page_num = 0;
	printf("Use %d page data for MAC computation.\n", chip_ds_msg.page_num);
	//set no anonymous mode
	chip_ds_msg.anon = 0;		
	
	//Read MAC from chip, this step does not need to send secret to chip. ONLY 32 byte challenge data is used.
	if (ioctl(fd, SHA256_COMPUTE_READ_MAC, &chip_ds_msg) == -1)
	{
		printf("DS IOCTL COMPUTE_READ_MAC FAIL!\n");
		return -7;
	}
	
	anon = chip_ds_msg.anon;
	
	page_num = chip_ds_msg.page_num;
	if (page_num == 0)
		memcpy(page_data, ds_page_0_data, 32);
	else
		memcpy(page_data, ds_page_1_data, 32);
	memcpy(challenge, chip_ds_msg.challenge, 32);
	memcpy(secret, chip_ds_msg.secret, 32);
	memcpy(man_id, chip_ds_msg.man_id, 2);
	memcpy(chip_mac, chip_ds_msg.mac, 32);
	if (!SHA256_Compute_Software_Mac(page_num, page_data, challenge, rom_id, key, man_id, anon, chip_mac))
	{
		printf("MAC compare ERROR!\n");
		return -8;
	}
	else
		printf("MAC compare OK!\n");	

	close(fd);
	return 0;			
}



int MacStrToBin(char *str, unsigned  char *mac)
{
	int i;
	char *s, *e = NULL;
	if ((mac == NULL) || (str == NULL)) 
	{
		return -1;
	}
	s = (char *) str;
	for (i = 0; i < 6; ++i) {
		mac[i] = s ? strtoul(s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
	return 0;
}


int CheckMacAddrStr(char *mac_addr_string)
{
	if (!mac_addr_string)
		return -1;

	char tmp[20];
	memset(tmp, 0x0, sizeof(tmp));
	memcpy(tmp, mac_addr_string, 17);
	//printf("%s  %d\n", tmp, strlen(tmp));

	char *strtmp = strtok(tmp, ":");
	int hexnum = 0;
	while (strtmp != NULL)
	{
		if (strlen(strtmp) != 2)
			return -1;
		if ((strtmp[0] >= '0' && strtmp[0] <= '9') || (strtmp[0] >= 'A' && strtmp[0] <= 'F') || (strtmp[0] >= 'a' && strtmp[0] <= 'f')) {
			if ((strtmp[1] >= '0' && strtmp[1] <= '9') || (strtmp[1] >= 'A' && strtmp[1] <= 'F') || (strtmp[1] >= 'a' && strtmp[1] <= 'f')) {
				hexnum++;
				strtmp = strtok(NULL, ":");
			}
			else {
				return -1;
			}
		}
		else {
			return -1;
		}

	}
	if (hexnum != 6)
		return -1;
	return 0;
}


void SHA256_StrToHex(char *pbDest, char *pbSrc, int nLen)
{
	char h1, h2;
	char s1, s2;
	int i;

	for (i = 0; i < nLen; i++) 
	{
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];
		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		pbDest[i] = (s1 << 4) | s2;
	}
}


void SHA256_HexToStr(char *pbDest, char *pbSrc, int nLen)
{
	unsigned char ddl, ddh;
	int i;

	for (i = 0; i < nLen; i++) {
		ddh = 48 + (pbSrc[i] >> 4);
		ddl = 48 + (pbSrc[i] & 0xF);
		if (ddh > 57)
			ddh = ddh + 7;
		if (ddl > 57)
			ddl = ddl + 7;
		pbDest[i * 2] = ddh;
		pbDest[i * 2 + 1] = ddl;
	}

	pbDest[nLen * 2] = '\0';
}

int SHA256_Write_MacAddr_Once(char *mac_addr_str)
{
	int fd, i, testcrc, data_byte;	
	unsigned char macaddr[6];
	
	struct SHA256_DS_MSG	chip_ds_msg;
	fd = open(SECURITY_DEVICE, O_RDWR);
	if (fd < 0)
	{
		printf("\ncan not open SECURITY_DEVICE!");
		return -1;
	}
	if (CheckMacAddrStr(mac_addr_str) != 0)
	{
		printf("\nSHA256_Write_MacAddr mac str error!!");
		return -2;
	}		
	
	MacStrToBin(mac_addr_str, macaddr);
	printf("SHA256_Write_MacAddr  %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	
	if (ioctl(fd, SHA256_READ_CHIP_MEM, &chip_ds_msg) == -1)
	{
		printf("SHA256_Write_MacAddr DS IOCTL READ_ROM read ds error!\n");
		return -3;
	}								
	
	for (i = 0; i < 6; i++)	
		chip_ds_msg.page_1_data[i] = macaddr[i];
	
	testcrc = 0x00;	
	for (i = 0; i < 31; i++)
	{
		data_byte = chip_ds_msg.page_1_data[i] ;			
		testcrc = SHA256_Calc_Rom_Id_Crc(data_byte, testcrc);
	}		
	
	chip_ds_msg.page_1_data[31]  = testcrc;
	
	if(ioctl(fd, SHA256_INIT_CHIP_PRODUCTION_MAC, &chip_ds_msg) < 0)	
	{
		printf("SHA256_Write_MacAddr DS IOCTL INIT_CHIP_PRODUCTION_MAC FAIL!\n");
		return -4;
	}	
	
	return 0;
}




int SHA256_Write_ModelID_Once(char *modelidstr)
{
	int fd, i, testcrc, data_byte;	
	unsigned char modelid[10];
	
	struct SHA256_DS_MSG	chip_ds_msg;
	fd = open(SECURITY_DEVICE, O_RDWR);
	if (fd < 0)
	{
		printf("\ncan not open SECURITY_DEVICE!");
		return -1;
	}			
	printf("SHA256_Write_ModelID_Once modelidst is : %s\n", modelidstr);
	SHA256_StrToHex((char *)modelid, modelidstr, 10);
	if (ioctl(fd, SHA256_READ_CHIP_MEM, &chip_ds_msg) == -1)
	{
		printf("SHA256_Write_MacAddr DS IOCTL READ_ROM read ds error!\n");
		return -2;
	}								
	for(i = 10; i < 20; i++)
		chip_ds_msg.page_1_data[i + 10] = modelid[i];
	
	testcrc = 0x00;	
	for (i = 0; i < 31; i++)
	{
		data_byte = chip_ds_msg.page_1_data[i];			
		testcrc = SHA256_Calc_Rom_Id_Crc(data_byte, testcrc);
	}		
	
	chip_ds_msg.page_1_data[31]  = testcrc;
	
	if (ioctl(fd, SHA256_INIT_CHIP_PRODUCTION_MAC, &chip_ds_msg) < 0)	
	{
		printf("SHA256_Write_MacAddr DS IOCTL INIT_CHIP_PRODUCTION_MAC FAIL!\n");
		return -3;
	}		
	return 0;
}


int SHA256_Write_Mac_ModelID_Once(char *mac_addr_str, char *modelidstr)
{
	int fd, i, testcrc, data_byte;	
	unsigned char macaddr[6];
	unsigned char modelid[10];
	struct SHA256_DS_MSG	chip_ds_msg;
	fd = open(SECURITY_DEVICE, O_RDWR);
	if (fd < 0)
	{
		printf("\ncan not open SECURITY_DEVICE!");
		return -1;
	}
	if (CheckMacAddrStr(mac_addr_str) != 0)
	{
		printf("\nSHA256_Write_MacAddr mac str error!!");
		return -2;
	}		
	
	MacStrToBin(mac_addr_str, macaddr);
	printf("SHA256_Write_MacModel_Once  %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);	
	printf("SHA256_Write_MacModel_Once modelidst is : %s\n", modelidstr);
	//SHA256_StrToHex((char *)modelid, modelidstr, 10);
	/*
	if (ioctl(fd, SHA256_READ_CHIP_MEM, &chip_ds_msg) == -1)
	{
		printf("SHA256_Write_MacAddr DS IOCTL READ_ROM read ds error!\n");
		return -3;
	}
	*/
	for (i = 0; i < 32; i++)	
		chip_ds_msg.page_1_data[i] = 0;
	
	for (i = 0; i < 6; i++)	
		chip_ds_msg.page_1_data[i] = macaddr[i];
	
	for (i = 0; i < 10; i++)	
		//chip_ds_msg.page_1_data[i + 10] = modelid[i];
		chip_ds_msg.page_1_data[i + 10] = modelidstr[i];
	
	testcrc = 0x00;	
	for (i = 0; i < 31; i++)
	{
		data_byte = chip_ds_msg.page_1_data[i];			
		testcrc = SHA256_Calc_Rom_Id_Crc(data_byte, testcrc);
	}		
	
	chip_ds_msg.page_1_data[31]  = testcrc;
	
	if (ioctl(fd, SHA256_INIT_CHIP_PRODUCTION_MAC, &chip_ds_msg) < 0)	
	{
		printf("SHA256_Write_MacAddr DS IOCTL INIT_CHIP_PRODUCTION_MAC FAIL!\n");
		return -4;
	}		
	return 0;
}

int SHA256_Read_MacAddr(char *mac_addr_str)
{
	int fd, i, testcrc, data_byte;
	unsigned char page_data[32];	
	struct SHA256_DS_MSG	chip_ds_msg;
	fd = open(SECURITY_DEVICE, O_RDWR);
	if (fd < 0)
	{
		printf("\ncan not open SECURITY_DEVICE!");
		return -1;
	}	
	memset(page_data, 0, sizeof(page_data));
	if (ioctl(fd, SHA256_READ_CHIP_MEM, &chip_ds_msg) == -1)
	{
		printf("DS IOCTL READ_ROM read ds error!\n");
		return -2;
	}		
	for (i = 0; i < 32; i++)	
		page_data[i] = chip_ds_msg.page_1_data[i];						
	
	testcrc = 0x00;	
	for (i = 0; i < 31; i++)
	{
		data_byte = page_data[i];			
		testcrc = SHA256_Calc_Rom_Id_Crc(data_byte, testcrc);
	}		
	if (testcrc != page_data[31])
	{		
		printf("\nRead chip macaddr CRC errror!\n");
		return -3;
	}	
	sprintf(mac_addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", page_data[0], page_data[1], page_data[2], page_data[3], page_data[4], page_data[5]);		
	return 0;
}


int SHA256_Read_Mac_ModelID_Once(char *mac_addr_str, char *model_id_str)
{
	int fd, i, testcrc, data_byte;
	unsigned char page_data[32];	
	struct SHA256_DS_MSG	chip_ds_msg;
	fd = open(SECURITY_DEVICE, O_RDWR);
	if (fd < 0)
	{
		printf("\ncan not open SECURITY_DEVICE!");
		return -1;
	}	
	memset(page_data, 0, sizeof(page_data));
	if (ioctl(fd, SHA256_READ_CHIP_MEM, &chip_ds_msg) == -1)
	{
		printf("DS IOCTL READ_ROM read ds error!\n");
		return -2;
	}		
	for (i = 0; i < 32; i++)	
		page_data[i] = chip_ds_msg.page_1_data[i];						
	
	testcrc = 0x00;	
	for (i = 0; i < 31; i++)
	{
		data_byte = page_data[i];			
		testcrc = SHA256_Calc_Rom_Id_Crc(data_byte, testcrc);
	}		
	if (testcrc != page_data[31])
	{		
		printf("\nRead chip macaddr CRC errror!\n");
		return -3;
	}	
	//sprintf(mac_addr_str, "SHA256_Read_MacAddr : %02X:%02X:%02X:%02X:%02X:%02X", page_data[0], page_data[1], page_data[2], page_data[3], page_data[4], page_data[5]);		
	
	
	for (i = 0; i < 10; i++)
		model_id_str[i] = page_data[i + 10];
	
	sprintf(mac_addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", page_data[0], page_data[1], page_data[2], page_data[3], page_data[4], page_data[5]);		
	return 0;
}
