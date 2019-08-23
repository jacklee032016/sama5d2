Usage for new board
################################
08.21, 2019

With Programmer
====================================
Mux774-bootStrap-3.8.10.bin (Mon Aug 19 15:43:56 EDT 2019)
sam-ba.exe -x scripts/qspiStrap.qml

Mux774.u-boot.bin: Jun 18 2019 - 16:03:43 -0400
sam-ba.exe -x scripts/qspiUboot.qml

sam-ba.exe -x scripts/qspiClear.qml


With TFTP server
====================================

dtb: 
tftpboot 0x21000000 m774/mux774_v2.dtb;sf probe 1:0; sf erase 0x20000 0x10000; sf write 0x21000000 0x20000 0x10000

kernel
tftpboot 0x21000000 m774/zImage.Mux774; sf probe 1:0; sf erase 0x100000 0x400000; sf write 0x21000000 0x100000 0x400000

File System
tftpboot 0x21000000 m774/root.jffs2.2019_05_29;
sf probe 1:0; sf erase 0x500000 0x3000000

sf write 0x21000000 0x500000 0xXXXX

