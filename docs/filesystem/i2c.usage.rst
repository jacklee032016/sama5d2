I2C device and access in kernel and User space
#########################################################
06.12, 2019 JL

Device list
============================
Default dtb defines:
#. i2c0: dts item
   * i2c@f8028000: address
   * device file: /dev/i2c-1
   * re-define the pins of i2c0: PD29(DATA)/PD30(CLK)
#. flx4: dts item
   * flexcom@fc018000: 
   * device file: /dev/i2c-0
#. i2c1: dts item
   * i2c@fc028000: 
   * device file: /dev/i2c-2
   * default pins: PD4/PD5

dmesg message
------------------------
::

   dmesg | grep i2c 

	at91_i2c f8028000.i2c: could not find pctldev for node /ahb/apb/pinctrl@fc038000/i2c0_default, deferring probe
	at91_i2c fc028000.i2c: could not find pctldev for node /ahb/apb/pinctrl@fc038000/i2c1_default, deferring probe
	at91_i2c fc018600.i2c: can't get DMA channel, continue without DMA support
	at91_i2c fc018600.i2c: Using FIFO (16 data)
	at91_i2c fc018600.i2c: AT91 i2c bus driver (hw version: 0x704).
	i2c /dev entries driver
	at91_i2c f8028000.i2c: can't get DMA channel, continue without DMA support
	at91_i2c f8028000.i2c: Using FIFO (16 data)
	at91_i2c f8028000.i2c: AT91 i2c bus driver (hw version: 0x704).
	at91_i2c fc028000.i2c: can't get DMA channel, continue without DMA support
	at91_i2c fc028000.i2c: Using FIFO (16 data)
	at91_i2c fc028000.i2c: AT91 i2c bus driver (hw version: 0x704).


usage of i2c tools
============================

detect bus and devices
------------------------

::

    i2cdetect -l: list all devices
	i2cdetect -y 1
	
	i2cdetect -y 1
		 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
	10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
	20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
	30: 30 31 -- 33 34 35 36 37 -- -- -- -- -- -- -- -- 
	40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- -- 
	50: -- -- -- -- -- -- -- 57 -- -- -- UU -- -- -- -- 
	60: 60 -- -- -- 64 -- -- -- -- -- -- -- -- -- -- -- 
	70: 70 -- -- -- -- -- -- --      	
	

read from sensor
------------------------

::

    i2cset -y 0 0x70 0x05: Set channel 1 of I2C multiplxer
	
	# read tos
	i2cget -y 0 0x48 0x03 w: read 2 bytes(word) from address 0x03, eg. pointer register is 0x03 
		sensor device address: 0x48(0x90)
		sensor register address: 0x03, Tos register
		read out: 0x0050, 0x50 is MSB
		
	i2ctransfer -y -v 0 w1@0x48 0x03 r2
		bus 0:
		write 1 byte at device 0x48
		write 1 byte of 0x03, pointer register
		read 2 bytes
	

	# read temperature
	i2cget -y 0 0x48 0x00 w: 
		read out: 0x203b, 3B is MSB; 0x20, LSB, low 5 bits are useless;
		
	i2ctransfer -y -v 0 w1@0x48 0x00 r2
	

Devices on bus-0
============================

channel-0
------------------------
::

		root@mux744:~# i2cdetect -y 0
			 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
		00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
		10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		30: 30 31 -- 33 34 35 36 37 -- -- -- -- -- -- -- -- 
		40: -- -- -- -- -- -- 46 -- -- -- -- -- -- -- -- -- 
		50: -- -- -- -- -- -- -- -- -- -- -- UU -- -- -- -- 
		60: 60 -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		70: 70 -- -- -- -- -- -- --   

                      
channel-1
------------------------
::

		root@mux744:~# i2cset -y 0 0x70 0x05
		root@mux744:~# i2cdetect -y 0
			 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
		00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
		10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		30: 30 31 -- 33 34 35 36 37 -- -- -- -- -- -- -- -- 
		40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- -- 
		50: -- -- -- -- -- -- -- 57 -- -- -- UU -- -- -- -- 
		60: 60 -- -- -- 64 -- -- -- -- -- -- -- -- -- -- -- 
		70: 70 -- -- -- -- -- -- --                         

channel-2
------------------------
::

		root@mux744:~# i2cset -y 0 0x70 0x06
		root@mux744:~# i2cdetect -y 0
			 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
		00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
		10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		30: 30 31 -- 33 34 35 36 37 -- -- -- -- -- -- -- -- 
		40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		50: -- -- -- -- 54 -- -- -- -- -- -- UU -- -- -- -- 
		60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		70: 70 -- -- -- -- -- -- --                         

channel-3
------------------------
::

		root@mux744:~# i2cset -y 0 0x70 0x07
		root@mux744:~# i2cdetect -y 0
			 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
		00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
		10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		30: 30 31 -- 33 34 35 36 37 -- -- -- -- -- -- -- -- 
		40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		50: -- -- -- -- -- -- -- -- -- -- -- UU -- -- -- -- 
		60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
		70: 70 -- -- -- -- -- -- --                         


Notes
------------------------
*i2cdump: 
+++++++++++++++++++++++
   * dump/read one byte from every address; 
   * but for PCT2072, there are one or two bytes in one address;
   * in this case only MSB(eg the first byte) is read out;


Atmel Flexcom 
+++++++++++++++++++++++
(Flexible Serial Communication Unit)

The Atmel Flexcom is just a wrapper which embeds a SPI controller, an I2C
controller and an USART. Only one function can be used at a time and is chosen
at boot time according to the device tree.

