/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA5D2 Xplained Board.
 *
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

#define CONFIG_MISC_INIT_R

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS			1
#define CONFIG_SYS_SDRAM_BASE           	0x20000000

/*#define CONFIG_SYS_SDRAM_SIZE			0x20000000 */
#define CONFIG_SYS_SDRAM_SIZE		0x10000000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		0x218000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

/* SerialFlash */
#ifdef CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		30000000
#endif

#ifdef CONFIG_SD_BOOT

/* bootstrap + u-boot + env in sd card */
#undef CONFIG_BOOTCOMMAND

#define CONFIG_BOOTCOMMAND	"fatload mmc " CONFIG_ENV_FAT_DEVICE_AND_PART " 0x21000000 at91-sama5d2_xplained.dtb; " \
				"fatload mmc " CONFIG_ENV_FAT_DEVICE_AND_PART " 0x22000000 zImage; " \
				"bootz 0x22000000 - 0x21000000"

#elif CONFIG_SPI_BOOT

/* bootstrap + u-boot + env in sd card, but kernel + dtb in eMMC */
#undef CONFIG_BOOTCOMMAND

#define CONFIG_BOOTCOMMAND	"ext4load mmc 0:1 0x21000000 /boot/at91-sama5d2_xplained.dtb; " \
				"ext4load mmc 0:1 0x22000000 /boot/zImage; " \
				"bootz 0x22000000 - 0x21000000"

#endif

#ifdef CONFIG_QSPI_BOOT

 #if WITH_MUXLAB_OLD_VERSION
#define	_DTB_START 			"0xA0000"	/* offset: 640K */
#define	_DTB_SIZE 			"0x10000"	/* size: 64K */

#define	_KERNEL_START 		"0xB0000"	/* offset: 704KB=640+64 */
#define	_KERNEL_SIZE 		"0x400000"	/* size : 4MB */

#define	_RAMDISK_START		"0x4D0000"	/* offset: */
#define	_RAMDISK_SIZE		"0x1000000"	/* size: 16MB */
#else
#define	_DTB_START 			"0x40000"
#define	_DTB_SIZE 			"0x10000"

#define	_KERNEL_START 		"0x100000"
#define	_KERNEL_SIZE 		"0x500000"

#define	_RAMDISK_START		"0x600000"
#define	_RAMDISK_SIZE		"0x700000"
#endif

#undef CONFIG_ENV_SPI_BUS
#undef CONFIG_ENV_SPI_CS
#undef CONFIG_BOOTCOMMAND
#define CONFIG_ENV_SPI_BUS		1
#define CONFIG_ENV_SPI_CS		0

#if 1

#define CONFIG_BOOTCOMMAND              "sf probe 1:0; "				\
                                        ""
                                        
#else
#define CONFIG_BOOTCOMMAND              "sf probe 1:0; "				\
                                        "sf read 0x25000000 "_RAMDISK_START" "_RAMDISK_SIZE"; "		\
                                        "sf read 0x21000000 "_DTB_START" "_DTB_SIZE"; "		\
                                        "sf read 0x22000000 "_KERNEL_START" "_KERNEL_SIZE"; "	\
                                        "bootz 0x22000000 - 0x21000000"
#endif

/* SPL */
#define CONFIG_SPL_TEXT_BASE		0x200000
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#ifdef CONFIG_SD_BOOT
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"

#elif CONFIG_SPI_BOOT
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x10000

#endif

#endif

#endif

