README for Bootstrap
################################


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
