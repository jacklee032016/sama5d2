README for Kernel debugging
################################
April 11, 2019 Jack Lee

* 4.14.73 in yocto 
===========================
4.14.74(gcc 7.3.1), update from 4.9.75(gcc 5.4.1)

::

   atmel_usart_serial atmel_usart_serial.0.auto: using dma0chan8 for rx DMA transfers
   atmel_usart_serial atmel_usart_serial.0.auto: using dma0chan9 for tx DMA transfers
   VFS: Cannot open root device "mtdblock7" or unknown-block(0,0): error -19
   Please append a correct "root=" boot option; here are the available partitions:
   0100            8192 ram0 
    (driver?)
   0101            8192 ram1 
    (driver?)
   0102            8192 ram2 
    (driver?)
   0103            8192 ram3 
   (driver?)
   Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0)
   ---[ end Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0)


Correct message

::

   at91_i2c fc018600.i2c: can't get DMA channel, continue without DMA support
   at91_i2c fc018600.i2c: Using FIFO (16 data)
   at91_i2c fc018600.i2c: AT91 i2c bus driver (hw version: 0x704).
   atmel_qspi f0020000.spi: n25q512ax3 (65536 Kbytes)
   9 ofpart partitions found on MTD device f0020000.spi
   Creating 9 MTD partitions on "f0020000.spi":
   0x000000000000-0x000000010000 : "at91bootstrap"
   0x000000010000-0x000000020000 : "bootloader env"
   0x000000020000-0x0000000a0000 : "bootloader"
   0x0000000a0000-0x0000000b0000 : "device tree"
   0x0000000b0000-0x0000004b0000 : "kernel"
   0x0000004b0000-0x0000004c0000 : "configuration"
   0x0000004c0000-0x0000004d0000 : "edid"
   0x0000004d0000-0x0000034d0000 : "rootfs"
   0x0000034d0000-0x000004000000 : "reserved"
   atmel_spi f8000000.spi: Using dma0chan0 (tx) and dma0chan1 (rx) for DMA transfers
   atmel_spi f8000000.spi: Using FIFO (16 data)
   atmel_spi f8000000.spi: Atmel SPI Controller version 0x311 at 0xf8000000 (irq 28)
   libphy: Fixed MDIO Bus: probed
   libphy: MACB_mii_bus: probed

Wrong message

::

   at91_i2c fc018600.i2c: AT91 i2c bus driver (hw version: 0x704).
   atmel_qspi f0020000.spi: can't select erase settings supported by both the SPI controller and memory.
   atmel_qspi: probe of f0020000.spi failed with error -22
   atmel_spi f8000000.spi: Using dma0chan0 (tx) and dma0chan1 (rx) for DMA transfers
   atmel_spi f8000000.spi: Using FIFO (16 data)
   atmel_spi f8000000.spi: Atmel SPI Controller version 0x311 at 0xf8000000 (irq 28)


So QSPI and SPI drivers are both used, and must defined in dts file;


Fixed
----------------------------
Type:n25q512ax3; ID:0x20ba20;

* drivers/mtd/spi-nor/spi-nor.c:spi_nor_select_erase(): select fixed erase size of 64K for mux7xx board;
* DTS file: enable `qspi` and disable `sdmmc1`; refer to dts file;
* add jffs2 support in: FS/MISC_FS/JFFS2



Problems
----------------------------

#. at91-sama5d2_adc: probe of fc030000.adc failed with error -22
#. hwclock: can't open '/dev/misc/rtc': No such file or directory
#. /etc/rcS.d/S03systemd-udevd: line 103: can't create /proc/sys/kernel/hotplug: nonexistent directory

