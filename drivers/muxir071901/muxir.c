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

//register in SOC


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





//In IR rx mode, PWM channel 1(PB1 pin) is enabled to generate voltage for IR receiver circuit.
//IR_INPUT--PD23
//IR OUTPUT--PD22

//PD23 IRQ number is pioa32+piob32+pioc32+piod23=119, each IO group is 32.
#define IRQ_PIN_NUMBER	119
#define MAX_SYM_CNT 200

#define IR_MAGIC 'I'
#define IR_SENSOR_MODE		_IO(IR_MAGIC, 0)  
#define IR_EMITTER_MODE		_IO(IR_MAGIC, 1) 
#define IR_IDLE_MODE		_IO(IR_MAGIC, 2) 
#define IR_READ_MODE	  	_IOR(IR_MAGIC, 3, unsigned char *)
#define IR_MAGIC_HEADER		0x0768			//IR raw data header


//symbol[] format : IR_MAGIC_HEADER, carrier_freq, ir_data_length, ir_data_0, ir_data_1,...

//declair wait queue
static DECLARE_WAIT_QUEUE_HEAD(muxir_wq);
// condition to wake up queue
static volatile int muxir_press;
//static unsigned int key_val;
static volatile int muxir_press_start;
struct timer_list muxir_timer;
static unsigned short int symbol_cnt;
static unsigned short int symbol[MAX_SYM_CNT];
static unsigned int current_cnt;
static unsigned char ir_mode;	//0--sensor; 1--emitter; 2--idle mode 
static volatile void *counter_reg;				//counter value register
static volatile void *counter_control;			//counter control register
static unsigned int sensor_irq_flag;	//0--no isr; 1--isr installed
static unsigned short int int_sum;
static unsigned short int carrier_freq;
static DEFINE_SPINLOCK(ir_lock);
static unsigned long spin_flags;

void  muxir_timer_timeout(unsigned long arg)
{
	muxir_press = 1;	
	//clear GPIO interruption???	
    wake_up_interruptible(&muxir_wq);
}

// interruption function, read counter register, wake up queue 
static irqreturn_t muxir_isr(int irq, void *pin)
{   	
	unsigned int cnt_tmp;	
	unsigned int cnt_diff;
    if( muxir_press_start == 0 )
	{		
		muxir_press_start = 1;			
        muxir_timer.expires = jiffies + msecs_to_jiffies(200); 
		muxir_timer.data = 0;
		add_timer(&muxir_timer);		
		//soft reset counter
		writel_relaxed(0x4, counter_control);				
		symbol_cnt = 3;				
		int_sum = 0;	
		carrier_freq = 0;	
		//current_cnt = readl_relaxed(counter_reg);
		current_cnt = 0;
	}
	else
	{		
		cnt_tmp = readl_relaxed(counter_reg);
		if(cnt_tmp < current_cnt)
			printk(KERN_ERR"cnt Error!\n");
		cnt_diff = (cnt_tmp - current_cnt)& 0xFFFF;
		current_cnt = cnt_tmp;

		symbol[symbol_cnt] = cnt_diff;	
		symbol_cnt++;
		if(symbol_cnt == MAX_SYM_CNT)
			symbol_cnt = MAX_SYM_CNT - 1;							
		int_sum++;
	}			
    return IRQ_HANDLED;
}

//configure PD22 as IR_OUT. flag==1, set PD22 as GPIO output, flag==0, set PD22 as GPIO input
static int set_ir_out(int flag)
{
	void *reg_base;
	if(flag == 1)
	{
		//init registers for PD22 as output, non security,
		//PD22 Non-Secure	
		reg_base = ioremap(S_PIO_SIONR3, 4);
		writel_relaxed(0x00400000, reg_base);
		iounmap(reg_base);
		//Mask register	
		reg_base = ioremap(PIO_MSKR3, 4);
		writel_relaxed(0x00400000, reg_base);
		iounmap(reg_base);
		//CFG register
		reg_base = ioremap(PIO_CFGR3,4);
		writel_relaxed(0x00000100, reg_base);
		iounmap(reg_base);
	}
	else
	{
		//init registers for PD22 as input, non security,
		//PD22 Non-Secure	
		reg_base = ioremap(S_PIO_SIONR3, 4);
		writel_relaxed(0x00400000, reg_base);
		iounmap(reg_base);
		//Mask register	
		reg_base = ioremap(PIO_MSKR3, 4);
		writel_relaxed(0x00400000, reg_base);
		iounmap(reg_base);
		//CFG register
		reg_base = ioremap(PIO_CFGR3,4);
		writel_relaxed(0x00000000, reg_base);
		iounmap(reg_base);		
	}
	return 0;
}
//set PWM channel 1 for sensor power supply, flag==1, enable PWM_IR, flag==0, disable PWM_IR
static int set_pwm_power(int flag)
{
	void *reg_base;	
	if(flag == 1)
	{
		//enable PWM_IR signal
		//Disable write protection in PMC
		reg_base = ioremap(PMC_WPMR,4);
		writel_relaxed(0x504D4300, reg_base);
		iounmap(reg_base);
		//Enable the PID38 PWM clock in the Power Management Controller (PMC)	
		reg_base = ioremap(PMC_PCER1,4);		
		writel_relaxed(0x40, reg_base);		
		iounmap(reg_base);		
		//Set write protection in PMC
		reg_base = ioremap(PMC_WPMR,4);
		writel_relaxed(0x504D4301, reg_base);
		iounmap(reg_base);		
	
		//Unlock User Interface by writing the WPCMD field in PWM_WPCR.Enable all the group
		reg_base = ioremap(PWM_WPCR,4);
		writel_relaxed(0x50574DFC, reg_base);
		iounmap(reg_base);
	
		//Configuration of the clock generator (DIVA, PREA, DIVB, PREB in the PWM_CLK register if required).	
		reg_base = ioremap(PWM_CLK, 4);
		//DIVA, PREA=8, 166MHz/256
		writel_relaxed(0x00000801, reg_base);
		iounmap(reg_base);	
	
		//Selection of the clock for each channel (CPRE field in PWM_CMRx)
		reg_base = ioremap(PWM_CMR_1,4);
		//CPRE=0, all other bit is 0
		writel_relaxed(0x0, reg_base);
		iounmap(reg_base);
	
		//Configuration of the waveform alignment for each channel (CALG field in PWM_CMRx)
		//Selection of the counter event selection (if CALG = 1) for each channel (CES field in PWM_CMRx)
		//Configuration of the output waveform polarity for each channel (CPOL bit in PWM_CMRx)
		
		//Configuration of the period for each channel (CPRD in the PWM_CPRDx register). Writing in PWM_CPRDx	
		reg_base = ioremap(PWM_CPRD_1,4);
		//CPRD is 0xF, about 20KHz
		writel_relaxed(0x00000F00, reg_base);
		iounmap(reg_base);
	
		//register is possible while the channel is disabled. After validation of the channel, the user must use
		//PWM_CPRDUPDx register to update PWM_CPRDx as explained below.
	
		//Configuration of the duty-cycle for each channel (CDTY in the PWM_CDTYx register). Writing in 	
		//PWM_CDTYx register is possible while the channel is disabled. After validation of the channel, the user
		//must use PWM_CDTYUPDx register to update PWM_CDTYx as explained below.
		//Defines the waveform duty-cycle. This value must be defined between 0 and CPRD (PWM_CPRDx).
		reg_base = ioremap(PWM_CDTY_1, 4);	
		writel_relaxed(0x00000700, reg_base);
		iounmap(reg_base);
	
		//Configuration of the dead-time generator for each channel (DTH and DTL in PWM_DTx) if enabled (DTE bit
		//in PWM_CMRx). Writing in the PWM_DTx register is possible while the channel is disabled. After validation
		//of the channel, the user must use PWM_DTUPDx register to update PWM_DTx
	
		//Selection of the synchronous channels (SYNCx in the PWM_SCM register)
		reg_base = ioremap(PWM_SCM, 4);	
		//all not synchronous channles
		writel_relaxed(0x0, reg_base);
		iounmap(reg_base);
		
		//Selection of the moment when the WRDY flag and the corresponding DMA Controller transfer request are
		//set (PTRM and PTRCS in the PWM_SCM register)	

		//Configuration of the Update mode (UPDM in PWM_SCM register)
		//Configuration of the update period (UPR in PWM_SCUP register) if needed
	
		//Configuration of the comparisons (PWM_CMPVx and PWM_CMPMx)	
		//Configuration of the event lines (PWM_ELMRx)	
		//Configuration of the fault inputs polarity (FPOL in PWM_FMR)
		//Configuration of the fault protection (FMOD and FFIL in PWM_FMR, PWM_FPV and PWM_FPE1)
		//Enable of the interrupts (writing CHIDx and FCHIDx in PWM_IER1, and writing WRDY, UNRE, CMPMx and CMPUx in PWM_IER2)
	
		//Enable of the PWM channels (writing CHIDx in the PWM_ENA register)	
		reg_base = ioremap(PWM_ENA,4);	
		//enable channel 1
		writel_relaxed(0x02, reg_base);
		iounmap(reg_base);	
	
		//Lock User Interface by writing the WPCMD field in PWM_WPCR.Enable all the group
		reg_base = ioremap(PWM_WPCR,4);
		writel_relaxed(0x50574DFD, reg_base);
		iounmap(reg_base);
	}
	//flag = 0, turn off PWM
	else
	{
		//Unlock User Interface by writing the WPCMD field in PWM_WPCR.Enable all the group
		reg_base = ioremap(PWM_WPCR,4);
		writel_relaxed(0x50574DFC, reg_base);
		iounmap(reg_base);
		
		//Disable of the PWM channels (writing CHIDx in the PWM_DIS register)	
		reg_base = ioremap(PWM_DIS,4);	
		//disable channel 1
		writel_relaxed(0x02, reg_base);
		iounmap(reg_base);	
		
		//Lock User Interface by writing the WPCMD field in PWM_WPCR.Enable all the group
		reg_base = ioremap(PWM_WPCR,4);
		writel_relaxed(0x50574DFD, reg_base);
		iounmap(reg_base);		
	}
	return 0;
}

//init timer counter 1 channel 0
static int tc_1_0_init(void)
{
	void *reg_base;
	unsigned int status;
	status = 0;	
	//TC 1 PMC clock is already enabled in bootstrap.
	
	//Enable TC 0 clk in PMC module	
	//Disable write protection in PMC	
	reg_base = ioremap(PMC_WPMR, 4);
	writel_relaxed(0x504D4300, reg_base);
	iounmap(reg_base);	
	
	//Enable PID36 TC1 clock
	reg_base = ioremap(PMC_PCER1, 4);	
	writel_relaxed(0x10, reg_base);	
	iounmap(reg_base);
	
	//Enable UART3 PID27 clock		
	reg_base = ioremap(PMC_PCER0, 4);
	writel_relaxed(0x08000000, reg_base);
	iounmap(reg_base);
	
	//Set write protection in PMC
	reg_base = ioremap(PMC_WPMR, 4);
	writel_relaxed(0x504D4301, reg_base);
	iounmap(reg_base);			
		
	//P2125 The Timer Counter clock of the first channel must be enabled to access this register.
	//Enable channel 0 of TC 1 clock
	reg_base = ioremap(TC_CCR_1_0, 4);
	writel_relaxed(0x1, reg_base);
	iounmap(reg_base);
	
	//set TC Interrupt Enable Register--disable all the interruption	
	reg_base = ioremap(TC_IDR_1_0, 4);
	writel_relaxed(0xFF, reg_base);
	iounmap(reg_base);
	
	//TC QDEC Interrupt Disable Register
	reg_base = ioremap(TC_QIDR_1_0, 4);
	writel_relaxed(0x07, reg_base);
	iounmap(reg_base);
	
	//Open write protection of TC0
	reg_base = ioremap(TC_WPMR_1, 4);
	writel_relaxed(0x54494D00, reg_base);
	iounmap(reg_base);	
	//The following registers can be write-protected:
	//TC Block Mode Register
	//TC Channel Mode Register: Capture Mode
	//TC Channel Mode Register: Waveform Mode
	//TC Fault Mode Register
	//TC Stepper Motor Mode Register
	//TC Register A
	//TC Register B
	//TC Register C
	//TC Extended Mode Register

	//set TC Channel Mode Register: Waveform Mode channel 0 of TC 0	
	//TCCLKS: 001--TIMER_CLOCK3  Clock selected: internal System bus clock divided by 32 clock signal (from PMC) 83M/32
	//CLKI(clock invert) : 0--Counter is incremented on rising edge of the clock.
	//BURST(burst signal selection): 0 -- NONE  The clock is not gated by an external signal.
	//CPCSTOP: 0--0: Counter clock is not stopped when counter reaches RC.
	//CPCDIS: 0--Counter clock is not disabled when counter reaches RC.
	//EEVTEDG: 0-NONE External Event Edge Selection
	//EEVT: 0--External Event Selection
	//ENETRG: 0--The external event has no effect on the counter and its clock.
	//WAVSEL:0--UP UP mode without automatic trigger on RC Compare
	//WAVE: Waveform Mode 1--Waveform mode is enabled.
	//ACPA: RA Compare Effect on TIOAx 0--NONE
	//ACPC: RC Compare Effect on TIOAx 0--NONE
	//AEEVT: External Event Effect on TIOAx 0--NONE
	//ASWTRG: Software Trigger Effect on TIOAx 0--NONE
	//BCPB: RB Compare Effect on TIOBx 0--NONE
	//BCPC: RC Compare Effect on TIOBx 0--NONE
	//BEEVT: External Event Effect on TIOBx 0--NONE
	//BSWTRG: Software Trigger Effect on TIOBx 0--NONE
	reg_base = ioremap(TC_CMR_1_0, 4);	
	writel_relaxed(0x00008002, reg_base);
	iounmap(reg_base);	
	
	//set TC Extended Mode Register, NODIVCLK-0--The selected clock is defined by field TCCLKS in TC_CMRx.
	reg_base = ioremap(TC_EMR_1_0, 4);	
	writel_relaxed(0x0, reg_base);
	iounmap(reg_base);
	
	//set TC Fault Mode Register of TC1
	reg_base = ioremap(TC_FMR_1, 4);	
	writel_relaxed(0x0, reg_base);
	iounmap(reg_base);
		
	//Enable write protection of TC1
	reg_base = ioremap(TC_WPMR_1, 4);
	writel_relaxed(0x54494D01, reg_base);
	iounmap(reg_base);
	
	writel_relaxed(0x5, counter_control);		
	return 0;
}


static int muxir_open(struct inode * inode, struct file * file)
{
	void *reg_base;
	int irq_num;	
	muxir_press = 0;
	//disable PD23(ir) interrupt
	reg_base = ioremap(PIO_IDR3, 4);
	writel_relaxed(0x00800000, reg_base);	
	iounmap(reg_base);
	//free sensor mode irq
	if(sensor_irq_flag == 1)
	{
		irq_num = gpio_to_irq(IRQ_PIN_NUMBER);	
		free_irq(irq_num, NULL);
		sensor_irq_flag = 0;
	}	
	return 0;
}

static long muxir_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int error, irq_num;	
	void *reg_base;	
	unsigned int status;	
	switch(cmd)  
    {  
	   case IR_SENSOR_MODE:  
			//disable PD23(ir) interrupt
			reg_base = ioremap(PIO_IDR3, 4);
			writel_relaxed(0x00800000, reg_base);
			iounmap(reg_base);			
			ir_mode = 0;
			set_ir_out(0);
			//set normal PWM output to supply power for IR sensor
			//enable PWM channel 1, set PB1 as PWM output clock to IR power supply
			set_pwm_power(1);
			//init registers for PB1 as output, non security,
			reg_base = ioremap(S_PIO_SIONR1,4);
			writel_relaxed(0x00000002, reg_base);
			iounmap(reg_base);
			//Mask register	
			reg_base = ioremap(PIO_MSKR1,4);
			writel_relaxed(0x20000002, reg_base);
			iounmap(reg_base);
			//CFG register, function D, output, PWML1-PB1-function D
			reg_base = ioremap(PIO_CFGR1,4);
			writel_relaxed(0x00000104, reg_base);
			iounmap(reg_base);
			
			muxir_press_start = 0;			
			muxir_press = 0;
			current_cnt = 0;	
			memset(symbol, 0, sizeof(symbol));		
			//init TC 1 channel 0	
			tc_1_0_init();
			
			//IR sensor mode, register interruption ISR			
			if(sensor_irq_flag == 0)	
			{
				irq_num = gpio_to_irq(IRQ_PIN_NUMBER);
				error = request_irq(irq_num, muxir_isr, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "muxir", NULL);
				if(error)
				{
					printk(KERN_ERR"request muxir IRQ error!");
				}
				else
					sensor_irq_flag = 1;				
			}
			//enable PD23(ir) interrupt
			reg_base = ioremap(PIO_IER3,4);
			writel_relaxed(0x00800000, reg_base);
			iounmap(reg_base);		

			
			
			
			printk(KERN_ERR"Emitter mode...Read PMC Peripheral Clock Status Register 0, 1\n");
			reg_base = ioremap(0xF0014018, 4);
			status = readl_relaxed(reg_base);			
			iounmap(reg_base);	
			printk(KERN_ERR"PMC_PCSR0 = 0x%08x\n", status);
			
			reg_base = ioremap(0xF0014108, 4);
			status = readl_relaxed(reg_base);			
			iounmap(reg_base);	
			printk(KERN_ERR"PMC_PCSR1 = 0x%08x\n", status);
			
			break;			
	   
       case IR_EMITTER_MODE:
			ir_mode = 1;
			muxir_press = 0;
			//disable PD23(ir) interrupt
			reg_base = ioremap(PIO_IDR3, 4);
			writel_relaxed(0x00800000, reg_base);	
			iounmap(reg_base);
			//free sensor mode irq
			if(sensor_irq_flag == 1)
			{
				irq_num = gpio_to_irq(IRQ_PIN_NUMBER);
				free_irq(irq_num, NULL);
				sensor_irq_flag = 0;
			}
			//Disable PWM_IR clock output
			set_pwm_power(0);	
			reg_base = ioremap(PIO_CODR3, 4);
			set_ir_out(1);
			//set PD22 to LOW
			writel_relaxed(0x00400000, reg_base);	
			iounmap(reg_base);
			
			
			
			printk(KERN_ERR"Emitter mode...Read PMC Peripheral Clock Status Register 0, 1\n");
			reg_base = ioremap(0xF0014018, 4);
			status = readl_relaxed(reg_base);			
			iounmap(reg_base);	
			printk(KERN_ERR"PMC_PCSR0 = 0x%08x\n", status);
			
			reg_base = ioremap(0xF0014108, 4);
			status = readl_relaxed(reg_base);			
			iounmap(reg_base);	
			printk(KERN_ERR"PMC_PCSR1 = 0x%08x\n", status);
			
			
			
			break;			
			
		case IR_IDLE_MODE:
			ir_mode = 2;
			muxir_press = 0;
			//disable PD23(ir) interrupt
			reg_base = ioremap(PIO_IDR3, 4);
			writel_relaxed(0x00800000, reg_base);	
			iounmap(reg_base);
			//free sensor mode irq
			if(sensor_irq_flag == 1)
			{
				irq_num = gpio_to_irq(IRQ_PIN_NUMBER);
				free_irq(irq_num, NULL);
				sensor_irq_flag = 0;
			}
			//Disable PWM_IR clock output
			set_pwm_power(0);	
			reg_base = ioremap(PIO_CODR3, 4);
			set_ir_out(1);
			//set PD22 to LOW
			writel_relaxed(0x00400000, reg_base);	
			iounmap(reg_base);
			break;			
			
		case IR_READ_MODE:
			status = ir_mode;
			if (copy_to_user((unsigned int *)arg, &status, sizeof(unsigned int)))
			{	
				return -EACCES;
			}										
			break;			
				
		default: return -EINVAL;  			
    }
	return 0;	
}

static int muxir_close(struct inode * inode, struct file * file)
{
    return 0;
}

static ssize_t muxir_read(struct file *file, char __user *buf, size_t count, loff_t *loff)
{    
    int ret;
	int	i;
	/*
	if(!muxir_press)
	{
		if(file->f_flags & O_NONBLOCK)
		{		
			return EAGAIN;
		}
		else					
			wait_event_interruptible(muxir_wq, muxir_press);			
	}
	else
	*/
	wait_event_interruptible(muxir_wq, muxir_press);  	
    muxir_press = 0;
	muxir_press_start = 0;	
	symbol[0] = IR_MAGIC_HEADER;
	symbol[1] = 0; //carrier_freq is read from FPGA, NOT in ir driver
    symbol[2] = symbol_cnt;		
	
	//TC1 channel 0 reference clock is 83MHz/32	
	for(i = 3; i < symbol_cnt; i++)
		symbol[i] = (symbol[i]*32)/83;
	
	ret = copy_to_user(buf, &symbol[0], sizeof(symbol));		
	current_cnt = 0;
	int_sum = 0;	
	memset(symbol, 0, sizeof(symbol));  	
		
	if( ret == 0 )
		return (symbol_cnt + 1);
	else
		return -1;	
}

static ssize_t muxir_write(struct file *file, const char __user *buf, size_t count, loff_t *loff)
{
    int ret, i, j, emitter_cnt;
	void *reg_base1;
	void *reg_base2;	
	
	if(ir_mode != 1)					//Not emitter mode
	{
        printk(KERN_ERR"IR Not emitter mode. Do nothing\n");
		return -EINVAL;
    }
	reg_base1 = ioremap(PIO_SODR3, 4);
	reg_base2 = ioremap(PIO_CODR3, 4);
	set_ir_out(1);
	//set PD22 to LOW
	writel_relaxed(0x00400000, reg_base2);	
	if(count <= MAX_SYM_CNT*2)		//each data is 2 bytes
    {
		memset(symbol, 0, sizeof(symbol));
		ret = copy_from_user(symbol, buf, count);
        if(ret)
        {
            printk(KERN_ERR"copy from user fail\n");
			return -EFAULT;
        }		
		if(symbol[0] != IR_MAGIC_HEADER)		
		{				
			printk(KERN_ERR"IR magic header error! %d \n", symbol[0]);
			return -EINVAL;
		}				
		emitter_cnt = symbol[2];	
		if(emitter_cnt > MAX_SYM_CNT)
		{				
			printk(KERN_ERR"symbol_cnt %d wrong.\n", emitter_cnt);
			return -EINVAL;
		}					
		
		/*
		for(i = 0; i < emitter_cnt; i++)
			printk(KERN_ERR" emitter symbol[%d] = 0x%x \n", i, symbol[i]);		
		*/
		//to protect the ir critical section
		spin_lock_irqsave(&ir_lock, spin_flags);
		for(i = 3; i <= emitter_cnt; i = (i+2))
		{
			//set PD22 to HIGH			
			writel_relaxed(0x00400000, reg_base1);	
			if((symbol[i] & 0xFFFF) > 1000)
			{
				for(j = 0; j < symbol[i]/1000; j++)
					udelay(1000);
				udelay(symbol[i]%1000);				
			}
			else
				udelay(symbol[i]);			
			//set PD22 to LOW
			writel_relaxed(0x00400000, reg_base2);	
			if((symbol[i+1] & 0xFFFF) > 1000)
			{
				for(j = 0; j < symbol[i+1]/1000; j++)
					udelay(1000);
				udelay(symbol[i+1]%1000);				
			}
			else
				udelay(symbol[i+1]);
		}			
		spin_unlock_irqrestore(&ir_lock, spin_flags);				
    }
	else
	{
        printk(KERN_ERR"size must be less than %d\n", MAX_SYM_CNT);
		return -EINVAL;
    }
	//set PD22 to LOW
	writel_relaxed(0x00400000, reg_base2);
	iounmap(reg_base1);
	iounmap(reg_base2);
    return count;
}

static unsigned int muxir_poll(struct file *file, poll_table* wait)
{
	unsigned int mask = 0;	
	poll_wait(file, &muxir_wq, wait);
	if(muxir_press)
	{
		mask |= POLLIN | POLLRDNORM; 	//data is available		
	}
	return mask;
}

static struct file_operations muxir_fops = 
{
    .owner      = THIS_MODULE,
	.read		= muxir_read,
	.write 		= muxir_write,
    .open       = muxir_open,
	.poll 		= muxir_poll,
	.unlocked_ioctl = muxir_ioctl,
    .release    = muxir_close,
};

static struct miscdevice muxir_dev = 
{   
    .minor		= MISC_DYNAMIC_MINOR,
    .name		= "muxir",
    .fops  		= &muxir_fops,
};

static int __init muxir_init(void)
{		
	void *reg_base;	
	if (misc_register(&muxir_dev))
    {
        printk(KERN_ERR"ERROR: could not register muxir devices.\n");
		return -1;
    }	
	//init timer
	init_timer(&muxir_timer);	
	muxir_timer.function = muxir_timer_timeout;	
	ir_mode = 2; //init to idle mode
	muxir_press = 0;
	sensor_irq_flag = 0;
	counter_reg = ioremap(TC_CV_1_0, 4);	
	counter_control = ioremap(TC_CCR_1_0, 4);
	//disable PD23(ir) interrupt
	reg_base = ioremap(PIO_IDR3, 4);
	writel_relaxed(0x00800000, reg_base);
	iounmap(reg_base);

	
	unsigned int status;
	
	
	
	
	printk(KERN_ERR"Emitter mode...Read PMC Peripheral Clock Status Register 0, 1\n");
	reg_base = ioremap(0xF0014018, 4);
	status = readl_relaxed(reg_base);			
	iounmap(reg_base);	
	printk(KERN_ERR"PMC_PCSR0 = 0x%08x\n", status);
			
	reg_base = ioremap(0xF0014108, 4);
	status = readl_relaxed(reg_base);			
	iounmap(reg_base);	
	printk(KERN_ERR"PMC_PCSR1 = 0x%08x\n", status);
	
	
	printk(KERN_ERR"muxir 071902\n");
	
    return 0;	
}
static void __exit muxir_exit(void)
{	
	void *reg_base;	
	int irq_num;
	//disable PD23(ir) interrupt
	reg_base = ioremap(PIO_IDR3, 4);
	writel_relaxed(0x00800000, reg_base);		
	iounmap(reg_base);
	irq_num = gpio_to_irq(IRQ_PIN_NUMBER);	
	if(sensor_irq_flag)
		free_irq(irq_num, NULL);
	iounmap(counter_reg);
	iounmap(counter_control);
	del_timer(&muxir_timer);
	misc_deregister(&muxir_dev);	
}

module_init(muxir_init);
module_exit(muxir_exit);
MODULE_LICENSE("GPL");

