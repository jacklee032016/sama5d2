Work logs for u-boot
################################

05.19, 2019
-------------------
* Add Rules.mak into Makefile:
   * add MUX_FLAGS into build process to define BOARD and its name;
* Add support SAMA5d2 XPLAINED, Mux774;
* New build binary name as BOARD_NAME.u-boot.bin;


05.17, 2019
--------------------
* Modify environment partition: 0x10000(offset), 0x10000(size), and test with fw_printenv
    *file include/config/at91-sama4_common.h
::

	#define CONFIG_ENV_OFFSET		0x10000
	#define CONFIG_ENV_SIZE			0x10000		/* 64K, one sector */
	    

04.11, 2019
-------------------
* Add directory of board into default include path:
   * add config.mk in BOARD_DIR with ``PLATFORM_CPPFLAGS += -I$(srctree)/board/$(BOARDDIR)/``.
   