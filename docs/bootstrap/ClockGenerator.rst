README for Bootstrap
################################


Command to access clock generator
=========================================================

# set I2C multiplxer
i2cset -f -y 0 0x70 0 0x04

# disable clock6: power down
i2cset -f -y 0 0x60 0x16 0x8C

# enable clock6: power up
i2cset -f -y 0 0x60 0x16 0x6f

# dump clock generator
i2cdump -f -y 0 0x60



Configuration of Clock Generator
=========================================================
Output pins:
----------------------------------------------------------

* CLK7: 50M, FPGA(11);
* CLK6: 148.5M, FPGA;
* CLK5: 12.288M, I2S0_MCLK; HDMI and audio ADC/DAC(NAU 8822);
* CLK4: 25M, Ref; Ethernet;
* CLK3: 24M, HDMI, HDMI repeator;
* CLK2: 27M, Ref Clk; FPGA;
* CLK1: 24M, TPxx
* CLK0: 28.6M, TPxx;


Dynamic configration of CLK6:
----------------------------------------------------------
Steps:
#. CLK6 disabled and power down;
#. Write MultiSynth NB parameters: p1, p2, and p3;
#. Write MultiSynth Parameter of CLK6: (90);
#. Write VCXO parameter in register 160;
#. Reset PLL B;
#. CLK7 power on and enabled;

Notes:
* PLL_B is controller by VCXO; PLL_A is controlled by SSC(Spread Spectrum Clock);

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


Configuration of Clock Utility
=========================================================
09.27 2019
----------------------------------------------------------
* Configuration ``ClockGenerator pro``:
   * Input mode: 25M, internal load: 0 pF, so register of 0xB7(183) should be 0x12(when 6Pf, it is 0x52);
   * Spread Spectrum Clock disabled;
   * Only one clock can be VCXO, eg. 148.5M;
      * APR: 30ppm;
   * Featrure: None,only one is VCXO;
   * Disable CLK0 and CLK1;


06.19 2019
----------------------------------------------------------
* Configuration ``ClockGenerator pro``:
   * Input mode: 25M, internal load: 6 pF;
   * Spread Spectrum Clock disabled;
   * Only one clock can be VCXO, eg. 148.5M;
      * APR: 30ppm;
   * Featrure: None,only one is VCXO;



root@mux744:~# i2cset -f -y 0 0x70 0 0x04
root@mux744:~# i2cdump -f -y 0 0x60
No size specified (using byte-data access)
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
00: 11 fc 13 00 00 00 d8 01 00 00 00 00 00 00 90 00    ???...??......?.
10: 8c 8c 0f 0f 0f 0f 8c 0f 00 00 00 01 00 10 00 00    ????????...?.?..
20: 00 00 42 40 00 0f d1 fe 09 c0 00 00 00 00 00 00    ..B@.?????......
30: 00 00 00 00 00 00 00 00 00 00 00 03 00 0e aa 00    ...........?.??.
40: 00 02 00 02 00 10 c0 00 00 00 00 01 00 10 00 00    .?.?.??....?.?..
50: 00 00 00 80 00 22 9f 00 00 00 00 12 00 00 00 00    ...?."?....?....
60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
90: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
a0: 00 00 c1 0a 07 00 00 00 00 00 00 00 00 00 00 00    ..???...........
b0: ff 0c 07 02 00 30 14 12 60 60 b8 00 00 23 07 00    .???.0??``?..#?.
c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 40 27 00    .............@'.
d0: 00 00 00 80 00 00 00 00 00 00 1c 00 00 00 00 40    ...?......?....@
e0: 00 00 3c 70 00 00 00 00 00 00 00 00 00 00 00 00    ..<p............
f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
   