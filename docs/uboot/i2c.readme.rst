I2C access and debug on u-boot
################################
04.16, 2019

I2c driver in u-boot
=============================
For device with only byte register address 
----------------------------------------------

`dm_i2c_probe()` (in i2c_uclass.c) create `dm_i2c_chip` which only support 1-byte register address;

`dm_i2c_read|dm_i2c_write()` only can read/write with 1-byte register address;

At least, for the `generic` driver of i2c;

It can be used to access the temperature sensor;

For devices with more than 1 byte address
---------------------------------------------
Such as rtl8307 switch chip whose size of register adress is 3 bytes, the `dm_i2c_xfer()` must be used in read/write reigster;

read: 
++++++++++
an arry of 2 items of i2c_msg must be used:

* the first one is for address of register;
* the second one is for read operation (defined in flags field of i2c_msg) and buffer for data;

write: 
++++++++++
Only one i2c_msg is needed:

* flags is 0 in bit1, the default value is for write operation;
* the first part of buffer is address, the second part of buffer is the data buffer;

refers to the implementation of `dm_i2c_read|dm_i2c_write()` in i2c_uclass;



I2C multiplexer
=============================
Only channel configuration operation can be support by the multiplexer; after channel has been configured, the multiplexer is transparent for the device behide it, 
except when access it directly with multiplexer's address;

For example, in command line of u-boot:
::

   i2c dev 0
   i2c probe

Now only devices before multiplexer and multiplexer can be seen;

::

   i2c mw 0x70 0x0 0x06
   i2c probe
   
Now devices connect to the channel 2 of multiplexer can be show also. here, **0x70** is device address of multiplexer, **0x0** is address, 
it is mandatory requirement of i2c command in u-boot; **0x06** is channel 2 (2+4) of multiplexer, for 774 board, it is channel connect to switch chip;



I2C bus operation
=============================

xplaind board
---------------------
0x54 EEPROM on I2C bus-1, no device on bus-0;

Device tree:
::

			i2c1: i2c@fc028000 {
				pinctrl-names = "default";
				pinctrl-0 = <&pinctrl_i2c1_default>;
				status = "okay";

				i2c_eeprom: i2c_eeprom@5c {
					compatible = "atmel,24mac402";
					reg = <0x5c>;
				};
				
			};


Write to i2c device
::

   i2c bus: display all device on buses
       Bus 1:  i2c@fc028000  (active 1)
       5c: i2c_eeprom@5c, offset len 1, flags 0
       
   ic2 dev 1: use bus 1
   i2c mw 54 2.1 0xAA : write to device 0x54, address 2, adress length 1, value 0xAA; no message when it is OK;


Read from i2c device
::

   i2c md 54 0.1 0x10 
   0000: 55 00 aa 00 00 00 00 00 00 00 00 00 00 00 00 00    U...............
  

Probe
::

   i2c probe
   Valid chip addresses: 34 54


Read
# Read from the device to memeory address
   i2c read <chip> <chip address> <length> <memory address>
   
   i2c read 0x54 0x0 10  0x22000000
   md 0x22000000  : display memory location
   
Error:
::
  
   i2c dev 0: failed, because now bus-0 is added in device tree;
   