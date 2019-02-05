# Sama5d26 Project



1 Programming
================================================================================

Open bootconfig applet with device sama5d2x through JTAG link
::

   /d/Xilinx/sam-ba_3.2.2/sam-ba.exe -p j-link -d sama5d2 -a bootconfig
   /d/Xilinx/sam-ba_3.2.2/sam-ba.exe -p jlink -d sama5d2 -a bootconfig
   /d/Xilinx/sam-ba_3.2.1/sam-ba.exe -p j-link -d sama5d2 -a bootconfig
   /d/Xilinx/sam-ba_3.2.1/sam-ba.exe -p jlink -d sama5d2 -a bootconfig
   
   Opening J-Link with S/N '29424457'
   Found Atmel SAMA5D2x device
   Disabling watchdog
   Connection opened.
   Connection closed.

   
## 1.1 Debug

When RomBOOT is not enabled, it complains "SAMBA/Applet.qml:232: Error: Could not initialize applet (status: undefined)":

    Short the pins to make RomBOOT is loaded;

	
When try to program, it complains "qml/SAMBA/Applet.qml:232: Error: Could not initialize applet (status: 1)":

    Plug off the shorted pins, and begin to program;

	
When this happens, try later:
::

   $ sam-ba -p jlink -d sama5d2 -a bootconfig
   Opening J-Link with S/N '29424457'
   Error: Unsupported device   

   
2 Partitions
================================================================================
::

   13 ofpart partitions found on MTD device f0020000.spi
   Creating 13 MTD partitions on "f0020000.spi":
   0x000000000000-0x000000010000 : "at91bootstrap"
   0x000000010000-0x000000020000 : "uboot env"
   0x000000020000-0x0000000a0000 : "uboot"
   0x0000000a0000-0x0000000b0000 : "device tree"
   0x0000000b0000-0x0000004b0000 : "kernel"
   0x0000004b0000-0x0000004c0000 : "configuration"
   0x0000004c0000-0x0000004d0000 : "edid"
   0x0000004d0000-0x0000014d0000 : "rootfs"
   0x0000014d0000-0x0000018d0000 : "app"
   0x0000018d0000-0x0000018e0000 : "datacfg0"
   0x0000018e0000-0x0000018f0000 : "datacfg1"
   0x0000018f0000-0x000001900000 : "datacfg2"
   0x000001900000-0x000002000000 : "reserved"   

   
## 2.1 Compile DataFlashBoot

Clear all configuration:
::

    make mrproper

Configure sama5d2:
::

    make sama5d2_xplaineddf_qspi_uboot_defconfig

    make menuconfig

    make

	
	
