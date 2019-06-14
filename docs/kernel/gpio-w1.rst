One-Wire interface in Kernel
################################
05.22, 2019


* Netlink
--------------------
* Enable ``drivers/connector``: this is Connector of socket link;
	* w1_netlink is built into Wire.ko;
* Only access the interface from master device, refer to ``enum w1_commands``;
	* access: command, data, length, for standard EEPROM;
* Interface of ``struct attribute`` from slave device can only access from sys file system;


* driver
--------------------
		

::

   DRIVERS/Dallas's 1-wire support/
      master: GPIO master
      Slave: W1_SLAVE_DS2805, 112-byte  


* DTB
--------------------
		

::

	onewire_device {
		compatible = "w1-gpio";
		gpios = <&pioA PIN_PD10 GPIO_ACTIVE_HIGH>; /* PH7 */
		
		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_led_gpio_default>;	
	};


* User Space
--------------------


		/sys/bus/w1/devices/w1_bus_master1
		/sys/devices/w1_bus_master1



Strong/Weak Pull-up		
--------------------
* Strong Pull-up:
	* Low resister value of pull-up resister;
	* More current flow when pin is broken;
	* Response much quick to the voltage variation in pin;
* Weak Pull-up:
	* High resister value;
	* Less curren flow when pin is broken;
	* Response more slower to the voltage variation in pin;

* Normally internal impedance of MCU pin is 1 MQ, R2=1M;
* pull-up resister is 10K~100K, R1=10K~100K;
	
	
	
* DS28E15 circuit design:
	* 768:
		* 1.0K, very small, like strong pull-up;
		* A Capacitor is connected, so the repsonse time is slower;
		* Software make the pin to high: MCU internal impedence is added??
	* 774: 
		* 2.0K, a little bit weak;
	
* Kernel driver: drivers/w1/slave/w1_ds2804.c:
	* When strong pull-up is on, no delay is needed for reading status;
		
Open Drain
--------------------
Term from semiconductor

*For w1-gpio driver:
  *	When OpenDrain is enabled:
     * Write 1|0, means output 1|0 to that pin directly;
  * When OpenDrain is disabled:
     * Write 1: set that pin to input mode, eg. external pull-up make it output as ;
	 * Write 0: output 0 to that pin;