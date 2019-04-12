Read me for Yocto Sama5d2
################################
April 8th, 2019


Tune
============================

* Remove kernel-image from image of file system
---------------------------------------------------

   in MACHINE definition of meta-atmel/conf/machine/sama5d2-xplained.conf:
   
   change as following

::
   
    IMAGE_INSTALL_append = " kernel-image kernel-devicetree"

    IMAGE_INSTALL_append = " kernel-devicetree"


* Python and flask/flask-restful
--------------------------------------------------
in BUILD/conf/local.conf

::

   CORE_IMAGE_EXTRA_INSTALL = "python-core python-pip python-flask-restful python-flask"   

   It should be like the following to support python3
   
   CORE_IMAGE_EXTRA_INSTALL = "python3-core python3-pip python3-flask python3-flask-restful" 


* udhcpc and busybos configuration
-------------------------------------------------
    bitbake -c menuconfig busybox: execure command `menuconfig` on package 'busybox';
    bitbake busybox : build package `busybox`;
    bitbake core-image-minimal: rebuild image from packages;  



Build
============================


run command
-----------------
enter `$POKY/build-microchip`:

::

   export OEROOT=../
   source ../scripts/oe-buildenv-internal

   # only SDK
   ../bitbake/bin/bitbake core-image-base -c populate_sdk 
   ../bitbake/bin/bitbake atmel-xplained-demo-image -c populate_sdk : only SDK is build

   # only images
   
   
   ../bitbake/bin/bitbake core-image-minimal: 
       core-image-minimal-sama5d2-xplained-20190409140119.rootfs.tar.gz
       about 23.1 MB after gzipped;
       uncompressed: 65M: python2.7: 48M, zImage: 3M;
       
       No python, only 11MB;
   
   
   ../bitbake/bin/bitbake core-image-base: 
       core-image-base-sama5d2-xplained-20190409134835.rootfs.tar.gz
       about 35.4 MB after gzipped;
   
   ../bitbake/bin/bitbake atmel-demo-image: 
       atmel-demo-image-sama5d2-xplained-20190409133932.rootfs.tar.gz,
       about 57MB after gzipped;
   
   ../bitbake/bin/bitbake atmel-xplained-demo-image: 
       atmel-xplained-demo-image-sama5d2-xplained-20190409131036.rootfs.tar.gz
       based on atmel-demo-image, adding nodejs nodejs-npm and mpio, etc.;
       about 68MB after gzipped;
   


bitbake core-image-minimal
bitbake atmel-qt5-demo-image


Configuration
-----------------

local.conf
++++++++++++++++++++++++

::

   MACHINE ?= "sama5d2-xplained"
   
   DL_DIR ?= "/home/lzj/yocto/downloads"
   
   DISTRO = "poky-atmel"
   
   PACKAGE_CLASSES ?= "package_ipk"
   
   USER_CLASSES ?= "buildstats image-mklibs"
   


bblayers.conf
++++++++++++++++++++++++

::

   ${BSPDIR}/poky/meta \
   ${BSPDIR}/poky/meta-poky \
   ${BSPDIR}/poky/meta-yocto-bsp \
   ${BSPDIR}/meta-atmel \
   ${BSPDIR}/meta-openembedded/meta-oe \
   ${BSPDIR}/meta-openembedded/meta-networking \
   ${BSPDIR}/meta-openembedded/meta-python \
   ${BSPDIR}/meta-openembedded/meta-multimedia \
   