Debug NOR flash
################################
09.15, 2019, Jack Lee


Fix the problem of reboot from OS
===============================================
in drivers/mtd/spi-nor/spi-bor.c:

::

	{ "n25q512ax3Sfdp",  INFO(0x20ba20, 0, 64 * 1024, 1024, /*SECT_4K |*/ USE_FSR | SPI_NOR_QUAD_READ|SPI_NOR_SKIP_SFDP| SPI_NOR_4B_OPCODES) },	/* specs. p.43 */

    SPI_NOR_SKIP_SFDP: without query Serial Flash Discovery Parameter
	SPI_NOR_4B_OPCODES: use 4-byte address mode; this option fixed the problem;
	

So, in 3-byte address mode, flash is the set the status, which is not recognised by ``RomBOOT``, and reset command or something similar don't set the flash 
into default status which is recognised by RomBOOT and read code from flash;

For RomBOOT, it only send first 3 bytes of address to read the code which is always save in the first 128Mb, never write any extended address register, 
so no code is read back.

But if in 4-byte address mode, after reset, flash can response the read operation with only 3 bytes of addresses;




Clock and Data Capturing
===============================================
* CPOL: Clock POLarity
* CPHA: Clock PHAse

CPOL, CPHA is the characters of clock signal sent by master:
* CPOL:
    * 0: No clock, signal is 0; clock is on, signal is 1;
	* 1: No clock, signal is 1; clock is on, signal is 0;

* CPHA:
    * 0: rise edee is at the beginning of clock signal;
    * 1: rise edge is at the end of clock signal;
	
2 combination of CPOL and CPHA:

(0, 0) or (1, 1)



QSPI and SPI
===============================================
Enter or exit QUAD operation(QSPI) mode:
* Enter QUAD operation command: 35H;
* Exit QUAD operation command: F5H

3-byte address or 4-byte address mode
===============================================
When size is more than 128Mb, the 4th byte address must be used.

3-byte address mode
-----------------------------------------------
3 bytes of address + one Extended Address Register:

So the extended address register must be writted after the first 3 bytes addresses;


4-byte address mode
-----------------------------------------------
With command to enter or exit 4-byte address mode:
* Enter command: B7H
* Exit command: E9H


