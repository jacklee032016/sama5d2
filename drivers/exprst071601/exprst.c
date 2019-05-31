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
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/list.h>
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>

#define EXP_RST_MAGIC 'P'
#define EXP_RST_HIGH  	_IO(EXP_RST_MAGIC,0)  
#define EXP_RST_LOW 	_IO(EXP_RST_MAGIC,1)


//PC19 chip EP91C1E reset

static int exprst_open(struct inode * inode, struct file * file)
{
	void *reg_base;
	//init registers for PC19 as output, non security,
	//Non-Secure	
	reg_base = ioremap(S_PIO_SIONR2,4);
	writel_relaxed(0x00080000, reg_base);
	iounmap(reg_base);
	//Mask register	
	reg_base = ioremap(PIO_MSKR2,4);
	writel_relaxed(0x00080000, reg_base);
	iounmap(reg_base);
	//CFG register, Pull up enable and output
	reg_base = ioremap(PIO_CFGR2,4);
	writel_relaxed(0x00000300, reg_base);
	iounmap(reg_base);
	return 0;
}

static int exprst_close(struct inode * inode, struct file * file)
{
    return 0;
}

static long exprst_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void *reg_base1;
	void *reg_base2;	
	reg_base1 = ioremap(PIO_SODR2, 4);	
	reg_base2 = ioremap(PIO_CODR2, 4);
	
	switch(cmd)  
    {  
	   case EXP_RST_HIGH:  				   
			writel_relaxed(0x00080000, reg_base1);						
			break;			
	   
       case EXP_RST_LOW:  				
			writel_relaxed(0x00080000, reg_base2);				
			break;							
	   default: return -EINVAL;  			
    }	
	iounmap(reg_base1);
	iounmap(reg_base2);	
	return 0;	
}

static struct file_operations exprst_fops = 
{
    .owner      = THIS_MODULE,
    .open       = exprst_open,
    .release    = exprst_close,
    .unlocked_ioctl = exprst_ioctl,
};

static struct miscdevice exprst_dev = 
{   
    .minor		= MISC_DYNAMIC_MINOR,
    .name		= "exprst",
    .fops  		= &exprst_fops,
};

static int __init exprst_init(void)
{
	if (misc_register(&exprst_dev))
    {
        printk(KERN_ERR"ERROR: could not register exprst devices\n");
		return -1;
    }
    return 0;
}
static void __exit exprst_exit(void)
{
    misc_deregister(&exprst_dev);
}

module_init(exprst_init);
module_exit(exprst_exit);
MODULE_LICENSE("GPL");

