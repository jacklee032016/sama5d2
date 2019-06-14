#!/bin/sh
# script in busybox, bash must be sh

I2C_BUS=0

DEV_ADDR_MUX=0x70
DEV_ADDR_SENSOR=0x48

MUX_CHAN_SENSOR=0x05

SENSOR_ADDR_TOS=0x03
SENSOR_ADDR_TEMP=0x00

i2cset -y $I2C_BUS $DEV_ADDR_MUX $MUX_CHAN_SENSOR

echo "Read Tos..."
i2cget -y $I2C_BUS $DEV_ADDR_SENSOR $SENSOR_ADDR_TOS w

echo "Read temperature..."
i2cget -y $I2C_BUS $DEV_ADDR_SENSOR $SENSOR_ADDR_TEMP w
