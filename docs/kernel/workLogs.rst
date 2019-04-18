Work logs for kernel
################################



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
