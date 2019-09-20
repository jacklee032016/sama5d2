Programming for boards with 64MB Flash
##########################################
April 17, 2019


Flash mapping
====================

Creating 10 MTD partitions on "f0020000.spi":
::

   0: 0x0000,0000-0x0001,0000 : 64K  (0x0001,0000), "at91bootstrap"
   1: 0x0001,0000-0x0002,0000 : 64K  (0x0001,0000), "bootloader env"
   2: 0x0002,0000-0x0003,0000 : 64K  (0x0001,0000), "device tree"
   3: 0x0003,0000-0x000d,0000 : 640KB(0x000A,0000), "bootloader"
   4: 0x000d,0000-0x0010,0000 : 192KB(0x0003,0000), "reserved-1"
   5: 0x0010,0000-0x0050,0000 : 4MB  (0x0040,0000), "kernel"
   6: 0x0050,0000-0x0350,0000 : 48MB (0x0300,0000), "rootfs"
   7: 0x0350,0000-0x03f0,0000 : 10MB (0x00A0,0000), "data"
   8: 0x03f0,0000-0x0010,0000 : 1MB  (0x0010,0000), "reserved-2"
   

Steps:
===================

#. **Program bootStrap and u-boot**
::

   sam-ba.exe -x scripts/qspiStrap.qml
   sam-ba.exe -x scripts/qspiUboot.qml

#.  Clear environment of u-boot
::

   sam-ba.exe -x scripts/qspiClear.qml


#. **Program DTB in u-boot**
::

   tftpboot 0x21000000 at91-sama5d2_xplained.dtb;   sf probe 1:0; sf erase 0x20000 0x10000; sf write 0x21000000 0x20000 0x10000
  * 
   tftpboot 0x21000000 m774/mux774_v2.dtb;sf probe 1:0; sf erase 0x20000 0x10000; sf write 0x21000000 0x20000 0x10000
   tftpboot 0x21000000 mux768.dtb;sf probe 1:0; sf erase 0x20000 0x10000; sf write 0x21000000 0x20000 0x10000

   
#. **Program u-boot.bin itself **   
   tftpboot 0x21000000 u-boot.bin; sf probe 1:0; sf erase 0x30000 0xa0000; sf write 0x21000000 0x30000 0xa0000

#. **Program kernel in u-boot**
::

   tftpboot 0x21000000 m774/zImage.Mux774; sf probe 1:0; sf erase 0x100000 0x400000; sf write 0x21000000 0x100000 0x400000
   tftpboot 0x21000000 zImage.Mux768; sf probe 1:0; sf erase 0x100000 0x400000; sf write 0x21000000 0x100000 0x400000
   tftpboot 0x21000000 zImage; sf probe 1:0; sf erase 0x100000 0x400000; sf write 0x21000000 0x100000 0x400000

Clear environment:
   
   sf probe 1:0; sf erase 0x10000 0x10000;

#. **Program JFFS2 root File System**
::
   128M*2018 Blocks = 262144, =0x40000
   tftpboot 0x21000000 App.ext4.2019_05_26
   mmc write 0x21000000 8 40000

   256M*2018 Blocks = 262144*2, =0x80000
   tftpboot 0x21000000 App.ext4.2019_05_27
   mmc write 0x21000000 8 80000

# 768, small file system
::

   tftpboot 0x21000000 root.jffs2.2019_06_18
   sf probe 1:0; sf erase 0x500000 0x1B00000;   # 0x05+0x1B = 0x20, 32MB
   sf write 0x21000000 0x500000 0xXXXX:  here XXXX is come from result of tftpboot command;
   
   
# 774, 64MB file system   
::

   tftpboot 0x21000000 root.jffs2.2019_04_10
   tftpboot 0x21000000 m774/root.jffs2.2019_05_29
   sf probe 1:0; sf erase 0x500000 0x3000000; 
   
   sf write 0x21000000 0x500000 0x3000000
   or
   sf write 0x21000000 0x500000 0xXXXX:  here XXXX is come from result of tftpboot command;
   
