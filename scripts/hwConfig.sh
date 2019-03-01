#!/bin/bash

HW_PORT=j-link
HW_BOARD=
HW_DEVICE=sama5d2

# when write to flash, only board type can be used; no serial flash defined for same device type
ATMEL_DEVICE="-d sama5d2"
ATMEL_DEVICE="-b sama5d2-xplained"


APPLET_INFO=bootconfig
APPLET_SERIAL=serialflash
APPLET_QSPI=qspiflash

# QSPI Flash,page:256 bytes; erase sector: 64KB
# 0x0000,0000 -- 0x0001,0000: bootstrap, 64KB,serial flash now
# 0x0001,0000 -- 0x0002,0000: NULL,64KB
# 0x0002,0000 -- 0x0003,0000: u-boot env,64KB
# 0x0003,0000 -- 0x0004,0000: NULL, 64KB
# 0x0004,0000 -- 0x0005,0000: Device Tree,64KB
# 0x0005,0000 -- 0x0006,0000: NULL, 64KB

# 0x0006,0000 -- 0x0010,0000: u-boot,640K
# 0x0010,0000 -- 0x0060,0000: kernel uImage, 6MB

# 0x0060,0000 -- 0x00D0,0000: RAM Disk, 7MB

#bootstrap size: 32K,only serial Flash
BOOTSTRAP_SIZE=0x8000

#bootstrap size: 64K, QSPI Flash
BOOTSTRAP_SIZE=0x10000

ENV_START=0x20000
ENV_SIZE=0x10000

# DTB, 64KB
DTB_START=0x40000
DTB_SIZE=0x10000


# uboot size: 512K
UBOOT_START=0x60000
UBOOT_SIZE=0xA0000

# kernel: 5MB
KERNEL_START=0x100000
KERNEL_SIZE=0x400000


# RAM Disk: 7MB
RAMDISK_START=0x600000
RAMDISK_SIZE=0x700000
