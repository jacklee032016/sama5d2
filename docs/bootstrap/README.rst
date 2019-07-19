README for Bootstrap
################################

06.19 2019
----------------
* Configuration ``ClockGenerator pro``:
   * Input mode: 25M, internal load: 6 pF;
   * Spread Spectrum Clock disabled;
   * Only one clock can be VCXO, eg. 148.5M;
      * APR: 30ppm;
   * Featrure: None,only one is VCXO;
   
   

05.18, 2019
----------------
* Support multiple boards: xplained, Mux774;
* Binary name is related with board name and from Makefile;
   * Makefile include macros from ../Rules.mak;
   * ``-D__EXT_RELEASE__ $(MUX_FLAGS)`` added in Makefile;   
* include from mux7xx.h and mux7xxCompact.h;


03.14, 2019
----------------
* remove directory of 'config/at91bootstrap-config': this is for 'make menuconfig', a lot of header files with size of 0;


Memory map
----------------
* Memory allocated at 0x2000,0000, maximum size if 0x2000,0000, eg.512M Byte;
* bootstrap runs at 0x0000,0000, like ROM program; this is the internal memory area;



02.22, 2019
----------------
* Load u-boot from 0x60000,with size of 0xA0000: .config
* Add debug info in code;


xplained board
========================
Bootstrap is in serial flash, and it boots u-boot from QSPI flash;

boards/sama5d2_xplained/sama5d2_xplained.c
::

   ddramc_reg_config: support 16 bit DDR SDRAM;
