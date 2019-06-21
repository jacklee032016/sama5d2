/*
 * w1-gpio - GPIO w1 bus master driver
 *
 * Copyright (C) 2007 Ville Syrjala <syrjala@sci.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/delay.h>

#include <linux/w1.h>
#include <linux/w1-gpio.h>


#define	__EXT_RELEASE__

#include "mux7xxCompact.h"

static u8 w1_gpio_set_pullup(void *data, int delay)
{
	struct w1_gpio_platform_data *pdata = data;

	if (delay) {
		pdata->pullup_duration = delay;
	} else {
		if (pdata->pullup_duration) {
			gpio_direction_output(pdata->pin, 1);

			msleep(pdata->pullup_duration);

			gpio_direction_input(pdata->pin);
		}
		pdata->pullup_duration = 0;
	}

	return 0;
}

/* Not Open Drain, this is used by DS28E15 */
static void w1_gpio_write_bit_dir(void *data, u8 bit)
{
	struct w1_gpio_platform_data *pdata = data;

	if (bit)
		gpio_direction_input(pdata->pin);
	else
		gpio_direction_output(pdata->pin, 0);
}


static void w1_gpio_write_bit_val(void *data, u8 bit)
{
	struct w1_gpio_platform_data *pdata = data;

	gpio_set_value(pdata->pin, bit);
}

static u8 w1_gpio_read_bit(void *data)
{
	struct w1_gpio_platform_data *pdata = data;

	return gpio_get_value(pdata->pin) ? 1 : 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id w1_gpio_dt_ids[] = {
	{ .compatible = "w1-gpio" },
	{}
};
MODULE_DEVICE_TABLE(of, w1_gpio_dt_ids);
#endif

static int w1_gpio_probe_dt(struct platform_device *pdev)
{
	struct w1_gpio_platform_data *pdata = dev_get_platdata(&pdev->dev);
	struct device_node *np = pdev->dev.of_node;
	int gpio;

	pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	if (of_get_property(np, "linux,open-drain", NULL))
	{
		pdata->is_open_drain = 1;
	}

	gpio = of_get_gpio(np, 0);
	if (gpio < 0) {
		if (gpio != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Failed to parse gpio property for data pin (%d)\n", gpio);

		return gpio;
	}
	pdata->pin = gpio;
	EXT_DEBUGF(MUX_W1_DEBUG, "GPIO pin %d", gpio);

	gpio = of_get_gpio(np, 1);
	if (gpio == -EPROBE_DEFER)
		return gpio;
	/* ignore other errors as the pullup gpio is optional */
	pdata->ext_pullup_enable_pin = gpio;

	pdev->dev.platform_data = pdata;

	if(of_property_read_u8(np, "tSlot", &pdata->rwParams.tSlot))
	{
		EXT_ERRORF("parse tSlot failed");
		pdata->rwParams.tSlot = 13;
	}
	if(of_property_read_u8(np, "tW1L", &pdata->rwParams.tW1L))
	{
		EXT_ERRORF("parse tW1L failed");
		pdata->rwParams.tW1L = 1;
	}
	if(of_property_read_u8(np, "tW0L", &pdata->rwParams.tW0L))
	{	
		EXT_ERRORF("parse tW0L failed");
		pdata->rwParams.tW0L = 9;
	}
	if(of_property_read_u16(np, "tRdDelta", &pdata->rwParams.tRdDelta))
	{
		EXT_ERRORF("parse tRdDelta failed");
		pdata->rwParams.tRdDelta = 500;
	}

	EXT_INFOF("W1-gpio: tSlot:%d; tW1L:%d; tW0L:%d; tRdDelta:%d", pdata->rwParams.tSlot, pdata->rwParams.tW1L, pdata->rwParams.tW0L, pdata->rwParams.tRdDelta);
	return 0;
}


/* it is called when master driver is loaded*/
static int w1_gpio_probe(struct platform_device *pdev)
{
	struct w1_bus_master *master;
	struct w1_gpio_platform_data *pdata;
	int err;

//	EXT_INFOF("%s on %s is initialize...", pdev->name, BOARD_NAME );
	EXT_INFOF("Master W1-GPIO on %s is initialize...", BOARD_NAME );
	if (of_have_populated_dt())
	{
		err = w1_gpio_probe_dt(pdev);
		if (err < 0)
			return err;
	}

	pdata = dev_get_platdata(&pdev->dev);

	if (!pdata) {
		dev_err(&pdev->dev, "No configuration data\n");
		return -ENXIO;
	}

	master = devm_kzalloc(&pdev->dev, sizeof(struct w1_bus_master), GFP_KERNEL);
	if (!master) {
		dev_err(&pdev->dev, "Out of memory\n");
		return -ENOMEM;
	}

	err = devm_gpio_request(&pdev->dev, pdata->pin, "w1");
	if (err) {
		dev_err(&pdev->dev, "gpio_request (pin) failed\n");
		return err;
	}

	EXT_DEBUGF(MUX_W1_DEBUG, "W1 pin:%d", pdata->pin );
	if (gpio_is_valid(pdata->ext_pullup_enable_pin))
	{
		
		EXT_DEBUGF(MUX_W1_DEBUG, "W1 pullup pin enabled" );
		err = devm_gpio_request_one(&pdev->dev,
				pdata->ext_pullup_enable_pin, GPIOF_INIT_LOW,
				"w1 pullup");
		if (err < 0)
		{
			dev_err(&pdev->dev, "gpio_request_one "
					"(ext_pullup_enable_pin) failed\n");
			return err;
		}
		
		EXT_DEBUGF(MUX_W1_DEBUG, "W1 pullup pin:%d", pdata->ext_pullup_enable_pin );
	}


	master->data = pdata;
	master->read_bit = w1_gpio_read_bit;

	if (pdata->is_open_drain)
	{
		gpio_direction_output(pdata->pin, 1);
		master->write_bit = w1_gpio_write_bit_val;
	}
	else
	{
		gpio_direction_input(pdata->pin);
		master->write_bit = w1_gpio_write_bit_dir;
		master->set_pullup = w1_gpio_set_pullup;
	}

	/* copy param before register into master */
	memcpy(&master->rwParams, &pdata->rwParams, sizeof(struct w1_rw_param) );
	EXT_INFOF("W1-master: tSlot:%d; tW1L:%d; tW0L:%d; tRdDelta:%d", master->rwParams.tSlot, master->rwParams.tW1L, master->rwParams.tW0L, master->rwParams.tRdDelta);
	err = w1_add_master_device(master);
	if (err) {
		dev_err(&pdev->dev, "w1_add_master device failed\n");
		return err;
	}

	if (pdata->enable_external_pullup)
		pdata->enable_external_pullup(1);

	if (gpio_is_valid(pdata->ext_pullup_enable_pin))
		gpio_set_value(pdata->ext_pullup_enable_pin, 1);

	platform_set_drvdata(pdev, master);

	EXT_INFOF("Master W1-GPIO %s is probed now", BOARD_NAME);
	return 0;
}

static int w1_gpio_remove(struct platform_device *pdev)
{
	struct w1_bus_master *master = platform_get_drvdata(pdev);
	struct w1_gpio_platform_data *pdata = dev_get_platdata(&pdev->dev);

	if (pdata->enable_external_pullup)
		pdata->enable_external_pullup(0);

	if (gpio_is_valid(pdata->ext_pullup_enable_pin))
		gpio_set_value(pdata->ext_pullup_enable_pin, 0);

	w1_remove_master_device(master);

	return 0;
}

static int __maybe_unused w1_gpio_suspend(struct device *dev)
{
	struct w1_gpio_platform_data *pdata = dev_get_platdata(dev);

	if (pdata->enable_external_pullup)
		pdata->enable_external_pullup(0);

	return 0;
}

static int __maybe_unused w1_gpio_resume(struct device *dev)
{
	struct w1_gpio_platform_data *pdata = dev_get_platdata(dev);

	if (pdata->enable_external_pullup)
		pdata->enable_external_pullup(1);

	return 0;
}

static SIMPLE_DEV_PM_OPS(w1_gpio_pm_ops, w1_gpio_suspend, w1_gpio_resume);

static struct platform_driver w1_gpio_driver = {
	.driver = {
		.name	= "w1-gpio",
		.pm	= &w1_gpio_pm_ops,
		.of_match_table = of_match_ptr(w1_gpio_dt_ids),
	},
	.probe = w1_gpio_probe,
	.remove = w1_gpio_remove,
};

module_platform_driver(w1_gpio_driver);

MODULE_DESCRIPTION("GPIO w1 bus master driver");
MODULE_AUTHOR("Ville Syrjala <syrjala@sci.fi>");
MODULE_LICENSE("GPL");
