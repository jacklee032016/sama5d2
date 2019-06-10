Work logs for kernel
################################

06.10, 2019
--------------------
RTC, hardware clock and system 
* Enable kernel RTC:
    * system clock sync to hardware clock;
	* read hardware clock to system clock when OS start up;
	* RTC_DRV_AT91RM9200;
* Test 
    * date -s 06102019
    * hwclock -w|--systohc  # system clock to hardware clock
	* power off and power on;

	
05.27~06.02, 2019
--------------------
* enable w1(one-wire) and w1-gpio master;
* add DS28E15 as module in w1;
* disable wireless (WLAN in kernel: network/wireless, drivers/net/wireless);
* build kernel and output zImage file with extension of board name;
* add ptp support in macb driver: dts and its capacity definition in driver:
	* add device tree defintion for driver: ``compatible = "atmel,sama5d2-gem"``; 
	* add ``MACB_CAPS_GEM_HAS_PTP`` into capacity of ``sama5d2_config``;
	* disable pm of sama5d2_config;

	
05.26, 2019
--------------------
* When usb2: ehci@500000 {
			status = "okay";
		};
	enabled, kernel reboot when EHCI driver is loading;


05.23, 2019
--------------------
* Remove touchscreens;
* Remove media --> video capture;
* Remove Graphics;

About 3.5M size of kernel;


04.17, 2019
--------------------
* macb ethernet driver support 100MB/Full Duplex defaultly:
  * drivers/net/ethernet/cadence/macb_main.c
  * macb_init_hw()/macb_handle_link_change(): speed = SPEED_100;duplex=DUPLEX_FULL;
  * ethtool show it is 10MB/Half;
  * constants defined in include/uapi/linux/ethtool.h
  
04.11, 2019
--------------------

Type:n25q512ax3; ID:0x20ba20;

* drivers/mtd/spi-nor/spi-nor.c:spi_nor_select_erase(): select fixed erase size of 64K for mux7xx board;
* DTS file: enable `qspi` and disable `sdmmc1`; refer to dts file;
* add jffs2 support in: FS/MISC_FS/JFFS2
