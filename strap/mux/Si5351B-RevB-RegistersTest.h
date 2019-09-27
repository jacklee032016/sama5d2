/*
 * Si5351B Rev B Configuration Register Export Header File
 *
 * This file represents a series of Silicon Labs Si5351B Rev B 
 * register writes that can be performed to load a single configuration 
 * on a device. It was created by a Silicon Labs ClockBuilder Pro
 * export tool.
 *
 * Part:		                                       Si5351B Rev B
 * Design ID:                                          
 * Includes Pre/Post Download Control Register Writes: Yes
 * Created By:                                         ClockBuilder Pro v2.34.1 [2019-05-31]
 * Timestamp:                                          2019-09-27 14:56:35 GMT-04:00
 *
 * A complete design report corresponding to this export is included at the end 
 * of this header file.
 *
 */

#ifndef SI5351B_REVB_REG_CONFIG_HEADER
#define SI5351B_REVB_REG_CONFIG_HEADER

#define SI5351B_REVB_REG_CONFIG_NUM_REGS				73

typedef struct
{
	unsigned int address; /* 16-bit register address */
	unsigned char value; /* 8-bit register data */

} si5351b_revb_register_t;

si5351b_revb_register_t const si5351b_revb_registers_m774[SI5351B_REVB_REG_CONFIG_NUM_REGS] =
{
	{ 0x0002, 0x13 },
	{ 0x0003, 0x00 },
	{ 0x0007, 0x01 },
	{ 0x000F, 0x00 },
	{ 0x0010, 0x8C },
	{ 0x0011, 0x8C },
	{ 0x0012, 0x0F },
	{ 0x0013, 0x0F },
	{ 0x0014, 0x0F },
	{ 0x0015, 0x0F },
	{ 0x0016, 0x2F },
	{ 0x0017, 0x0F },
	{ 0x001A, 0x00 },
	{ 0x001B, 0x01 },
	{ 0x001C, 0x00 },
	{ 0x001D, 0x10 },
	{ 0x001E, 0x00 },
	{ 0x001F, 0x00 },
	{ 0x0020, 0x00 },
	{ 0x0021, 0x00 },
	{ 0x0022, 0x42 },
	{ 0x0023, 0x40 },
	{ 0x0024, 0x00 },
	{ 0x0025, 0x0F },
	{ 0x0026, 0xD1 },
	{ 0x0027, 0xFE },
	{ 0x0028, 0x09 },
	{ 0x0029, 0xC0 },
	{ 0x003A, 0x00 },
	{ 0x003B, 0x03 },
	{ 0x003C, 0x00 },
	{ 0x003D, 0x0E },
	{ 0x003E, 0xAA },
	{ 0x003F, 0x00 },
	{ 0x0040, 0x00 },
	{ 0x0041, 0x02 },
	{ 0x0042, 0x00 },
	{ 0x0043, 0x02 },
	{ 0x0044, 0x00 },
	{ 0x0045, 0x10 },
	{ 0x0046, 0xC0 },
	{ 0x0047, 0x00 },
	{ 0x0048, 0x00 },
	{ 0x0049, 0x00 },
	{ 0x004A, 0x00 },
	{ 0x004B, 0x01 },
	{ 0x004C, 0x00 },
	{ 0x004D, 0x10 },
	{ 0x004E, 0x00 },
	{ 0x004F, 0x00 },
	{ 0x0050, 0x00 },
	{ 0x0051, 0x00 },
	{ 0x0052, 0x00 },
	{ 0x0053, 0x80 },
	{ 0x0054, 0x00 },
	{ 0x0055, 0x22 },
	{ 0x0056, 0x9F },
	{ 0x0057, 0x00 },
	{ 0x0058, 0x00 },
	{ 0x0059, 0x00 },
	{ 0x005A, 0x06 },
	{ 0x005B, 0x12 },
	{ 0x0095, 0x00 },
	{ 0x0096, 0x00 },
	{ 0x0097, 0x00 },
	{ 0x0098, 0x00 },
	{ 0x0099, 0x00 },
	{ 0x009A, 0x00 },
	{ 0x009B, 0x00 },
	{ 0x00A2, 0xD3 },
	{ 0x00A3, 0x1C },
	{ 0x00A4, 0x02 },
	{ 0x00B7, 0x12 },

};

/*
 * Design Report
 *
 * Overview
 * ========
 * Part:               Si5351B
 * Project File:       Z:\works\Si5351B-RevB-Project.09.27.slabtimeproj
 * Created By:         ClockBuilder Pro v2.34.1 [2019-05-31]
 * Timestamp:          2019-09-27 14:56:35 GMT-04:00
 * 
 * Design Rule Check
 * =================
 * Errors:
 * - No errors
 * 
 * Warnings:
 * - OUT2 [27 MHz] and OUT3 [24 MHz] may have coupling [1]
 * - OUT4 [25 MHz] and OUT5 [12.288 MHz] may have coupling [1]
 * - OUT6 [148.5 MHz] and OUT7 [50 MHz] may have coupling [1]
 * 
 * Footnotes:
 * [1] To avoid coupling in outputs, Silicon Labs recommends the following:
 * 
 * - Avoid adjacent frequency values that are close. CBPro uses an output's integration bandwidth (IBW) to determine whether two adjacent frequencies are too close. An IBW of 20 MHz is used for frequencies 80 MHz and larger. Lower frequencies will use IBW of OUT/4. CBPro will flag fundamental coupling and coupling up to the fourth harmonic, where coupling frequency = Absolute(OUTa*x - OUTb*y) for all combinations of x and y 1 through 4. If any one of these values is less than or equal to the IBW, the output is flagged as having possible coupling.
 * - Adjacent frequency values that are integer multiples of one another are okay and these outputs should be grouped accordingly.
 * - Unused outputs can be used to separate clock outputs that might otherwise interfere with one another.
 * 
 * Silicon Labs recommends you validate your design's jitter performance using an Evaluation Board.
 * 
 * Design
 * ======
 * Inputs:
 *     IN0: 25 MHz
 * 
 * Outputs:
 *    OUT0: Unused
 *    OUT1: Unused
 *    OUT2: 27 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 *    OUT3: 24 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 *    OUT4: 25 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 *    OUT5: 12.288 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 *    OUT6: 148.5 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 *    OUT7: 50 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 * 
 * Frequency Plan
 * ==============
 * PLL_A:
 *    Enabled Features = None
 *    Fvco             = 900 MHz
 *    M                = 36
 *    Input0:
 *       Source           = Crystal
 *       Source Frequency = 25 MHz
 *       Fpfd             = 25 MHz
 *       Load Capacitance = Not_Applicable
 *    Output2:
 *       Features       = SpreadSpectrum
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 27 MHz
 *       N              = 33.3333333333333333... [ 33 + 1/3 ]
 *    Output3:
 *       Features       = SpreadSpectrum
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 24 MHz
 *       N              = 37.5
 *    Output4:
 *       Features       = SpreadSpectrum
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 25 MHz
 *       N              = 36
 *    Output5:
 *       Features       = SpreadSpectrum
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 12.288 MHz
 *       N              = 73.2421875
 *    Output7:
 *       Features       = None
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 50 MHz
 *       N              = 18
 * PLL_B:
 *    Enabled Features = VCXO
 *    Fvco             = 891 MHz
 *    M                = 35.64
 *    Input0:
 *       Source           = Crystal
 *       Source Frequency = 25 MHz
 *       Fpfd             = 25 MHz
 *       Load Capacitance = Not_Applicable
 *    Output6:
 *       Features       = VCXO
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 148.5 MHz
 *       N              = 6
 * 
 * Settings
 * ========
 * 
 * Location      Setting Name   Decimal Value      Hex Value        
 * ------------  -------------  -----------------  -----------------
 * 0x0002[3]     XO_LOS_MASK    0                  0x0              
 * 0x0002[4]     CLK_LOS_MASK   1                  0x1              
 * 0x0002[5]     LOL_A_MASK     0                  0x0              
 * 0x0002[6]     LOL_B_MASK     0                  0x0              
 * 0x0002[7]     SYS_INIT_MASK  0                  0x0              
 * 0x0003[7:0]   CLK_OEB        0                  0x00             
 * 0x0007[7:4]   I2C_ADDR_CTRL  0                  0x0              
 * 0x000F[2]     PLLA_SRC       0                  0x0              
 * 0x000F[3]     PLLB_SRC       0                  0x0              
 * 0x000F[4]     PLLA_INSELB    0                  0x0              
 * 0x000F[5]     PLLB_INSELB    0                  0x0              
 * 0x000F[7:6]   CLKIN_DIV      0                  0x0              
 * 0x0010[1:0]   CLK0_IDRV      0                  0x0              
 * 0x0010[3:2]   CLK0_SRC       3                  0x3              
 * 0x0010[4]     CLK0_INV       0                  0x0              
 * 0x0010[5]     MS0_SRC        0                  0x0              
 * 0x0010[6]     MS0_INT        0                  0x0              
 * 0x0010[7]     CLK0_PDN       1                  0x1              
 * 0x0011[1:0]   CLK1_IDRV      0                  0x0              
 * 0x0011[3:2]   CLK1_SRC       3                  0x3              
 * 0x0011[4]     CLK1_INV       0                  0x0              
 * 0x0011[5]     MS1_SRC        0                  0x0              
 * 0x0011[6]     MS1_INT        0                  0x0              
 * 0x0011[7]     CLK1_PDN       1                  0x1              
 * 0x0012[1:0]   CLK2_IDRV      3                  0x3              
 * 0x0012[3:2]   CLK2_SRC       3                  0x3              
 * 0x0012[4]     CLK2_INV       0                  0x0              
 * 0x0012[5]     MS2_SRC        0                  0x0              
 * 0x0012[6]     MS2_INT        0                  0x0              
 * 0x0012[7]     CLK2_PDN       0                  0x0              
 * 0x0013[1:0]   CLK3_IDRV      3                  0x3              
 * 0x0013[3:2]   CLK3_SRC       3                  0x3              
 * 0x0013[4]     CLK3_INV       0                  0x0              
 * 0x0013[5]     MS3_SRC        0                  0x0              
 * 0x0013[6]     MS3_INT        0                  0x0              
 * 0x0013[7]     CLK3_PDN       0                  0x0              
 * 0x0014[1:0]   CLK4_IDRV      3                  0x3              
 * 0x0014[3:2]   CLK4_SRC       3                  0x3              
 * 0x0014[4]     CLK4_INV       0                  0x0              
 * 0x0014[5]     MS4_SRC        0                  0x0              
 * 0x0014[6]     MS4_INT        0                  0x0              
 * 0x0014[7]     CLK4_PDN       0                  0x0              
 * 0x0015[1:0]   CLK5_IDRV      3                  0x3              
 * 0x0015[3:2]   CLK5_SRC       3                  0x3              
 * 0x0015[4]     CLK5_INV       0                  0x0              
 * 0x0015[5]     MS5_SRC        0                  0x0              
 * 0x0015[6]     MS5_INT        0                  0x0              
 * 0x0015[7]     CLK5_PDN       0                  0x0              
 * 0x0016[1:0]   CLK6_IDRV      3                  0x3              
 * 0x0016[3:2]   CLK6_SRC       3                  0x3              
 * 0x0016[4]     CLK6_INV       0                  0x0              
 * 0x0016[5]     MS6_SRC        1                  0x1              
 * 0x0016[6]     FBA_INT        0                  0x0              
 * 0x0016[7]     CLK6_PDN       0                  0x0              
 * 0x0017[1:0]   CLK7_IDRV      3                  0x3              
 * 0x0017[3:2]   CLK7_SRC       3                  0x3              
 * 0x0017[4]     CLK7_INV       0                  0x0              
 * 0x0017[5]     MS7_SRC        0                  0x0              
 * 0x0017[6]     FBB_INT        0                  0x0              
 * 0x0017[7]     CLK7_PDN       0                  0x0              
 * 0x001C[17:0]  MSNA_P1        4096               0x01000          
 * 0x001F[19:0]  MSNA_P2        0                  0x00000          
 * 0x001F[23:4]  MSNA_P3        1                  0x00001          
 * 0x0024[17:0]  MSNB_P1        4049               0x00FD1          
 * 0x0027[19:0]  MSNB_P2        920000             0xE09C0          
 * 0x0027[23:4]  MSNB_P3        1000000            0xF4240          
 * 0x003C[17:0]  MS2_P1         3754               0x00EAA          
 * 0x003F[19:0]  MS2_P2         2                  0x00002          
 * 0x003F[23:4]  MS2_P3         3                  0x00003          
 * 0x0044[17:0]  MS3_P1         4288               0x010C0          
 * 0x0047[19:0]  MS3_P2         0                  0x00000          
 * 0x0047[23:4]  MS3_P3         2                  0x00002          
 * 0x004C[17:0]  MS4_P1         4096               0x01000          
 * 0x004F[19:0]  MS4_P2         0                  0x00000          
 * 0x004F[23:4]  MS4_P3         1                  0x00001          
 * 0x0054[17:0]  MS5_P1         8863               0x0229F          
 * 0x0057[19:0]  MS5_P2         0                  0x00000          
 * 0x0057[23:4]  MS5_P3         128                0x00080          
 * 0x005A[7:0]   MS6_P1         6                  0x06             
 * 0x005B[7:0]   MS7_P1         18                 0x12             
 * 0x0095[14:0]  SSDN_P2        0                  0x0000           
 * 0x0095[7]     SSC_EN         0                  0x0              
 * 0x0097[14:0]  SSDN_P3        0                  0x0000           
 * 0x0097[7]     SSC_MODE       0                  0x0              
 * 0x0099[11:0]  SSDN_P1        0                  0x000            
 * 0x009A[15:4]  SSUDP          0                  0x000            
 * 0x00A2[21:0]  VCXO_PARAM     138451             0x021CD3         
 * 0x00B7[7:6]   XTAL_CL        0                  0x0
 * 
 *
 */

#endif