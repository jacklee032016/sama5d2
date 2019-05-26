One-Wire interface in Kernel
################################
05.22, 2019

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
