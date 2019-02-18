#!/bin/bash

HW_PORT=jlink
HW_BOARD=
HW_DEVICE=sama5d2

# when write to flash, only board type can be used; no serial flash defined for same device type
ATMEL_DEVICE="-d sama5d2"
ATMEL_DEVICE="-b sama5d2-xplained"


APPLET_INFO=bootconfig
APPLET_SERIAL=serialflash
APPLET_QSPI=qspiflash



#bootstrap size: 32K
BOOTSTRAP_SIZE=0x8000

#bootstrap size: 128K
BOOTSTRAP_SIZE=0x20000

# uboot size: 512K
UBOOT_SIZE=0x80000


