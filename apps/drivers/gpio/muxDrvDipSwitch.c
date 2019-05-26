
#include "muxHwDrv.h"
#include "muxDrv.h"

#include "mux7xxCompact.h"

static volatile void *Reg_PIO_PDSR0;


static int switch_open(struct inode * inode, struct file * file)
{
	void *reg_base;
	//init registers for PA18,PA19,PA20,PA21 as input, non security,	
	reg_base = ioremap(S_PIO_SIONR0, 4);
	writel_relaxed(0x003C0000, reg_base);
	iounmap(reg_base);
	
	//Mask register	
	reg_base = ioremap(PIO_MSKR0,4);
	writel_relaxed(0x003C0000, reg_base);
	iounmap(reg_base);

	//CFG register
	reg_base = ioremap(PIO_CFGR0,4);
	writel_relaxed(0x00000000, reg_base);
	iounmap(reg_base);	
	return 0;
}

static int switch_close(struct inode * inode, struct file * file)
{
	return 0;
}

static long switch_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{    
	unsigned int status;	
	switch(cmd)  
	{  
		case MUX_DRV_DIP_SWITCH_READ:  						
			status = ( readl_relaxed(Reg_PIO_PDSR0) & 0x003C0000) >> 18;				
			if (copy_to_user((unsigned int *)arg, &status, sizeof(unsigned int)))
			{	
				return -EACCES;
			}				
			break;		 				

		default:
			return -EINVAL;  					
	} 	
	return 0;	
}

static struct file_operations switch_fops = 
{
    .owner      = THIS_MODULE,
    .open       = switch_open,
    .release    = switch_close,
    .unlocked_ioctl = switch_ioctl,
};

static struct miscdevice switch_dev = 
{   
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= MUX_DRV_DIP_SWITCH_NAME,
	.fops	= &switch_fops,
};


static int _ledVideoOpen(struct inode * inode, struct file * file)
{
	void *reg_base;
	
	//init registers for PB7 as output, non security,
	//PB7 Non-Secure	
	reg_base = ioremap(S_PIO_SIONR1, 4);
	writel_relaxed(0x01, reg_base);
	iounmap(reg_base);
	
	//Mask register	
	reg_base = ioremap(PIO_MSKR1, 4);
	writel_relaxed(0x01, reg_base);
	iounmap(reg_base);
	
	//CFG register
	reg_base = ioremap(PIO_CFGR1, 4);
	writel_relaxed(0x00000100, reg_base);	
	iounmap(reg_base);

	return 0;
}

static int _ledVideoClose(struct inode * inode, struct file * file)
{
	return 0;
}

static long _ledVideoIoctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	switch(cmd)  
	{  
		case MUX_DRV_LED_VIDEO_OP_ON: 
			writel_relaxed(0x01, Reg_PIO_SODR1);			
			break;

		case MUX_DRV_LED_VIDEO_OP_OFF:  	
			writel_relaxed(0x01, Reg_PIO_CODR1);	
			break;
			
		default:
			return -EINVAL;  			
	}
	return 0;	
}

static struct file_operations _ledVideoFops = 
{
	.owner	= THIS_MODULE,
	.open	= _ledVideoOpen,
	.release	= _ledVideoClose,
	.unlocked_ioctl = _ledVideoIoctl,
};

static struct miscdevice videoled_dev = 
{   
    .minor		= MISC_DYNAMIC_MINOR,
    .name		= MUX_DRV_LED_VIDEO_NAME,
    .fops  		= &_ledVideoFops,
};

static int __init _ledVideoInit(void)
{	
	if (misc_register(&videoled_dev))
	{
		printk(KERN_ERR"ERROR: could not register videoled devices\n");
		return -1;
	}
	
	Reg_PIO_SODR1 = ioremap(PIO_SODR1, 4);
	Reg_PIO_CODR1 = ioremap(PIO_CODR1, 4);		
	return 0;
}
static void __exit _ledVideoExit(void)
{
	iounmap(Reg_PIO_SODR1);
	iounmap(Reg_PIO_CODR1);   
	misc_deregister(&videoled_dev);
}


static int __init switch_init(void)
{
	if (misc_register(&switch_dev))
	{
		printk(KERN_ERR"ERROR: could not register muxswitch devices\n");
		return -1;
	}
	
	Reg_PIO_PDSR0 = ioremap(PIO_PDSR0, 4);

	return _ledVideoInit();
}

static void __exit switch_exit(void)
{
	_ledVideoExit();
	
	iounmap(Reg_PIO_PDSR0);
	misc_deregister(&switch_dev);
}


module_init(switch_init);
module_exit(switch_exit);
MODULE_LICENSE("GPL");

