/*
* Control external devices through I2C bus
*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>	
#include <unistd.h>
#include <string.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#define	EP91A6SQ_REG_AUDIO_INFOFRAME_CC			0x29		/* channel conut */
#define	EP91A6SQ_REG_AUDIO_INFOFRAME_SS			0x2A

#define	EP91A6SQ_REG_AUDIO_PATH						0x10


/* read audio sample frequency and audo channels from EP91A6SQ HDMI repeater. Register Map. p.6  */
int	cmnSysI2cTxReadAudioParams(unsigned char *sampleRate, unsigned char *channels, unsigned char *depth)
{
	unsigned char readValue, audioRate, audioSize, audioChannels;
	int ret;

	ret = EP91A6_READ(EP91A6SQ_REG_AUDIO_INFOFRAME_CC, &readValue, sizeof(readValue));
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("EP91A6SQ read failed");
		return EXIT_FAILURE;
	}
	audioChannels = (readValue&0x07);
	
	/* 10.29, 2019, changed as requirement from FPGA*/
#if 0	
	*channels = audioChannels + 1; /*channel count, such as 2~8 */
#else
	*channels = 8; 
#endif

	ret = EP91A6_READ(EP91A6SQ_REG_AUDIO_INFOFRAME_SS, &readValue, sizeof(readValue));
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("EP91A6SQ read failed");
		return EXIT_FAILURE;
	}
	audioSize = (readValue&0x03);
	*depth = (audioSize==1)?16:(audioSize==2)?20:(audioSize==3)?24:INVALIDATE_VALUE_U8;
	
	audioRate = (readValue>>2)&0x07; /* bit2~bit4 */
	*sampleRate = audioRate;

	/* 10.30, 2019. select audio path in HDMI repeater */
	audioChannels = 0x80;
	ret = EP91A6_WRITE(EP91A6SQ_REG_AUDIO_PATH, &audioChannels, sizeof(audioChannels));
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("EP91A6SQ read failed");
		return EXIT_FAILURE;
	}


	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "\t\tEP91A6SQ audio rate:%s; depth:%d(reg: 0x%02X); channels:%d", 
		CMN_FIND_A_RATE(*sampleRate),  *depth,  readValue, *channels);


	return EXIT_SUCCESS;
}

/* an610. p.11 */
#define	CLK_REG_DEVICE_STATUS						0
#define	CLK_REG_OUT_ENABLE_CTRL					3	/* CLK0~7 enable */
#define	CLK_REG_CLK6_CTRL							22

#define	CLK_REG_MULTISYNTH_NB_PARAM_P3_1		34	/* bit15-8 of P3 */
#define	CLK_REG_MULTISYNTH_NB_PARAM_P3_0		35	/* b7-b0 of P3 */

#define	CLK_REG_MULTISYNTH_NB_PARAM_P1_2		36	/* b17-b16 of P1 */
#define	CLK_REG_MULTISYNTH_NB_PARAM_P1_1		37	/* b15-b8 of P1 */
#define	CLK_REG_MULTISYNTH_NB_PARAM_P1_0		38	/* b7-b0 of P1 */

#define	CLK_REG_MULTISYNTH_NB_PARAM_P2_P3		39	/* 19-16(P3): 19-16(P2)*/
#define	CLK_REG_MULTISYNTH_NB_PARAM_P2_1		40	/* b15-b8 of P2 */
#define	CLK_REG_MULTISYNTH_NB_PARAM_P2_0		41	/* b7-b0 of P2 */

#define	CLK_REG_MULTI_SYNTH_6_PARAM				90

#define	CLK_REG_VCXO_PARAM_B0					162		/* 164 is the third byte of VCXO param, so it is little endian for VCXO param */

#define	CLK_REG_PLL_RESET							177


#define	CLK_READ_BYTE(_addr)	\
	{regAddr = (_addr); if(SI5351B_READ(regAddr, &chValue, 1) == EXIT_FAILURE) { \
		EXT_ERRORF("SI5351B read register address 0x%X failed", regAddr); return EXT_FALSE; }/*else{ EXT_INFOF("Read 0x%02x from reg 0x%02x(%d) OK", (chValue), (regAddr), regAddr);}*/ }

#define	CLK_WRITE_BYTE(_addr, _value)	\
	{regAddr = (_addr); chValue = (_value); if(SI5351B_WRITE(regAddr, &chValue, 1) == EXIT_FAILURE) { \
		EXT_ERRORF("SI5351B read register address 0x%X failed", regAddr); return EXT_FALSE; }/*else{ EXT_INFOF("Write 0x%02x(0x%2x) to reg 0x%02x(%d) OK", chValue, (_value), (regAddr), regAddr);};*/ }

#define	CLK_READ_3_BYTES(_addr) \
	{regAddr = (_addr); p = (unsigned char *)&intValue; if(SI5351B_READ(regAddr, p, 3)== EXIT_FAILURE) { \
		EXT_ERRORF("SI5351B read 3 bytes from register address 0x%X failed", regAddr); return EXT_FALSE; } }

#define	CLK_WRITE_3_BYTES(_addr, _value) \
	{regAddr = (_addr); intValue = (_value); p = (unsigned char *)&intValue; if(SI5351B_WRITE(regAddr, p, 3)== EXIT_FAILURE) { \
		EXT_ERRORF("SI5351B write 3 bytes from register address 0x%X failed", regAddr); return EXT_FALSE; } }


#define	CLK_OP_CLK6_DISABLE()	\
				CLK_WRITE_BYTE(CLK_REG_OUT_ENABLE_CTRL, 0x40)


#define	CLK_OP_CLK6_ENABLE()	\
				CLK_WRITE_BYTE(CLK_REG_OUT_ENABLE_CTRL, 0x0)		/* enable all clks */


#define	CLK_OP_CLK6_POWER_DOWN()	\
				CLK_WRITE_BYTE(CLK_REG_CLK6_CTRL, 0x80)


#define	CLK_OP_CLK6_POWER_UP()	\
				CLK_WRITE_BYTE(CLK_REG_CLK6_CTRL, 0x6F)

				

#define	CLK_OP_WRITE_VCXO_PARAM(_intParam)	\
				CLK_WRITE_3_BYTES(CLK_REG_VCXO_PARAM_B0, (_intParam))


#define	CLK_OP_RESET_PLL_B()	\
				CLK_WRITE_BYTE(CLK_REG_PLL_RESET, 0x80)

/**** register map of MutliSynth NB Parameter  
34: msnb_p3(15:08)
35: msnb_p3(07:00)

36: msnb_p1(17:16)
37: msnb_p1(15:08)
38: msnb_p1(07:00)

39: msnb_p3(19:16):7-4; msnb_p2(19:16):3-0; 

40: msnb_p2(15:08)
41: msnb_p2(07:00)
So, p1:18bits; p2:20bits; p3: 20bits; and registers are big endian
***************************************/
int _si5351bWriteMsbnParams(	unsigned long p1, unsigned long  p2, unsigned long  p3)
{
	unsigned char u8value;

	unsigned int regAddr;
	unsigned char chValue;

	u8value = ((p3 & 0xFFFFF) >> 8) & 0xFF;
 	CLK_WRITE_BYTE(CLK_REG_MULTISYNTH_NB_PARAM_P3_1, u8value);

	//gpio_i2c_write(0xC0, 42, u8value);
	u8value = (p3 & 0xFFFFF) & 0xFF;
	CLK_WRITE_BYTE(CLK_REG_MULTISYNTH_NB_PARAM_P3_0, u8value);
	//gpio_i2c_write(0xC0, 43, u8value);
	//set REG 60 R2_DIV to 100(divide by 16)
	//u8value = ((ms0_p1 & 0x3FFFF) >> 16) & 0x3;
	//u8value |= 0x40;
 
	 //gpio_i2c_write(0xC0, 44, u8value);
	 //u8value = ((ms0_p1 & 0x3FFFF) >> 8) & 0xFF;
 
	u8value = (p1 >> 10) & 0xC0;
	CLK_WRITE_BYTE(CLK_REG_MULTISYNTH_NB_PARAM_P1_2, u8value);
	
	u8value = ((p1 & 0xFFFFF) >> 8) & 0xFF;
 	CLK_WRITE_BYTE(CLK_REG_MULTISYNTH_NB_PARAM_P1_1, u8value);
	
	u8value = (p1 & 0xFFFFF) & 0xFF;
 	CLK_WRITE_BYTE(CLK_REG_MULTISYNTH_NB_PARAM_P1_0, u8value);
	
	u8value = (((p3 >> 12) & 0xF0) | ((p2 >> 16) & 0x0F));
	CLK_WRITE_BYTE(CLK_REG_MULTISYNTH_NB_PARAM_P2_P3, u8value);
	
	u8value = ((p2 & 0xFFFFF) >> 8) & 0xFF;
 	CLK_WRITE_BYTE(CLK_REG_MULTISYNTH_NB_PARAM_P2_1, u8value);
	
	u8value = (p2 & 0xFFFFF) & 0xFF;
 	CLK_WRITE_BYTE(CLK_REG_MULTISYNTH_NB_PARAM_P2_0, u8value);
	//u8value = (ms0_p2 & 0xFFFFF) & 0xFF;
 
	 //ret = write_i2c_register(41, fd, &u8value, 1);

	return EXIT_SUCCESS;
}


typedef struct
{
	/* MS NB: MultiSynth B Divider */
	unsigned long		p1;
	unsigned long		p2;
	unsigned long		p3;
	
	unsigned long		msDivider6;
	
	unsigned long		vcoxParam;	/* for CLK6 */
}CLOCK_PARAMS;

#define	FREQ_MHZ							UNIT_M_HZ

#define	XTAL_REF_CLOCK					(25*FREQ_MHZ)

#define	FVCO_MINMUM_FREQ					(600*FREQ_MHZ)
#define	FVCO_MAXIUM_FREQ					(900*FREQ_MHZ)

/* stage 3 */
#define	S3_DIVIDER_RATE_MAX					254 //32	// 254
/* stage 2 */
#define	S2_MS_DIVIDER_MIN						8
#define	S2_MS_DIVIDER_MAX						900



int	_si53512bCaculateParams(unsigned int clkFreq, CLOCK_PARAMS *clkParams)
{
	unsigned long divider_a, divider_b, divider_c, ms0_p1, ms0_p2, ms0_p3;
	unsigned int APR = 240;
	unsigned int DIV_C = 10000;
	unsigned long vcox_param;
	
	unsigned int	msDivider6; 	/* divider of stage 3 for CLK6, eg (a+b/c) in p3 */
	unsigned long 	fvco;
	unsigned long	msnbDivder; 	/* ==(MultiSynthx*Rx) */

	// choose a valid RATIO
	// msDivder6 must be an even number, from 6 to 254. p2.
	// (divider_a*REF_CLOCK) must be between 600 Mhz and 900 Mhz
	// msnbDivder must be between 8 and 900

	
#if 1
	for (msDivider6 = 6; msDivider6 <= S3_DIVIDER_RATE_MAX; msDivider6 += 2) 
	{
		fvco = clkFreq*msDivider6;
		msnbDivder = fvco/XTAL_REF_CLOCK;
		
//		EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "msDivider6:%d;msnbDivder:%ld(%ld); Fvco:%lu MHz(%lu/%u)", 
//			msDivider6, msnbDivder, fvco/XTAL_REF_CLOCK, (fvco/UNIT_M_HZ), fvco, XTAL_REF_CLOCK );

		if ( (( fvco > FVCO_MINMUM_FREQ) && (fvco < FVCO_MAXIUM_FREQ) )
			&& ((msnbDivder >= S2_MS_DIVIDER_MIN) && (msnbDivder <= S2_MS_DIVIDER_MAX) ) ) 
		{
			break;
		}
	}

	if(msDivider6 >= S3_DIVIDER_RATE_MAX ||msnbDivder < S2_MS_DIVIDER_MIN || msnbDivder > S2_MS_DIVIDER_MAX )
	{
		EXT_ERRORF("Suitable MS divider is not found: Fvco:%ld; MsnbDivider: %ld; msDivider: %d", fvco, msnbDivder, msDivider6 );
		return EXIT_FAILURE;
	}
#else
	for (msDivider6 = 6; msDivider6 <= 32; msDivider6 += 2) 
	{
//		if (ratio == 7) 
//			continue;   // invalid ratio

		fvco = clkFreq*msDivider6;
		divider_a = (clkFreq * msDivider6)/XTAL_REF_CLOCK;
		//printf("ratio:%d   a:%d  %u\n", ratio, divider_a, (divider_a*REF_CLOCK) );

		if (((divider_a*XTAL_REF_CLOCK) > 600000000) && ((divider_a*XTAL_REF_CLOCK) < 900000000) && (divider_a >= 15) && (divider_a <= 90) ) 
		{
			break;
		}
	}
#endif
	
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "Fvco:%ldMHz; MsnbDivider:%ld; msDivider:%d", fvco/UNIT_M_HZ, msnbDivder, msDivider6 );

	
	divider_a = fvco/(XTAL_REF_CLOCK);
	divider_b = fvco - (divider_a * XTAL_REF_CLOCK);
	divider_b /= (XTAL_REF_CLOCK/DIV_C);   	
	divider_c = DIV_C; 
	
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "Divider_a:%ld; _b:%ld; _c:%ld", divider_a, divider_b, divider_c);
	vcox_param = APR * ((128* divider_a) + (divider_b / divider_c));
	vcox_param *= 103;
	vcox_param /= 100;
	
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "VCXO Param=%ld(0x%lx)", vcox_param, vcox_param);	
	//1.03 * 120 * ((128* divider_a) + (double)(divider_b / 1000000));
	ms0_p1 = 128*divider_a + (128*divider_b / divider_c) - 512;
	ms0_p2 = 128*divider_b - divider_c*(128*divider_b / divider_c);
	ms0_p3 = divider_c;
 
	clkParams->p1 = ms0_p1;
	clkParams->p2 = ms0_p2;
	clkParams->p3 = ms0_p3;
	clkParams->msDivider6 = msDivider6;
	clkParams->vcoxParam = vcox_param;

	return EXIT_SUCCESS;
}


int cmnSysI2cSi5351VcxoControl(unsigned long clkFreq)
{
	CLOCK_PARAMS clkParams;
	unsigned int	tmp;
	unsigned char u8value;
	
	unsigned int regAddr;
	unsigned char chValue;
	unsigned int intValue;
	unsigned char *p;

#if 0
	{
		int i;
		for(i=0; i< 64; i++)
		{
			CLK_READ_BYTE(i);
		}
	}

#endif
	
	memset(&clkParams, 0, sizeof(CLOCK_PARAMS));
	if(_si53512bCaculateParams(clkFreq, &clkParams) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	CLK_OP_CLK6_DISABLE();
	CLK_OP_CLK6_POWER_DOWN();

	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "Disable CLK6");

	if(_si5351bWriteMsbnParams(clkParams.p1, clkParams.p2, clkParams.p3) == EXIT_FAILURE)
	{
		MUX_ERROR("Write MultiSynth NB parameters failed");
		return EXIT_FAILURE;
	}
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "Configured MS NB Parameters");
	
#if 0	
	
	u8value = ratio; 
	ret = write_i2c_register(90, fd, &u8value, 1);

	u8value = (unsigned char)vcox_param & 0xFF;
	ret = write_i2c_register(162, fd, &u8value, 1);
	u8value = (unsigned char)((vcox_param & 0xFF00) >> 8) & 0xFF;
	ret = write_i2c_register(163, fd, &u8value, 1);
	u8value = (unsigned char)((vcox_param & 0x7F0000) >> 16);
	ret = write_i2c_register(164, fd, &u8value, 1);

	u8value = 0x80;
	ret = write_i2c_register(177, fd, &u8value, 1);
	
	//u8value = ((ms0_p1 & 0x3FFFF)) & 0xFF;
	//gpio_i2c_write(0xC0, 46, u8value);
 
	 //  
	 //u8value = (((ms0_p3 & 0xFFFFF) >> 12) & 0xF0) | (((ms0_p2 & 0xFFFFF) >> 16) & 0xF);
	 //gpio_i2c_write(0xC0, 47, u8value);
 
	  //
	  //u8value = ((ms0_p2 & 0xFFFFF) >> 8) & 0xFF;
	  //gpio_i2c_write(0xC0, 48, u8value);
	  //  
	  //u8value = ((ms0_p2 & 0xFFFFF)) & 0xFF;
	  //gpio_i2c_write(0xC0, 49, u8value);
 
	   //  
	   //Power up clk6
	u8value = 0x6F;
	ret = write_i2c_register(22, fd, &u8value, 1);
	//gpio_i2c_write(0xC0, 16, 0x0F);
	
	//Enable clk6
	u8value = 0x00;
	ret = write_i2c_register(03, fd, &u8value, 1);
#else	
	u8value = (unsigned char)clkParams.msDivider6; 
	CLK_WRITE_BYTE(CLK_REG_MULTI_SYNTH_6_PARAM, u8value);
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "Configured MS Divider6 Parameters");

#if 0
	u8value = (unsigned char)clkParams.vcoxParam & 0xFF;
	CLK_WRITE_BYTE(CLK_REG_VCXO_PARAM_B0, u8value);
	u8value = (unsigned char)((clkParams.vcoxParam & 0xFF00) >> 8) & 0xFF;
	CLK_WRITE_BYTE(CLK_REG_VCXO_PARAM_B0+1, u8value);
	u8value = (unsigned char)((clkParams.vcoxParam & 0x7F0000) >> 16);
	CLK_WRITE_BYTE(CLK_REG_VCXO_PARAM_B0+2, u8value);
	EXT_INFOF("Configured VCXO Parameters");
#else
	tmp = (unsigned int)clkParams.vcoxParam;
	CLK_OP_WRITE_VCXO_PARAM(tmp);
#endif

	CLK_OP_RESET_PLL_B();

	CLK_OP_CLK6_POWER_UP();

	CLK_OP_CLK6_ENABLE();
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "Enable CLK6 again");

#endif
 
	/* read PLL status.If PLL loss of lock, do a PLL reset and return -3	 */
	CLK_READ_BYTE(0x00);
//	if( (chValue & 0xF0) )
	if( (chValue != 0x11) )
	{
		EXT_ERRORF("PLL wrong. Read PLL status : 0x%02x", chValue);
		return -3;
	}
	 
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "CLK6 set as:%ldMHz, PLL status:0x%02x", clkFreq/UNIT_M_HZ, chValue);

	return 0;
}

int cmnSysI2cSi5351VcxoPllCheck(void)
{
	unsigned int regAddr;
	unsigned char chValue;
	unsigned int intValue;
	unsigned char *p;
	
#define	TEST_VCXO_PARAM			0x123456
	
	CLK_READ_BYTE(CLK_REG_DEVICE_STATUS);
	
	if(chValue & 0x60) /* PLL A or B loss of Lock status */
	{
		MUX_ERROR("SI5351B PLL loss of lock, PLL status : 0x%02x\n", chValue);
		return EXT_TRUE;
	}

	CLK_READ_3_BYTES(CLK_REG_VCXO_PARAM_B0);
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "\t\tSi5351B VCXO, default:0x%08X", intValue);

	CLK_WRITE_3_BYTES(CLK_REG_VCXO_PARAM_B0, TEST_VCXO_PARAM);
	CLK_READ_3_BYTES(CLK_REG_VCXO_PARAM_B0);
	EXT_DEBUGF(EXT_CMN_SYS_HW_DEBUG, "\t\tSi5351B VCXO:0x%08X: %s", intValue, (intValue==TEST_VCXO_PARAM)?"OK":"Failed");
	
	return EXT_FALSE;
}


