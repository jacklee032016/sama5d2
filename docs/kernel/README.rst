README for Bootstrap
################################

* make image
--------------------
Add following lines in Makefile, otherwise 'make' will prompt parameters input:

::

   ARCH=arm
   export ARCH

In order to make dtb in kernel directory:

::

   make ARCH=arm dtbs or make dtbs

	result in arch/arm/boot/dts/m500774_at91-sama5d2_rx.dtb and m500774_at91-sama5d2_tx.dtb 


02.23, 2019
--------------------
* Support RAM Disk:
  * CONFIG_BLK_DEV_RAM_SIZE big to hold the ram disk;
  * Now it is 40MB as the requirement of RAM disk in build scripts;
  * 40MB is also command line from u-boot to kernel;
  * Only 20% is used, eg about 8MB, compressed ram disk is about 2.6MB;
  


git clone git://github.com/linux4sam/linux-at91.git

git checkout linux-4.14-at91-rt

make ARCH=arm sama5_defconfig

make ARCH=arm menuconfig




CONFIG_ARCH_MULTI_V7=y
CONFIG_ARCH_MULTI_V6_V7=y
CONFIG_ARCH_AT91=y
CONFIG_SOC_SAMA5D2=y
