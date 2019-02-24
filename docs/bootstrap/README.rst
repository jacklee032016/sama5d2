README for Bootstrap
################################

02.22, 2019
----------------
* Load u-boot from 0x60000,with size of 0xA0000;
* Add debug info in code;


xplained board
========================
Bootstrap is in serial flash, and it boots u-boot from QSPI flash;

boards/sama5d2_xplained/sama5d2_xplained.c
::

   ddramc_reg_config: support 16 bit DDR SDRAM;
