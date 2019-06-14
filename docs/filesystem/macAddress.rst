MAC Address and Flash
#####################################
06.14, 2019

Set MAC Address 
========================
::

	saveenv : save environ into flash in u-boot
	fw_printenv: print save environ in OS:
	     * define /etc/fw_env.config as :


/etc/fw_env.config
::

	# Char device            offset within part
	# MTD device name	Device offset	Env. size	Flash sector size	Number of sectors
	/dev/mtd1		0x00000		0x10000		0x10000			

Notes:
* /dev/mtd1: the second partition of flash;
* 0x00000: start address of flash device, from first partition;
* 0x10000: start address of the environ partition;
* 0x10000: sizeo of the environ partition;


Problem
------------------------------
Command
::
	fw_setenv test foo: work fine; but after it run, fw_printenv can't found the correct environ;

	cat /dev/mtd1

Flash and File system
========================

* JFFS file system can't be built with erase size of 4K, too small. At least it is 8K;
* Flash has 1024 sectors, 64KB every sector; but it can be erased in 4KB, 64KB and 256Mb;
   * 

struct spi_nor_erase_map {
	struct spi_nor_erase_region	*regions;
	struct spi_nor_erase_region	uniform_region;
	struct spi_nor_erase_type	erase_type[SNOR_ERASE_TYPE_MAX];
	u8				uniform_erase_type;
};
   
   struct spi_nor_erase_type


io scheduler kyber registered
pinctrl-at91-pio4 fc038000.pinctrl: atmel pinctrl initialized
brd: module loaded
loop: module loaded
atmel_usart_serial.0.auto: ttyS0 at MMIO 0xf8020000 (irq = 36, base_baud = 5187500) is a ATMEL_SERIAL
console [ttyS0] enabled
atmel_usart_serial.1.auto: ttyS1 at MMIO 0xfc008000 (irq = 41, base_baud = 5187500) is a ATMEL_SERIAL
TEST: [drivers/mtd/spi-nor/atmel-quadspi.c-631.atmel_qspi_probe()]: 
TEST: [drivers/mtd/spi-nor/atmel-quadspi.c-714.atmel_qspi_probe()]: 
TEST:[drivers/mtd/spi-nor/atmel-quadspi.c-716]:Begin to scan NOR flash
TEST: [drivers/mtd/spi-nor/spi-nor.c-3382.spi_nor_select_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3313.spi_nor_select_uniform_erase()]: erase type:0x7; sector size:65536

TEST: [drivers/mtd/spi-nor/spi-nor.c-3315.spi_nor_select_uniform_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3318.spi_nor_select_uniform_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3318.spi_nor_select_uniform_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3322.spi_nor_select_uniform_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3325.spi_nor_select_uniform_erase()]: Erase Type No#d: size:2; shift:0; mask:-1;

TEST: [drivers/mtd/spi-nor/spi-nor.c-3318.spi_nor_select_uniform_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3322.spi_nor_select_uniform_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3325.spi_nor_select_uniform_erase()]: Erase Type No#d: size:1; shift:0; mask:-1;

TEST: [drivers/mtd/spi-nor/spi-nor.c-3318.spi_nor_select_uniform_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3322.spi_nor_select_uniform_erase()]: 
TEST: [drivers/mtd/spi-nor/spi-nor.c-3325.spi_nor_select_uniform_erase()]: Erase Type No#d: size:0; shift:0; mask:-1;

TEST: [drivers/mtd/spi-nor/spi-nor.c-3347.spi_nor_select_uniform_erase()]: 
 ERR:TEST: ERROR:[drivers/mtd/spi-nor/spi-nor.c-3386]:No uniform erase is found
atmel_qspi f0020000.spi: can't select erase settings supported by both the SPI controller and memory.
atmel_qspi: probe of f0020000.spi failed with error -22
atmel_spi f8000000.spi: DMA TX channel not available, SPI unable to use DMA
atmel_spi f8000000.spi: Atmel SPI Controller using PIO only
atmel_spi f8000000.spi: Using FIFO (16 data)
atmel_spi f8000000.spi: Atmel SPI Controller version 0x311 at 0xf8000000 (irq 28)
libphy: Fixed MDIO Bus: probed
CAN device driver interface
m_can: probe of f8054000.can failed with error -12