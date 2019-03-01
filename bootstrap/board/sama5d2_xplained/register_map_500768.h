//Register map for use with AN428 (JumpStart)
//http://www.silabs.com/clocks 
//Copyright 2014 Silicon Laboratories
//**************************************************
//C-Code File version = 1
//#BEGIN_HEADER
//Date = Wednesday, July 04, 2018 12:44 PM
//File version = 3
//Software Name = Si5351 ClockBuilder Desktop
//Software version = 6.5
//Software date = June 4, 2015
//Chip = Si5351B
//Part Number = Si5351B-x-GM
//#END_HEADER
//I2C address = 0x60
/*
#XTAL (MHz) = 25.000000000
#Mode = Automatic
#PLL A
# Input Frequency (MHz) = 25.000000000
# F divider = 1
# PFD (MHz) = 25.000000000
# VCO Frequency (MHz) =  811.008000000
# Feedback Divider = 32  1376/3125
# Internal Load Cap (pf) = 10
# SSC disabled
#PLL B
# Input Frequency (MHz) = 25.000000000
# F divider = 1
# PFD (MHz) = 25.000000000
# VCO Frequency (MHz) =  891.000000000
# Feedback Divider = 35  16/25
# Pull Range (±ppm) = 240.0
#Output Clocks
#Channel 0
# Output Frequency (MHz) = 28.636360000
# Multisynth Output Frequency (MHz) = 28.636360000
# Multisynth Divider = 28  229748/715909
# R Divider = 1
# PLL source = PLLA
# Initial phase offset (ns) = 0.000
# Error (ppm) = 0.0000
# Powered = On
# Inverted = No
# Drive Strength = b11
# Disable State = Low
# Clock Source = b11
#Channel 1
# Output Frequency (MHz) = 24.000000000
# Multisynth Output Frequency (MHz) = 24.000000000
# Multisynth Divider = 33  99/125
# R Divider = 1
# PLL source = PLLA
# Initial phase offset (ns) = 0.000
# Error (ppm) = 0.0000
# Powered = On
# Inverted = No
# Drive Strength = b11
# Disable State = Low
# Clock Source = b11
#Channel 2
# Output Frequency (MHz) = 27.000000000
# Multisynth Output Frequency (MHz) = 27.000000000
# Multisynth Divider = 30  14/375
# R Divider = 1
# PLL source = PLLA
# Initial phase offset (ns) = 0.000
# Error (ppm) = 0.0000
# Powered = On
# Inverted = No
# Drive Strength = b11
# Disable State = Low
# Clock Source = b11
#Channel 3
# Output Frequency (MHz) = 24.000000000
# Multisynth Output Frequency (MHz) = 24.000000000
# Multisynth Divider = 33  99/125
# R Divider = 1
# PLL source = PLLA
# Initial phase offset (ns) = 0.000
# Error (ppm) = 0.0000
# Powered = On
# Inverted = No
# Drive Strength = b11
# Disable State = Low
# Clock Source = b11
#Channel 4
# Output Frequency (MHz) = 25.000000000
# Multisynth Output Frequency (MHz) = 25.000000000
# Multisynth Divider = 32  1376/3125
# R Divider = 1
# PLL source = PLLA
# Initial phase offset (ns) = 0.000
# Error (ppm) = 0.0000
# Powered = On
# Inverted = No
# Drive Strength = b11
# Disable State = Low
# Clock Source = b11
#Channel 5
# Output Frequency (MHz) = 12.288000000
# Multisynth Output Frequency (MHz) = 12.288000000
# Multisynth Divider = 66
# R Divider = 1
# PLL source = PLLA
# Initial phase offset (ns) = 0.000
# Error (ppm) = 0.0000
# Powered = On
# Inverted = No
# Drive Strength = b11
# Disable State = Low
# Clock Source = b11
#Channel 6
# Output Frequency (MHz) = 148.500000000
# Multisynth Output Frequency (MHz) = 148.500000000
# Multisynth Divider = 6
# R Divider = 1
# PLL source = PLLB
# Error (ppm) = 0.0000
# Powered = On
# Inverted = No
# Drive Strength = b11
# Disable State = Low
# Clock Source = b11
#Channel 7
# Output Frequency (MHz) = 12.288000000
# Multisynth Output Frequency (MHz) = 12.288000000
# Multisynth Divider = 66
# R Divider = 1
# PLL source = PLLA
# Error (ppm) = 0.0000
# Powered = On
# Inverted = No
# Drive Strength = b11
# Disable State = Low
# Clock Source = b11
#
*/

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
//End of file
