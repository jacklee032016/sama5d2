MMC readme on u-boot
################################
05.26, 2019

Device Tree
================================

Device Tree
-------------------

::

		sdmmc0: sdio-host@a0000000 {
			bus-width = <8>;
			pinctrl-names = "default";
			pinctrl-0 = <&pinctrl_sdmmc0_cmd_dat_default &pinctrl_sdmmc0_ck_cd_default>;
			status = "okay"; /* J.L. Xplained board, must enable it */
			u-boot,dm-pre-reloc;
		};

		/* this is not exist */
		sdmmc1: sdio-host@b0000000 {
			bus-width = <4>;
			pinctrl-names = "default";
			pinctrl-0 = <&pinctrl_sdmmc1_cmd_dat_default &pinctrl_sdmmc1_ck_cd_default>;
			status = "okay"; /*J.L.  conflict with qspi0 */
			u-boot,dm-pre-reloc;
		};


Device Info
-------------------

::

		=> mmc list
		sdio-host@a0000000: 0
		sdio-host@b0000000: 1
		=> 
		sdio-host@a0000000: 0
		sdio-host@b0000000: 1


		=> mmc info
		
		Device: sdio-host@a0000000
		Manufacturer ID: fe
		OEM: 14e
		Name: MMC04 
		Bus Speed: 52000000
		Mode : MMC High Speed (52MHz)
		Rd Block Len: 512
		MMC version 4.4.1
		High Capacity: Yes
		Capacity: 3.7 GiB
		Bus Width: 4-bit
		Erase Group Size: 512 KiB
		HC WP Group Size: 4 MiB
		User Capacity: 3.7 GiB
		Boot Capacity: 1 MiB ENH
		RPMB Capacity: 128 KiB ENH
		
In Linux, device is /dev/mmcblk0		
		

Parition info
-------------------

::
		mmc part:

		Partition Map for MMC device 0  --   Partition Type: DOS
		
		Part    Start Sector    Num Sectors     UUID            Type
		  1     8               760340          aa16da68-01     83
		  
u-boot
------------------

		mmc info
		128M*2018 Blocks = 262144, =0x40000
   tftpboot 0x21000000 App.ext4.2019_05_26
   mmc write 0x21000000 8 40000

Kernel Driver
------------------
* First parittion in kernel is /dev/mmcblk0p1, not p0;

* device files:
	 * mmcblk0;
	 * mmcblk0boot0;
	 * mmcblk0boot1;
	 * mmcblk0p1;
	 * mmcblk0rpmb;



		Waiting for root device /dev/mmcblk0p0...
		mmc0: new DDR MMC card at address 0001
		mmcblk0: mmc0:0001 MMC04G 3.66 GiB 
		mmcblk0boot0: mmc0:0001 MMC04G partition 1 1.00 MiB
		mmcblk0boot1: mmc0:0001 MMC04G partition 2 1.00 MiB
		mmcblk0rpmb: mmc0:0001 MMC04G partition 3 128 KiB
		 mmcblk0: p1
  
 