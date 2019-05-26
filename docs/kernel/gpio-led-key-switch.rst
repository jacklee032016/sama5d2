README for GPIO-LEDS and GPIO-KEYS
#####################################
Jack Lee, 05.22, 2019

GPIO-Switch
========================================

* driver
--------------------
	* drvers/input/keyboard:
	   * gpio_keys.c and gpio_keys_polled.c
	* drivers/gpio
	

Device Tree
--------------------

::


				/* switches for 744. JL. 05.23, 2019 */
				pinctrl_switch_gpio_default: switch_gpio_default {
					pinmux = <PIN_PA18__GPIO>,
						<PIN_PA19__GPIO>,
						<PIN_PA20__GPIO>,
						<PIN_PA21__GPIO>;
					bias-pull-up;
				};


	MUX_DIP_SWITCH {
		compatible = "gpio-keys";

		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_switch_gpio_default>;
		
		dipSwitch01 {
			label = "DIP_SWITCH01";
			gpios = <&pioA PIN_PA18 GPIO_ACTIVE_LOW>;
			linux,code = <0>;	/* SW_LID */
			linux,input-type = <5>;	/* EV_SW */
			wakeup-source;
		};
		
		dipSwitch02 {
			label = "DIP_SWITCH02";
			gpios = <&pioA PIN_PA19 GPIO_ACTIVE_LOW>;
			linux,code = <1>;	/* SW_TABLET_MODE */
			linux,input-type = <5>;	/* EV_SW */
			wakeup-source;
		};
		
		dipSwitch03 {
			label = "DIP_SWITCH03";
			gpios = <&pioA PIN_PA20 GPIO_ACTIVE_LOW>;
			linux,code = <2>;	/* SW_HEADPHONE_INSERT */
			linux,input-type = <5>;	/* EV_SW */
			wakeup-source;
		};

		dipSwitch04 {
			label = "DIP_SWITCH04";
			gpios = <&pioA PIN_PA21 GPIO_ACTIVE_LOW>;
			linux,code = <3>;	/* SW_RFKILL_ALL */
			linux,input-type = <5>;	/* EV_SW */
			wakeup-source;
		};
	};



User space
--------------------
::

    /dev/input/event1 

		evtest --query /dev/input/event1 EV_SW SW_LID
		echo $? : when it is ON, return value of evtest is 10, otherwise it is 0;



GPIO-KEYS
========================================

* driver
--------------------
	* drvers/input/keyboard:
	   * gpio_keys.c and gpio_keys_polled.c
	* drivers/gpio
	
	DRIVERS/Input device Support/Keyboards/GPIO Buttons
	DRIVERS/Input device Support/Event interface


Device Tree
--------------------

::

	gpio_keys {
		compatible = "gpio-keys";

		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_key_gpio_default>;

		sw1 {
			label = "PB_SW1";
			gpios = <&pioA PIN_PC29 GPIO_ACTIVE_LOW>;
			linux,code = <108>;		/* KEY_DOWN, uapi/linux/input-event-codes.h */
			wakeup-source;
		};
	};


User space
--------------------
::

    /dev/input/event0 
	uapi/linux/input.h
	uapi/linux/input-event-codes.h

	Detect press and release event:
	
		Supported events:
			Event type 0 (Sync)
			Event type 1 (Key)
			Event code 108 (Down)
			
	
	refers to ``evtest``;


output of evtest: evtest /dev/input/event0

::

	Event: time 1558095612.336310, type 1 (Key), code 108 (Down), value 1
	Event: time 1558095612.336310, -------------- Report Sync ------------
	Event: time 1558095612.476319, type 1 (Key), code 108 (Down), value 0
	Event: time 1558095612.476319, -------------- Report Sync ------------

	
output of ``cat /dev/input/event0 | hexdump``

::

	0000000 a374 5cde cf9b 0006 0001 006c 0001 0000
	0000010 a374 5cde cf9b 0006 0000 0000 0000 0000
	0000020 a376 5cde b9d5 0007 0001 006c 0000 0000
	0000030 a376 5cde b9d5 0007 0000 0000 0000 0000
 	        /* timestamp    */ /*type code  value  */

			
GPIO-LEDS
========================================

* driver
--------------------
	* drvers/leds and drivers/gpio
	* Support SYSFS, led_trigger;
	
	LEDs can be controlled by GPIO directly; but it is more functional with gpio-led;

	DRIVERS/Input device Support/Export input device LEDs in sysfs



Device Tree
--------------------

::

	leds {
		compatible = "gpio-leds";
		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_led_gpio_default>;
		status = "okay"; /* conflict with pwm0 */

		act {
			label = "act";
			gpios = <&pioA PIN_PB9 GPIO_ACTIVE_HIGH>; /* high level is ON; default is OFF, eg. level is low */
		};


		power {
			label = "power";
			gpios = <&pioA PIN_PB5 GPIO_ACTIVE_HIGH>; /* high level is ON */
			default-state = "on";	/* default ON */
		};

		video {
			label = "video";
			gpios = <&pioA PIN_PB0 GPIO_ACTIVE_HIGH>; /* high level is ON */
			linux,default-trigger = "";		/* trigger : heartbeat|timer */
		};

		
After start up:
* POWER is ON, other 2 are OFF; 
* brightness=0, OFF; brightness!=0, ON;
* When trigger is used: ``echo "timer|heartbeat" > $LED/trigger``;
   * add trigger of 'timer', there are delay_on, delay_off for this LED;
   * add trigger of 'timer', there are delay_on, delay_off for this LED;
   * When trigger is used, brightness is useless;
* Disable Trigger: ``echo "none" > $LED/trigger``;
   * brightness can be used now;

User space
--------------------
::

    /sys/class/leds/POWER|VIDEO|ACT;
	


