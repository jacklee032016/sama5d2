README for u-boot
################################
02.18, 2019


uImage and zImage
-------------------------------
zImage: from kernel building, it is extracted by itselt; no mkimage is needed;
uImage: mkimage, like that "mkimage -A arm -O linux -C none -T kernel -a $MEM_ADDRESS -e $MEM_ADDRESS -n $NAME -d zImage uImage"


02.26, 2019
-------------------------------
Device Tree:

mkdir -p arch/arm/dts/ ; (cat arch/arm/dts/at91-sama5d2_xplained.dts; ) > arch/arm/dts/.at91-sama5d2_xplained.dtb.pre.tmp; arm-linux-gnueabi-gcc -E -Wp,-MD,arch/arm/dts/.at91-sama5d2_xplained.dtb.d.pre.tmp -nostdinc -I./arch/arm/dts -I./arch/arm/dts/include -Iinclude -I./include -I./arch/arm/include -include ./include/linux/kconfig.h -D__ASSEMBLY__ -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/dts/.at91-sama5d2_xplained.dtb.dts.tmp arch/arm/dts/.at91-sama5d2_xplained.dtb.pre.tmp ; ./scripts/dtc/dtc -O dtb -o arch/arm/dts/at91-sama5d2_xplained.dtb -b 0 -i arch/arm/dts/  -Wno-unit_address_vs_reg -Wno-simple_bus_reg -Wno-unit_address_format -Wno-pci_bridge -Wno-pci_device_bus_num -Wno-pci_device_reg -Wno-graph_child_address  -d arch/arm/dts/.at91-sama5d2_xplained.dtb.d.dtc.tmp arch/arm/dts/.at91-sama5d2_xplained.dtb.dts.tmp ; cat arch/arm/dts/.at91-sama5d2_xplained.dtb.d.pre.tmp arch/arm/dts/.at91-sama5d2_xplained.dtb.d.dtc.tmp > arch/arm/dts/.at91-sama5d2_xplained.dtb.d
make -f ./scripts/Makefile.build obj=arch/arm/dts arch/arm/dts/at91-sama5d2_xplained.dtb
make[2]: `arch/arm/dts/at91-sama5d2_xplained.dtb' is up to date.
test -e arch/arm/dts/at91-sama5d2_xplained.dtb || (						\
	echo >&2;							\
	echo >&2 "Device Tree Source is not correctly specified.";	\
	echo >&2 "Please define 'CONFIG_DEFAULT_DEVICE_TREE'";		\
	echo >&2 "or build with 'DEVICE_TREE=<device_tree>' argument";	\
	echo >&2;							\
	/bin/false)
  cat arch/arm/dts/at91-sama5d2_xplained.dtb > dts/dt.dtb
  ./tools/fdtgrep -b u-boot,dm-pre-reloc -b u-boot,dm-spl -RT arch/arm/dts/at91-sama5d2_xplained.dtb -n /chosen -n /config -O dtb | ./tools/fdtgrep -r -O dtb - -o dts/dt-spl.dtb 
  cat u-boot-nodtb.bin dts/dt.dtb > u-boot-dtb.bin
  cp u-boot-dtb.bin u-boot.bin


  
02.22, 2019
----------------
* Only support QSPI bootup, CONFIG_QSPI_BOOT, disable CONFIG_SPI_BOOT
  * when both options are enabled, something weired happens, such as env can't be erased because the erase size is SPI,not QSPI;
* Add ipaddr, sererip and mask definition defaultly in env.c;
* Move environment to 0x2,0000 and size of 0x1,0000;
* Load DTB from 0x4,0000, and size of 0x1,0000;
* Load kernel from 0x10,0000, and size of 0x50,0000;
* Load RAM disk from 0x60,0000, and size of 0x70,0000;
* bootarg: initrd=0x25000000,40M root=/dev/ram0 rw init=/linuxrc console=ttyS0,115200 earlyprintk



baudrate=115200
bootargs=console=ttyS0,115200 root=/dev/mmcblk0p1 rw rootfstype=ext4 rootwait atmel.pm_modes=standby,ulp1
bootcmd=ext4load mmc 0 0x24000000 boot/sama5d2_xplained.itb; bootm 0x24000000#kernel_dtb
bootdelay=1
ethaddr=fc:c2:3d:0d:16:13
fdtcontroladdr=3fb773c8
stderr=serial@f8020000
stdin=serial@f8020000
stdout=serial@f8020000

Environment size: 360/8188 bytes


scripts/Makefile.autoconf:
	create arch/$(ARCH)/include/asm/arch

	arch/arm/mach-at91/include/mach  --> arch/arm/include/asm/arch
	
Add into repository:

* remove include/config and arch/arm/include/arch
	

Build:
==================
* Host build:

   * scripts/dtc: dtc
   * tools
   
* arch/arm:   

   * cpu
   * cpu/armv7
   * lib
   * mach-at91

* board/atmel:

   * common
   * sama5d2-xplained

* cmd
* common
* common/init
* disk
* drivers

* env
   
	