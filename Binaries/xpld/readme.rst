
dtb:
tftpboot 0x21000000 xpld/at91-sama5d2_xplained.dtb;sf probe 1:0; sf erase 0x20000 0x10000; sf write 0x21000000 0x20000 0x10000
tftpboot 0x21000000 xpld/xplained.dtb;sf probe 1:0; sf erase 0x20000 0x10000; sf write 0x21000000 0x20000 0x10000

kernel:
tftpboot 0x21000000 xpld/zImage.Sama5d2Xpld; sf probe 1:0; sf erase 0x100000 0x400000; sf write 0x21000000 0x100000 0x400000
