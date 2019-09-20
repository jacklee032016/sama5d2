#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/io.h>
#include <linux/irq.h>

#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/delay.h>


#include "mux7xxCompact.h"

#define	DRIVER_NAME		"muxIrDriver"

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

static int ir_probe(struct platform_device *pdev)
{
//	struct uio_info *uioinfo;
	struct resource		*regs;
	struct resource *r = &pdev->resource[0];


	EXT_INFOF("devce:'%s' probed", pdev->dev.of_node->name);
	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs)
	{
		EXT_ERRORF("Memory resource failed");
		return -ENXIO;
	}

	EXT_INFOF("devce:'%s' mem: '%s'", pdev->dev.of_node->name, regs->name);

//	uioinfo->name = dev->dev.of_node->name /* name from device tree: "my_device" */
//	uioinfo->mem[0].addr = r->start; /* device address from device tree */
//	uioinfo->mem[0].size = resource_size(r); /* size from device tree */

	return 0;

}

static int ir_remove(struct platform_device *pdev)
{
//	struct ds1wm_data *ds1wm_data = platform_get_drvdata(pdev);

//	w1_remove_master_device(&ds1wm_master);
//	ds1wm_down(ds1wm_data);

	EXT_INFOF("devce:'%s' removed", pdev->dev.of_node->name);

	return 0;
}


#if 0
/* Specifying my resources information */
static struct resource sample_resources[] = {
        {
                .start          = RESOURCE1_START_ADDRESS,
                .end            = RESOURCE1_END_ADDRESS,
                .flags          = IORESOURCE_MEM,
        },
        {
                .start          = RESOURCE2_START_ADDRESS,
                .end            = RESOURCE2_END_ADDRESS,
                .flags          = IORESOURCE_MEM,
        },
    {
                .start          = SAMPLE_DEV_IRQNUM,
                .end            = SAMPLE_DEV_IRQNUM,
                .flags          = IORESOURCE_IRQ,
        }

};    

static struct platform_device sample_device = 
{
        .name           = DRIVER_NAME,
        .id             = -1,
        .num_resources  = ARRAY_SIZE(sample_resources),
        .resource       = sample_resources,
};
#endif

static const struct of_device_id mux_ir_table[] = 
{
	{
		.compatible = "mux_irc",
	},
	{0}
};


MODULE_DEVICE_TABLE(of, mux_ir_table);


static struct platform_driver mix_ir_platform_driver = 
{
	.probe = ir_probe,
	.remove = ir_remove,
	.driver = {
		.name = DRIVER_NAME,// "mux_irc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mux_ir_table),
        },
};


#if 0
module_platform_driver(mix_ir_platform_driver);
#else
static int muxIr_module_init(void)
{
	pr_debug("Hello!\n");
	EXT_INFOF("Hello!");

	/* Registering with Kernel */
	platform_driver_register(&mix_ir_platform_driver);
	return 0;
}

static void muxIr_module_exit(void)
{
	pr_debug("Goodbye!\n");
	EXT_INFOF("Goodbye!");
	/* Unregistering from Kernel */
	platform_driver_unregister(&mix_ir_platform_driver);
}

module_init(muxIr_module_init);
module_exit(muxIr_module_exit);
#endif


