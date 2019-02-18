About sama5d2-xplained board
###############################

Write testimage from Website
===================================


Write images to serial flash and bootup

* JP9 Close, so BOOTROM mode;
* Open JP9 again, and try following commands:

::

   sam-ba -p jlink -b sama5d2-xplained -a serialflash -c erase::0x3000 -c writeboot:at91bootstrap-sama5d2_xplained.bin <12K

   sam-ba -p jlink -b sama5d2-xplained -a serialflash -c erase:0x8000:0x70000 -c write:u-boot-sama5d2-xplained.bin:0x8000


   #bootstrap 32K:0x8000
   sam-ba -p jlink -b sama5d2-xplained -a serialflash -c erase::0x8000 -c writeboot:767samaBootstrap.bin

   # from: 128K(0x20000); size:512K (0x80000)
   # sam-ba -p jlink -b sama5d2-xplained -a serialflash -c erase:0x20000:0x80000 -c write:u-boot-sama5d2-xplained.bin:0x20000
   # my boostrap load u-boot from QSPI flash
   sam-ba -p jlink -b sama5d2-xplained -a qspiflash -c erase:0x20000:0x80000 -c write:u-boot-sama5d2-xplained.bin:0x20000
  
  

Hardware
===============

serial flash
---------------

Detected memory size is 4194304 bytes: 4MB
Page size is 256 bytes.

QSPI Flash
------------------

Detected memory size is 33554432 bytes: 32MB
Page size is 256 bytes.


Debug Console
-----------------
#. **EDBG-USB**
  * USB power interface, J1;
  * external chip to support it: JP1 open, JP2 closed;
  
#. **DEBUG**
  * USB-serial cable*
  * Debug interface from sama5d2;
  * JP1 close; JP2 open;
  