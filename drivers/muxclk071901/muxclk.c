#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/list.h>
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/poll.h> 


#define PIO_IER3		0xFC0380E0	//PIO Interrupt Enable Register
#define PIO_IDR3		0xFC0380E4	//PIO Interrupt Disable Register
#define PIO_IMR3		0xFC0380E8	//PIO Interrupt Mask Register

#define PMC_PCER0	0xF0014010	//PMC Peripheral Clock Enable Register 0
#define PMC_PCER1 	0xF0014100	//PMC Peripheral Clock Enable Register 1
#define PMC_PCDR1	0xF0014104	//PMC Peripheral Clock Disable Register 1
#define PMC_PCSR1	0xF0014108	//PMC Peripheral Clock Status Register 1
#define PWM_WPCR 	0xF802C0E4	//PWM Write Protection Control Register
#define PMC_WPMR	0xF00140E4	//PMC Write Protection Mode Register
#define PMC_PCR		0xF001410C	//PMC Peripheral Control Register
#define PWM_CLK		0xF802C000	//PWM Clock Register
#define PWM_CMR_0	0xF802C200	//PWM Channel Mode Register 0
#define PWM_CMR_1	0xF802C220	//PWM Channel Mode Register 1
#define PWM_CMR_2	0xF802C240	//PWM Channel Mode Register 2
#define PWM_CMR_3	0xF802C260	//PWM Channel Mode Register 3
#define PWM_CDTY_0	0xF802C204	//PWM Channel Duty Cycle Register 0
#define PWM_CDTY_1	0xF802C224	//PWM Channel Duty Cycle Register 1
#define PWM_CDTY_2	0xF802C244	//PWM Channel Duty Cycle Register 2
#define PWM_CDTY_3	0xF802C264	//PWM Channel Duty Cycle Register 3
#define PWM_CPRD_0	0xF802C20C	//PWM Channel Period Register 0
#define PWM_CPRD_1	0xF802C22C	//PWM Channel Period Register 1
#define PWM_CPRD_2	0xF802C24C	//PWM Channel Period Register 2
#define PWM_CPRD_3	0xF802C26C	//PWM Channel Period Register 3
#define PWM_SCM		0xF802C020	//PWM Sync Channels Mode Register
#define PWM_ENA		0xF802C004	//PWM Enable Register
#define PWM_DIS		0xF802C008	//PWM Disable Register

//This driver uses channel 0 of Timer counter 1.

//Timer counter 0 register
#define TC_WPMR_0	0xF800C0E4	//TC Write Protection Mode Register for TC 0
#define TC_CCR_0_0	0xF800C000	//TC Channel Control Register, channel 0 of TC 0
#define TC_CMR_0_0	0xF800C004	//TC Channel Mode Register: Waveform Mode
#define TC_IDR_0_0  0xF800C028	//TC Interrupt Disable Register
#define TC_QIDR_0_0 0xF800C0CC	//TC QDEC Interrupt Disable Register
#define TC_EMR_0_0  0xF800C030	//TC Extended Mode Register
#define	TC_FMR_0 	0xF800C0D8	//TC Fault Mode Register
#define TC_CV_0_0	0xF800C010	//TC Counter Value Register

//Timer counter 1 register
#define TC_WPMR_1	0xF80100E4	//TC Write Protection Mode Register for TC 1
#define TC_CCR_1_0	0xF8010000	//TC Channel Control Register, channel 0 of TC 1
#define TC_CMR_1_0	0xF8010004	//TC Channel Mode Register: Waveform Mode
#define TC_IDR_1_0  0xF8010028	//TC Interrupt Disable Register
#define TC_QIDR_1_0 0xF80100CC	//TC QDEC Interrupt Disable Register
#define TC_EMR_1_0  0xF8010030	//TC Extended Mode Register
#define	TC_FMR_1 	0xF80100D8	//TC Fault Mode Register
#define TC_CV_1_0	0xF8010010	//TC Counter Value Register

// This module only enable SPI0 peripheral clock

static int muxclk_open(struct inode * inode, struct file * file)
{	
	return 0;
}

static int muxclk_close(struct inode * inode, struct file * file)
{
    return 0;
}

static struct file_operations muxclk_fops = 
{
    .owner      = THIS_MODULE,	
    .open       = muxclk_open,	
    .release    = muxclk_close,
};

static struct miscdevice muxclk_dev = 
{   
    .minor		= MISC_DYNAMIC_MINOR,
    .name		= "muxclk",
    .fops  		= &muxclk_fops,
};

static int __init muxclk_init(void)
{		
	void *reg_base;	
	if (misc_register(&muxclk_dev))
    {
        printk(KERN_ERR"ERROR: could not register muxclk devices.\n");
		return -1;
    }
	//Disable write protection in PMC	
	reg_base = ioremap(PMC_WPMR, 4);
	writel_relaxed(0x504D4300, reg_base);
	iounmap(reg_base);	
	
	//Enable UART3 PID27 clock		
	//reg_base = ioremap(PMC_PCER0, 4);
	//writel_relaxed(0x08000000, reg_base);
	//iounmap(reg_base);
	
	//Enable SPI0 PID33 clock 	
	reg_base = ioremap(PMC_PCER1, 4);
	writel_relaxed(0x00000002, reg_base);
	iounmap(reg_base);
	
	//Set write protection in PMC
	reg_base = ioremap(PMC_WPMR, 4);
	writel_relaxed(0x504D4301, reg_base);
	iounmap(reg_base);			
		
    return 0;	
}
static void __exit muxclk_exit(void)
{		
	misc_deregister(&muxclk_dev);	
}

module_init(muxclk_init);
module_exit(muxclk_exit);
MODULE_LICENSE("GPL");

