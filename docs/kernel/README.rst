README for Bootstrap
################################

git clone git://github.com/linux4sam/linux-at91.git

git checkout linux-4.14-at91-rt

make ARCH=arm sama5_defconfig

make ARCH=arm menuconfig




CONFIG_ARCH_MULTI_V7=y
CONFIG_ARCH_MULTI_V6_V7=y
CONFIG_ARCH_AT91=y
CONFIG_SOC_SAMA5D2=y
