#!/bin/sh
# script in busybox, bash must be sh
# Write EEPROM of 23LC32A. 06.13, 2019

I2C_BUS=0

DEV_ADDR_EEPROM=0x57

EEPROM_ADDR="0x00 0x00"

EEPROM_VALUE=0xAA

READ_COUNT=32

echo "Write 10 bytes into address $EEPROM_ADDR..."
cmd="i2ctransfer -y $I2C_BUS w12@$DEV_ADDR_EEPROM $EEPROM_ADDR $EEPROM_VALUE-"
echo "    $cmd"
$cmd
echo ""
echo ""

echo "Read $READ_COUNT byte from address $EEPROM_ADDR..."
cmd="i2ctransfer -y $I2C_BUS w2@$DEV_ADDR_EEPROM $EEPROM_ADDR r$READ_COUNT"
echo "    $cmd"
$cmd
echo ""
