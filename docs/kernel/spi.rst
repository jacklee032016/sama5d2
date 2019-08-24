SPI device and driver
#######################################

xplaind board
===============================

MTD device driver
-------------------------------

dts definition:
+++++++++++++++++++++++++++++++

::

				m25p80@0 {
					compatible = "atmel,at25df321a";	/* this SPI in xplained board */
					reg = <0>;
					spi-max-frequency = <50000000>;
				};

dmesg
+++++++++++++++++++++++++++++++

::

	atmel_spi f8000000.spi: Using dma0chan0 (tx) and dma0chan1 (rx) for DMA transfers
	atmel_spi f8000000.spi: Using FIFO (16 data)
	Found Flash NOR ID:'at25df321a'
	Uniform erase size :65536
	m25p80 spi0.0: at25df321a (4096 Kbytes)
	atmel_spi f8000000.spi: Atmel SPI Controller version 0x311 at 0xf8000000 (irq 28)


device file
+++++++++++++++++++++++++++++++

add device file after QSPI partitions. For example, when 6 partitions of QSPI, then SPI flash is automatically
as /dev/mtd7, /dev/mtd7ro and /dev/mtdblock7.
	

cat /sys/bus/spi/drivers/m25p80/spi0.0/modalias 
spi:at25df321a



spidev driver
-------------------------------
This is driver for userspace interface for SPI device

dts definition:
+++++++++++++++++++++++++++++++

::

				spidev@0 {
					compatible = "atmel,at25df321a";
					#compatible = "spidev";
					reg = <0>;
					spi-max-frequency = <50000000>;

				m25p80@0 {
/*				spidev@0 {*/ 
					#compatible = "atmel,at25df321a";
					compatible = "spidev";
					reg = <0>;
					spi-max-frequency = <50000000>;
				};

so,
* spidev@0 or m25p80@0 can be used for this first(0) spi device;
* ``compatible = "spidev";`` is for spidev driver, which provides userspace interface for SPI device;
* `compatible = "atmel,at25df321a"`: assign device type and load standard MTD driver;
		

dmesg
+++++++++++++++++++++++++++++++

::

	atmel_spi f8000000.spi: Using dma0chan0 (tx) and dma0chan1 (rx) for DMA transfers
	atmel_spi f8000000.spi: Using FIFO (16 data)
	spidev spi0.0: buggy DT: spidev listed directly in DT
	------------[ cut here ]------------
	WARNING: CPU: 0 PID: 1 at drivers/spi/spidev.c:730 0xc041e408
	Modules linked in:
	CPU: 0 PID: 1 Comm: swapper Not tainted 4.14.73-linux4sam_6.0-rt44 #50
	Hardware name: Atmel SAMA5
	Function entered at [<c010d56c>] from [<c010ac14>]
	Function entered at [<c010ac14>] from [<c0116b9c>]
	Function entered at [<c0116b9c>] from [<c0116c64>]
	Function entered at [<c0116c64>] from [<c041e408>]
	Function entered at [<c041e408>] from [<c041b2d0>]
	Function entered at [<c041b2d0>] from [<c03b2dcc>]
	Function entered at [<c03b2dcc>] from [<c03b13fc>]
	Function entered at [<c03b13fc>] from [<c03b2adc>]
	Function entered at [<c03b2adc>] from [<c03b20cc>]
	Function entered at [<c03b20cc>] from [<c03b0574>]
	Function entered at [<c03b0574>] from [<c041bfac>]
	Function entered at [<c041bfac>] from [<c041ca10>]
	Function entered at [<c041ca10>] from [<c041cf70>]
	Function entered at [<c041cf70>] from [<c0420c40>]
	Function entered at [<c0420c40>] from [<c03b45b8>]
	Function entered at [<c03b45b8>] from [<c03b2dcc>]
	Function entered at [<c03b2dcc>] from [<c03b2f3c>]
	Function entered at [<c03b2f3c>] from [<c03b1354>]
	Function entered at [<c03b1354>] from [<c03b2354>]
	Function entered at [<c03b2354>] from [<c03b37d4>]
	Function entered at [<c03b37d4>] from [<c01018a8>]
	Function entered at [<c01018a8>] from [<c0700d14>]
	Function entered at [<c0700d14>] from [<c05e05a0>]
	Function entered at [<c05e05a0>] from [<c0107770>]
	---[ end trace 43756b237965da50 ]---
	atmel_spi f8000000.spi: Atmel SPI Controller version 0x311 at 0xf8000000 (irq 28)
	libphy: Fixed MDIO Bus: probed

spidev.c is driver for userspace access, so hardware is not check at all;

device file
+++++++++++++++++++++++++++++++

::

	crw-------    1 root     root      153,   0 Jun 11 02:34 /dev/spidev0.0

	cat /sys/bus/spi/drivers/spidev/spi0.0/modalias 
	spi:spidev
	
	
	read Flasd device with read command 0x9F:
	
	testMux -c spi
	testsMux 01.0.01-01, Aug 24 2019 22:39:42
	2019-06-11 02:44:03 [INFO,testMux]: [testMain.c-155]:Read flash ID #1:0x1f470100
	2019-06-11 02:44:03 [INFO,testMux]: [testMain.c-162]:Flash_id = 0x1F470100.
