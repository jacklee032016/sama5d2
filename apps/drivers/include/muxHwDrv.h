
#ifndef	__MUX_HW_DRV_H__
#define	__MUX_HW_DRV_H__

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

#include <linux/uaccess.h> /* copy_to_user */

//SAMA5D2 registers
//Secure PIO Set I/O Non-Secure Register
#define S_PIO_SIONR0	0xFC039030
#define S_PIO_SIONR1	0xFC039070
#define S_PIO_SIONR2	0xFC0390B0
#define S_PIO_SIONR3	0xFC0390F0
//Secure PIO Set I/O Secure Register
#define S_PIO_SIOSR0	0xFC039034
#define S_PIO_SIOSR1	0xFC039074
#define S_PIO_SIOSR2	0xFC0390B4
#define S_PIO_SIOSR3	0xFC0390F4
//PIO Mask Register
#define PIO_MSKR0		0xFC038000
#define PIO_MSKR1		0xFC038040
#define PIO_MSKR2		0xFC038080
#define PIO_MSKR3		0xFC0380C0
//PIO Configuration Register
#define PIO_CFGR0		0xFC038004
#define PIO_CFGR1		0xFC038044
#define PIO_CFGR2		0xFC038084
#define PIO_CFGR3		0xFC0380C4
//PIO Pin Data Status Register
#define PIO_PDSR0		0xFC038008
#define PIO_PDSR1		0xFC038048
#define PIO_PDSR2		0xFC038088
#define PIO_PDSR3		0xFC0380C8
//PIO Set Output Data Register
#define PIO_SODR0		0xFC038010
#define PIO_SODR1		0xFC038050
#define PIO_SODR2		0xFC038090
#define PIO_SODR3		0xFC0380D0
//PIO Clear Output Data Register
#define PIO_CODR0		0xFC038014
#define PIO_CODR1		0xFC038054
#define PIO_CODR2		0xFC038094
#define PIO_CODR3		0xFC0380D4

/*
* hardware definitions
*
* SIP Switch
* DIPSW_01  PA18
* DIPSW_02  PA19
* DIPSW_03  PA20
* DIPSW_04  PA21
*
*
* LED VIDEO: PB0
* LED ACT: PB4, Mux768; PB9, Mux774
* LED POWER: PB5 
* LED LINK: PB7, only Mux768

*/

#endif

