Tests on u-boot
################################
03.07, 2019


Add debug supports into u-boot
================================

Add new include directory of u-boot:

* add config.mk into `$(srctree)/board/$(BOARDDIR)/`, which defines board specific rule
* add `PLATFORM_CPPFLAGS += -I$(srctree)/board/$(BOARDDIR)/` into this `config.mk` file;


Ethernet switch control
=============================

* configure as 100Mbps, network does not work:
   * New u-boot make ethernet work after old u-boot make ethernet switch work and then re-program it;
   
   * read the port status
      * When everything is OK:

::

   After configured:
     p4 : 4 2 1 1 1 1
     p5 : 4 2 1 0 1 1
     p6 : 2 2 1 1 1 1
   Read stattus:
     p4 : 4 2 0 1 1 1
     p5 : 4 2 1 0 1 1
     p6 : 2 2 1 0 1 1      


      * when it does not work:
      
::

   Before configured:
     p4 : 4 2 0 1 1 1
     p5 : 4 2 1 0 1 1
     p6 : 2 2 0 1 1 1   
   After configured:
     p4 : 4 2 1 1 1 1
     p5 : 4 2 1 0 1 1
     p6 : 2 2 1 1 1 1
   Read stattus:
     p4 : 4 2 0 1 1 1
     p5 : 4 2 1 0 1 1
     p6 : 2 2 0 1 1 1     
     
     

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
       