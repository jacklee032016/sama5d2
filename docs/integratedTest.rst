setenv ipaddr 192.168.168.181
setenv gatewayip 192.168.168.1
setenv ethaddr fc:c2:3d:0d:16:13
printenv

tftpboot 0x21000000 mux774_v2.dtb;sf probe 1:0; sf erase 0x20000 0x10000; sf write 0x21000000 0x20000 0x10000

tftpboot 0x21000000 zImage.Mux774; sf probe 1:0; sf erase 0x100000 0x400000; sf write 0x21000000 0x100000 0x400000

tftpboot 0x21000000 root.jffs2.2019_04_10
sf probe 1:0; sf erase 0x500000 0x3000000;

sf write 0x21000000 0x500000 0xXXXX


scp drivers/sc/w1Ds28e15.ko root@192.168.168.103:/


scp mux7xxApp_7.3.0_arm_debug_2019_08_19.zip root@192.168.168.103:/var/volatile

scp ./Linux.bin.arm/lib/libCmnSys.so   root@192.168.168.103:/lib/


read registers from FPGA:
i2cdump -f -y 0 0x30

version
i2cget -f -y 0 0x30 0x1d
revision
i2cget -f -y 0 0x30 0x1e

local port V
i2cset -f -y 0 0x34 0x20 0x40

i2cget -f -y 0 0x34 0x20

i2cget -f -y 0 0x34 0x21

width

i2cget -f -y 0 0x30 0x06

