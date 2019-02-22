README for u-boot
################################
02.18, 2019



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
   
	