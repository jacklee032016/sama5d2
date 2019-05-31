/*
 *	w1_ds28e15.c - w1 family 1C (DS28E15) driver
 *
 * Copyright (c) 2012 Markus Franke <franke.m@sebakmt.com>
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/crc16.h>
#include <linux/uaccess.h>

/*
* Hw info:
* 512 bits, 2 pages, 256 bit/page, 32 bytes
* user memory: 64 byte, RW
* secret memory: 32 bytes, write, compute
* scratch pad, SRAM: 32 bytes, intermidiate storage for challenge and secret
* status memory: 8 bytes, read, write
* ROM ID: 8 bytes
*/

/*
* Address mapping design
*  Memory       read               Write
*   Page#0      data                data            : EEPROM read/write for the first page 
*   page#1      personality        key             : scratch and lock
*   page#2      MAC                challenge      : after write MAC, it can be read
*/


#define CRC16_INIT		0
#define CRC16_VALID		0xb001

#include <linux/w1.h>

#include "mux7xxCompact.h"

#define W1_FAMILY_DS28E15			0x17


/* Allow the strong pullup to be disabled, but default to enabled.
 * If it was disabled a parasite powered device might not get the required
 * current to copy the data from the scratchpad to EEPROM.  If it is enabled
 * parasite powered devices have a better chance of getting the current
 * required.
 */
static int w1_e15_strong_pullup = 1;//1;
module_param_named(strong_pullup, w1_e15_strong_pullup, int, 0);

/* enable/disable CRC checking on DS28E15 memory accesses */
static char w1_e15_enable_crccheck = 0;

/* EEPROM size is 64 bytes, but driver limits its first 32 bytes, eg page#0 */
#if 0
#define W1_E15_EEPROM_SIZE		64
#else
#define W1_E15_EEPROM_SIZE		32
#endif
#define W1_E15_PAGE_COUNT			2
#define W1_E15_PAGE_SIZE			32
#define W1_E15_PAGE_BITS			1
#define W1_E15_PAGE_MASK			0x1

#define W1_E15_READ_EEPROM		0xF0
#define W1_E15_WRITE_SCRATCH		0x0F
#define W1_E15_READ_SCRATCH		0xAA
#define W1_E15_READ_STATUS		0xAA	/* E15 */
#define W1_E15_COPY_SCRATCH		0x55
#define W1_E15_WRITE_EEPROM		0x55	/* E15 */
#define W1_E15_ACCESS_WRITE		0x5A

#define W1_E15_LOCK_SECRET		0x33
#define W1_E15_READ_MAC			0xA5	/* E15 */


#define W1_E15_RELEASE				0xAA


#define W1_E15_REG_LOGIC_STATE		0x220

// delay durations
#define EEPROM_WRITE_DELAY					10  	/* p#3, tPRD unit:ms */
#define SHA_COMPUTATION_DELAY					3   	/* p#3, tCSHA, unit:ms */
#define SECRET_PROGRAM_DELAY					100	/* p#3, tPRS, Program for secrect  unit:ms */


struct w1_e15_data
{
	u8	memory[W1_E15_EEPROM_SIZE];
	u32	validcrc;
};

/**
 * Check the file size bounds and adjusts count as needed.
 * This would not be needed if the file size didn't reset to 0 after a write.
 */
static inline size_t w1_e15_fix_count(loff_t off, size_t count, size_t size)
{
	if (off > size)
		return 0;

	if ((off + count) > size)
		return size - off;

	return count;
}

static int w1_e15_refresh_block(struct w1_slave *sl, struct w1_e15_data *data, int block)
{
	u8	wrbuf[2];
	int	off = block * W1_E15_PAGE_SIZE;

	if (data->validcrc & (1 << block))
		return 0;

	if (w1_reset_select_slave(sl))
	{
		data->validcrc = 0;
		return -EIO;
	}

	wrbuf[0] = W1_E15_READ_EEPROM;
	wrbuf[1] = 0; /* from page0, seg0 to end of page */
	w1_write_block(sl->master, wrbuf, sizeof(wrbuf));
	
	w1_read_block(sl->master, &data->memory[off], W1_E15_PAGE_SIZE);

	/* cache the block if the CRC is valid */
	if (crc16(CRC16_INIT, &data->memory[off], W1_E15_PAGE_SIZE) == CRC16_VALID)
		data->validcrc |= (1 << block);

	return 0;
}

static int w1_e15_read(struct w1_slave *sl, int addr, int len, char *data)
{
	u8 wrbuf[2];

	/* read directly from the EEPROM */
	if (w1_reset_select_slave(sl)) /* SKIP_ROM command will be sent if slave count is 1 */
	{
		EXT_ERRORF("Reset failed");
		return -EIO;
	}

	wrbuf[0] = W1_E15_READ_EEPROM;
	wrbuf[1] = 0; /* read PB, from seg0, page 0, to end of page */

	w1_write_block(sl->master, wrbuf, sizeof(wrbuf));

	w1_read_block(sl->master, wrbuf, 2);/* read CRC of command. page.38 */

	return w1_read_block(sl->master, data, len);
}

static ssize_t page0_read(struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, 
	char *buf, loff_t off, size_t count)
{
	struct w1_slave *sl = kobj_to_w1_slave(kobj);
	struct w1_e15_data *data = sl->family_data;
	int i, min_page, max_page;

	EXT_INFOF("off:%lld; count:%d", off, count);
	count = w1_e15_fix_count(off, count, W1_E15_EEPROM_SIZE);
	if (count == 0)
	{
		EXT_ERRORF("Params error:off:%lld; count:%d", off, count);
		return 0;
	}
	EXT_INFOF("After fixed, count:%d", count);

	mutex_lock(&sl->master->mutex);

	if (w1_e15_enable_crccheck)
	{
		min_page = (off >> W1_E15_PAGE_BITS);
		max_page = (off + count - 1) >> W1_E15_PAGE_BITS;
		
		for (i = min_page; i <= max_page; i++)
		{
			if (w1_e15_refresh_block(sl, data, i))
			{
				count = -EIO;
				goto out_up;
			}
		}
		
		memcpy(buf, &data->memory[off], count);
	}
	else
	{
		count = w1_e15_read(sl, off, count, buf);
	}

out_up:
	mutex_unlock(&sl->master->mutex);

	EXT_INFOF("Read count:%d bytes", count);
	return count;
}

static int w1_e15_write_page(struct w1_slave *sl, int addr, int len, const u8 *data)
{
	u8 wrbuf[4];
	u8 rdbuf[W1_E15_PAGE_SIZE + 3];
	u8 es = (addr + len - 1) & 0x1f;
	int i;
	struct w1_e15_data *f1C = sl->family_data;
	int ret = 0;

	EXT_INFOF("write, addr:%d; len:%d", addr, len);
	wrbuf[0] = W1_E15_WRITE_EEPROM;
	wrbuf[1] = 0; /* from seg0, page0 to end of page */

	/* Write Command and param byte */
	if (w1_reset_select_slave(sl))
	{
		TRACE();
		return -EIO;
	}

	/* write command */
	w1_write_block(sl->master, wrbuf, 2);
	/* read CRC of command */	
	w1_read_block(sl->master, rdbuf, 2);

	/* Compare what was read against the data written */
#if 0	
	if ((rdbuf[0] != wrbuf[1]) || (rdbuf[1] != wrbuf[2]) || (rdbuf[2] != es) || (memcmp(data, &rdbuf[3], len) != 0))
	{
		TRACE();
		return -1;
	}
#endif

	for(i=0; i<8; i++)
	{
		w1_write_block(sl->master, data+i*4, 4);

		w1_read_block(sl->master, rdbuf, 2);

		wrbuf[0] = W1_E15_RELEASE;
		w1_write_block(sl->master, wrbuf, 1);

		/* Wait is mandidate requirement for writing of every segment. JL. 05.31, 2019*/
		if (!w1_e15_strong_pullup)
			msleep(EEPROM_WRITE_DELAY - 8); /*max is 10 */

		/* sometime it is read as 0xAB: delay too longer, so read the following 0xff */
		w1_read_block(sl->master, rdbuf, 1);/* read CS */
		if(rdbuf[0] != 0xAA)
		{
			EXT_ERRORF("Failed in write seg#%d: 0x%x", i, rdbuf[0]);
			ret = -1;
			break;
		}
	}

#if 0
	wrbuf[0] = W1_E15_COPY_SCRATCH;
	wrbuf[3] = es;

	for (i = 0; i < sizeof(wrbuf); ++i)
	{
		/* issue 10ms strong pullup (or delay) on the last byte
		   for writing the data from the scratchpad to EEPROM */
		if (w1_e15_strong_pullup && i == sizeof(wrbuf)-1)
			w1_next_pullup(sl->master, tm);

		w1_write_8(sl->master, wrbuf[i]);
	}

	if (!w1_e15_strong_pullup)
		msleep(tm);

	if (w1_e15_enable_crccheck)
	{
		/* invalidate cached data */
		f1C->validcrc &= ~(1 << (addr >> W1_E15_PAGE_BITS));
	}
#endif

	/* Reset the bus to wake up the EEPROM (this may not be needed) */
//	w1_reset_bus(sl->master);

	TRACE();
	return ret;
}


#if 0
/**
 * Writes to the scratchpad and reads it back for verification.
 * Then copies the scratchpad to EEPROM.
 * The data must be on one page.
 * The master must be locked.
 *
 * @param sl	The slave structure
 * @param addr	Address for the write
 * @param len   length must be <= (W1_PAGE_SIZE - (addr & W1_PAGE_MASK))
 * @param data	The data to write
 * @return	0=Success -1=failure
 */
static int w1_e15_write(struct w1_slave *sl, int addr, int len, const u8 *data)
{
	u8 wrbuf[4];
	u8 rdbuf[W1_E15_PAGE_SIZE + 3];
	u8 es = (addr + len - 1) & 0x1f;
	unsigned int tm = 10;
	int i;
	struct w1_e15_data *f1C = sl->family_data;

	/* Write the data to the scratchpad */
	if (w1_reset_select_slave(sl))
		return -1;

	wrbuf[0] = W1_E15_WRITE_SCRATCH;
	wrbuf[1] = addr & 0xff;
	wrbuf[2] = addr >> 8;

	w1_write_block(sl->master, wrbuf, 3);
	w1_write_block(sl->master, data, len);

	/* Read the scratchpad and verify */
	if (w1_reset_select_slave(sl))
		return -1;

	w1_write_8(sl->master, W1_E15_READ_SCRATCH);
	w1_read_block(sl->master, rdbuf, len + 3);

	/* Compare what was read against the data written */
	if ((rdbuf[0] != wrbuf[1]) || (rdbuf[1] != wrbuf[2]) || (rdbuf[2] != es) || (memcmp(data, &rdbuf[3], len) != 0))
		return -1;

	/* Copy the scratchpad to EEPROM */
	if (w1_reset_select_slave(sl))
		return -1;

	wrbuf[0] = W1_E15_COPY_SCRATCH;
	wrbuf[3] = es;

	for (i = 0; i < sizeof(wrbuf); ++i)
	{
		/* issue 10ms strong pullup (or delay) on the last byte
		   for writing the data from the scratchpad to EEPROM */
		if (w1_e15_strong_pullup && i == sizeof(wrbuf)-1)
			w1_next_pullup(sl->master, tm);

		w1_write_8(sl->master, wrbuf[i]);
	}

	if (!w1_e15_strong_pullup)
		msleep(tm);

	if (w1_e15_enable_crccheck)
	{
		/* invalidate cached data */
		f1C->validcrc &= ~(1 << (addr >> W1_E15_PAGE_BITS));
	}

	/* Reset the bus to wake up the EEPROM (this may not be needed) */
	w1_reset_bus(sl->master);

	return 0;
}
#endif

static ssize_t page0_write(struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, 
	char *buf,  loff_t off, size_t count)

{
	struct w1_slave *sl = kobj_to_w1_slave(kobj);
	int addr, len, idx;
	u16 crc = CRC16_INIT;

	EXT_INFOF("off:%lld; count:%d", off, count);
	count = w1_e15_fix_count(off, count, W1_E15_EEPROM_SIZE);
	if (count == 0)
	{
		EXT_ERRORF("Params error:off:%lld; count:%d", off, count);
		return 0;
	}


	if (w1_e15_enable_crccheck)
	{
		/* can only write full blocks in cached mode */
		if ((off & W1_E15_PAGE_MASK) || (count & W1_E15_PAGE_MASK))
		{
			dev_err(&sl->dev, "invalid offset/count off=%d cnt=%zd\n", (int)off, count);
			return -EINVAL;
		}

		/* make sure the block CRCs are valid */
		for (idx = 0; idx < count; idx += W1_E15_PAGE_SIZE)
		{
			crc = crc16(crc, &buf[idx], W1_E15_PAGE_SIZE);
			if ( crc != CRC16_VALID)
			{
				dev_err(&sl->dev, "bad CRC at offset %d, idx:%d: 0x%x\n", (int)off, idx, crc );
				return -EINVAL;
			}
		}
	}

	mutex_lock(&sl->master->mutex);

	/* Can only write data to one page at a time */
	idx = 0;
	while (idx < count)
	{
		addr = off + idx;
		len = W1_E15_PAGE_SIZE - (addr & W1_E15_PAGE_MASK);
		if (len > (count - idx))
			len = count - idx;

		/* addr is the parameter byte for page write */
		if (w1_e15_write_page(sl, addr, len, &buf[idx]) < 0)
		{
			count = -EIO;
			goto out_up;
		}
		
		idx += len;
	}

out_up:
	mutex_unlock(&sl->master->mutex);

	return count;
}

//static BIN_ATTR_RW(page0, W1_E15_EEPROM_SIZE);
static BIN_ATTR_RW(page0, W1_E15_PAGE_SIZE);

#define	_E15_TEST_READ_SCRATCH_PAD		0

#if _E15_TEST_READ_SCRATCH_PAD
/* scratchpad will be 0 after data in it has been loaded */
static int w1_e15_read_scratchpad(struct w1_slave *sl, int addr, int len, const u8 *data)
{
	u8 wrbuf[4];
	u8 rdbuf[W1_E15_PAGE_SIZE + 3];
	u8 es = (addr + len - 1) & 0x1f;
	struct w1_e15_data *f1C = sl->family_data;

	EXT_INFOF("Read ScratchPas, addr:%d; len:%d", addr, len);
	wrbuf[0] = W1_E15_WRITE_SCRATCH;
	wrbuf[1] = 0x0F; /* read from scratch pad */

	/* Write Command and param byte */
	if (w1_reset_select_slave(sl))
	{
		TRACE();
		return -EIO;
	}

	/* write command */
	w1_write_block(sl->master, wrbuf, 2);
	/* read CRC of command */	
	w1_read_block(sl->master, rdbuf, 2);

	/* Compare what was read against the data written */
#if 0	
	if ((rdbuf[0] != wrbuf[1]) || (rdbuf[1] != wrbuf[2]) || (rdbuf[2] != es) || (memcmp(data, &rdbuf[3], len) != 0))
	{
		TRACE();
		return -1;
	}
#endif

	/* read 32 byte data */
	w1_read_block(sl->master, data, len);
	/* read CRC of data */
	w1_read_block(sl->master, rdbuf, 2);

	/* Reset the bus to wake up the EEPROM (this may not be needed) */
	w1_reset_bus(sl->master);

	return 0;
}
#endif

/* read personality and status of chip */
static ssize_t key_read(struct file *filp, struct kobject *kobj,
			struct bin_attribute *bin_attr, char *buf, loff_t off,
			size_t count)
{
	struct w1_slave *sl = kobj_to_w1_slave(kobj);
	int ret;
	u8 wrbuf[2];

	EXT_INFOF("off:%lld; count:%d", off, count);
	/* check arguments */
	if (off != 0 || count != W1_E15_PAGE_SIZE || buf == NULL)
	{
		EXT_ERRORF("Params error:off:%lld; count:%d", off, count);
		return -EINVAL;
	}	
	
	count = w1_e15_fix_count(off, count, W1_E15_EEPROM_SIZE);
	if (count == 0)
	{
		EXT_INFOF("Fix count error:%d", count);
		return 0;
	}

	EXT_INFOF("Read KEY: After fixed, count:%d", count);
#if _E15_TEST_READ_SCRATCH_PAD
#else
	wrbuf[0] = W1_E15_READ_STATUS;
	wrbuf[1] = 0xE0; /* parameter bytes, 0xE0: read personality */

	memset(buf, 0, count);
#endif

	mutex_lock(&sl->master->mutex);

#if _E15_TEST_READ_SCRATCH_PAD
	w1_e15_read_scratchpad(sl, 0, count, buf);
#else
	/* read directly from the EEPROM */
	if (w1_reset_select_slave(sl)) /* function command of SJIP_ROM in one-wire has been sent be this function. spec.32 */
	{
		EXT_ERRORF("Reset slave failed");
		return -EIO;
	}

	w1_write_block(sl->master, wrbuf, sizeof(wrbuf));
	w1_read_block(sl->master, wrbuf, 2);	/* read CRC */
	
//	w1_read_block(sl->master, buf, 4);		/* 4 bytes personality */
	w1_read_block(sl->master, buf, count);		/* tests */
#endif
	mutex_unlock(&sl->master->mutex);

	return count;
}


static int w1_e15_write_scratchpad(struct w1_slave *sl, int addr, int len, const u8 *data)
{
	u8 wrbuf[4];
	u8 rdbuf[W1_E15_PAGE_SIZE + 3];
	u8 es = (addr + len - 1) & 0x1f;

	struct w1_e15_data *f1C = sl->family_data;

	EXT_INFOF("write, addr:%d; len:%d", addr, len);
	wrbuf[0] = W1_E15_WRITE_SCRATCH;
	wrbuf[1] = 0; /* 0: write scratch pad */

	/* Write Command and param byte */
	if (w1_reset_select_slave(sl))
	{
		TRACE();
		return -EIO;
	}

	/* write command */
	w1_write_block(sl->master, wrbuf, 2);
	/* read CRC of command */	
	w1_read_block(sl->master, rdbuf, 2);

	/* Compare what was read against the data written */
#if 0	
	if ((rdbuf[0] != wrbuf[1]) || (rdbuf[1] != wrbuf[2]) || (rdbuf[2] != es) || (memcmp(data, &rdbuf[3], len) != 0))
	{
		TRACE();
		return -1;
	}
#endif

	/* write 32 byte data */
	w1_write_block(sl->master, data, len);
	/* read CRC of data */
	w1_read_block(sl->master, rdbuf, 2);

	/* Reset the bus to wake up the EEPROM (this may not be needed) */
//	w1_reset_bus(sl->master);

	return 0;
}


static int w1_e15_write_lock_key(struct w1_slave *sl, int addr, int len, const u8 *data)
{
	u8 wrbuf[4];
	u8 rdbuf[W1_E15_PAGE_SIZE + 3];
	u8 es = (addr + len - 1) & 0x1f;

	int i;
	struct w1_e15_data *f1C = sl->family_data;
	int ret = 0;

	if(w1_e15_write_scratchpad(sl, addr, len, data) <0)
	{
		return -EIO;
	}


	/********** step 2:  lock scratch *************/
	wrbuf[0] = W1_E15_LOCK_SECRET;
#if 0
	wrbuf[1] = 0xE0; /* load scratchpad into secret zone with lock: can't modify again  */
#else
	wrbuf[1] = 0; /* load without lock */
#endif

	/* Write Command and param byte */
	if (w1_reset_select_slave(sl))
	{
		TRACE();
		return -EIO;
	}

	/* write command */
	w1_write_block(sl->master, wrbuf, 2);
	/* read CRC of command */	
	w1_read_block(sl->master, rdbuf, 2);

	/* Write RELEASE */
	wrbuf[0] = W1_E15_RELEASE;
	w1_write_block(sl->master, wrbuf, 1);

	/* from tPRS to tPRS, errors(0xAB) decrease a lot */
//	if (!w1_e15_strong_pullup)
//		msleep(SECRET_PROGRAM_DELAY/2 ); /* tPRS */

	msleep(SECRET_PROGRAM_DELAY-70 ); /* tPRS */
	
	w1_read_block(sl->master, rdbuf, 1);/* read CS */
	if(rdbuf[0] == 0xAA)
	{
		ret = 0;
	}
	else if(rdbuf[0] == 0x55)
	{
		EXT_ERRORF("Key has been locked before");
		ret = -1;
	}
	else
	{
		EXT_ERRORF("Status error when write key: 0x%x", rdbuf[0]);
		ret = -1;
	}

#if 0
	wrbuf[0] = W1_E15_COPY_SCRATCH;
	wrbuf[3] = es;

	for (i = 0; i < sizeof(wrbuf); ++i)
	{
		/* issue 10ms strong pullup (or delay) on the last byte
		   for writing the data from the scratchpad to EEPROM */
		if (w1_e15_strong_pullup && i == sizeof(wrbuf)-1)
			w1_next_pullup(sl->master, tm);

		w1_write_8(sl->master, wrbuf[i]);
	}

	if (!w1_e15_strong_pullup)
		msleep(tm);

	if (w1_e15_enable_crccheck)
	{
		/* invalidate cached data */
		f1C->validcrc &= ~(1 << (addr >> W1_E15_PAGE_BITS));
	}
#endif

	/* Reset the bus to wake up the EEPROM (this may not be needed) */
//	w1_reset_bus(sl->master);

	TRACE();
	return ret;
}


static ssize_t key_write(struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, 
	char *buf,  loff_t off, size_t count)

{
	struct w1_slave *sl = kobj_to_w1_slave(kobj);
	int addr, len, idx;
	u16 crc = CRC16_INIT;

	EXT_INFOF("off:%lld; count:%d", off, count);
	count = w1_e15_fix_count(off, count, W1_E15_EEPROM_SIZE);
	if (count == 0)
	{
		EXT_ERRORF("Params error:off:%lld; count:%d", off, count);
		return 0;
	}


	if (w1_e15_enable_crccheck)
	{
		/* can only write full blocks in cached mode */
		if ((off & W1_E15_PAGE_MASK) || (count & W1_E15_PAGE_MASK))
		{
			dev_err(&sl->dev, "invalid offset/count off=%d cnt=%zd\n", (int)off, count);
			return -EINVAL;
		}

		/* make sure the block CRCs are valid */
		for (idx = 0; idx < count; idx += W1_E15_PAGE_SIZE)
		{
			crc = crc16(crc, &buf[idx], W1_E15_PAGE_SIZE);
			if ( crc != CRC16_VALID)
			{
				dev_err(&sl->dev, "bad CRC at offset %d, idx:%d: 0x%x\n", (int)off, idx, crc );
				return -EINVAL;
			}
		}
	}

	mutex_lock(&sl->master->mutex);

	/* Can only write data to one page at a time */
	idx = 0;
	while (idx < count)
	{
		addr = off + idx;
		len = W1_E15_PAGE_SIZE - (addr & W1_E15_PAGE_MASK);
		if (len > (count - idx))
			len = count - idx;

		/* addr is the parameter byte for page write */
		if (w1_e15_write_lock_key(sl, addr, len, &buf[idx]) < 0)
		{
			count = -EIO;
			goto out_up;
		}
		
		idx += len;
	}

out_up:
	mutex_unlock(&sl->master->mutex);

	return count;
}

static BIN_ATTR_RW(key, W1_E15_PAGE_SIZE);

/* read MAC after challenge has been sent */
static ssize_t MAC_read(struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, 
	char *buf, loff_t off, size_t count)
{
	struct w1_slave *sl = kobj_to_w1_slave(kobj);
	int ret;
	u8 wrbuf[2];

	EXT_INFOF("off:%lld; count:%d", off, count);
	/* check arguments */
	if (off != 0 || count != W1_E15_PAGE_SIZE || buf == NULL)
	{
		EXT_ERRORF("Params error:off:%lld; count:%d", off, count);
		return -EINVAL;
	}	
	
	count = w1_e15_fix_count(off, count, W1_E15_EEPROM_SIZE);
	if (count == 0)
	{
		EXT_INFOF("Fix count error:%d", count);
		return 0;
	}

	EXT_INFOF("After fixed, count:%d", count);
	mutex_lock(&sl->master->mutex);

	/* read directly from the EEPROM */
	if (w1_reset_select_slave(sl)) /* function command of SJIP_ROM in one-wire has been sent be this function. spec.32 */
	{
		EXT_ERRORF("Reset slave failed");
		return -EIO;
	}

	wrbuf[0] = W1_E15_READ_MAC;
	/* page#0 always is 0 */
#if 0	
	wrbuf[1] = 0xE0; /* ananymous */
#else
	wrbuf[1] = 0x00; /* Use ROM ID to caculate MAC */
#endif

	memset(buf, 0, count);

	w1_write_block(sl->master, wrbuf, sizeof(wrbuf));
	w1_read_block(sl->master, wrbuf, 2);	/* read CRC */

//	if (!w1_e15_strong_pullup) /* delay */
//		msleep(SHA_COMPUTATION_DELAY*2 -2);/* waiting 2*tCSHA */

	msleep(SHA_COMPUTATION_DELAY-2);/* waiting 2*tCSHA */

	w1_read_block(sl->master, wrbuf, 1);/* read CS */
	if(wrbuf[0] != 0xAA)
	{
		EXT_ERRORF("Failed in status when read MAC: 0x%x", wrbuf[0]);
		count = -EIO;
		goto _exit;
	}
	
	w1_read_block(sl->master, buf, count);	/* 32 bytes MAC */
	w1_read_block(sl->master, wrbuf, 2);	/* read CRC */

_exit:	
	mutex_unlock(&sl->master->mutex);

	return count;
}

/* write challenge and caculate MAC */
static ssize_t MAC_write(struct file *filp, struct kobject *kobj, struct bin_attribute *bin_attr, 
	char *buf,  loff_t off, size_t count)

{
	struct w1_slave *sl = kobj_to_w1_slave(kobj);
	int addr, len, idx;
	u16 crc = CRC16_INIT;

	EXT_INFOF("off:%lld; count:%d", off, count);
	count = w1_e15_fix_count(off, count, W1_E15_EEPROM_SIZE);
	if (count == 0)
	{
		EXT_ERRORF("Params error:off:%lld; count:%d", off, count);
		return 0;
	}


	if (w1_e15_enable_crccheck)
	{
		/* can only write full blocks in cached mode */
		if ((off & W1_E15_PAGE_MASK) || (count & W1_E15_PAGE_MASK))
		{
			dev_err(&sl->dev, "invalid offset/count off=%d cnt=%zd\n", (int)off, count);
			return -EINVAL;
		}

		/* make sure the block CRCs are valid */
		for (idx = 0; idx < count; idx += W1_E15_PAGE_SIZE)
		{
			crc = crc16(crc, &buf[idx], W1_E15_PAGE_SIZE);
			if ( crc != CRC16_VALID)
			{
				dev_err(&sl->dev, "bad CRC at offset %d, idx:%d: 0x%x\n", (int)off, idx, crc );
				return -EINVAL;
			}
		}
	}

	mutex_lock(&sl->master->mutex);

	/* Can only write data to one page at a time */
	idx = 0;
	while (idx < count)
	{
		addr = off + idx;
		len = W1_E15_PAGE_SIZE - (addr & W1_E15_PAGE_MASK);
		if (len > (count - idx))
			len = count - idx;

		/* addr is the parameter byte for page write */
		if (w1_e15_write_scratchpad(sl, addr, len, &buf[idx]) < 0)
		{
			count = -EIO;
			goto out_up;
		}
		
		idx += len;
	}

out_up:
	mutex_unlock(&sl->master->mutex);

	return count;
}

static BIN_ATTR_RW(MAC, W1_E15_PAGE_SIZE);

static ssize_t crc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (put_user(w1_e15_enable_crccheck + 0x30, buf))
		return -EFAULT;

	return sizeof(w1_e15_enable_crccheck);
}

static ssize_t crc_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char val;

	TRACE();
	if (count != 1 || !buf)
	{
		EXT_ERRORF("value invalidated");
		return -EINVAL;
	}

	EXT_INFOF("buf :'%s'", buf);
	if (get_user(val, (char *)buf))
	{
		EXT_ERRORF("Can't get data from user space");
		return -EFAULT;
	}

	/* convert to decimal */
	val = val - 0x30;
	if (val != 0 && val != 1)
	{
		EXT_ERRORF("value must be '1' or '0'");
		return -EINVAL;
	}

	/* set the new value */
	w1_e15_enable_crccheck = val;

	return sizeof(w1_e15_enable_crccheck);
}


/* device attributes: show/store */
static DEVICE_ATTR_RW(crc);

/* crc functions */
static struct attribute *w1_e15_attrs[] =
{
	&dev_attr_crc.attr,
	NULL,
};

/* eeprom and PIO functions */
static struct bin_attribute *w1_e15_bin_attrs[] =
{
	&bin_attr_page0,
	&bin_attr_key,
	&bin_attr_MAC,
	NULL,
};

static const struct attribute_group w1_e15_group =
{
	.attrs		= w1_e15_attrs,
	.bin_attrs	= w1_e15_bin_attrs,
};

static const struct attribute_group *w1_e15_groups[] =
{
	&w1_e15_group,
	NULL,
};

static int w1_e15_add_slave(struct w1_slave *sl)
{
	struct w1_e15_data *data = NULL;

	EXT_INFOF("Slave DS28E15 on %s is adding", BOARD_NAME );
	if (w1_e15_enable_crccheck)
	{
		data = kzalloc(sizeof(struct w1_e15_data), GFP_KERNEL);
		if (!data)
			return -ENOMEM;
		sl->family_data = data;
	}

	return 0;
}

static void w1_e15_remove_slave(struct w1_slave *sl)
{
	EXT_INFOF("Slave DS28E15 on %s is removing", BOARD_NAME );
	kfree(sl->family_data);
	sl->family_data = NULL;
}


static struct w1_family_ops w1_e15_fops =
{
	.add_slave      = w1_e15_add_slave,
	.remove_slave   = w1_e15_remove_slave,
	.groups		= w1_e15_groups,
};

static struct w1_family w1_family_e15 = 
{
	.fid = W1_FAMILY_DS28E15,
	.fops = &w1_e15_fops,
};

module_w1_family(w1_family_e15);

MODULE_AUTHOR("Jack Lee <jack@muxlab.com>");
MODULE_DESCRIPTION("w1 family 1C driver for DS28E15, 512b and SHA-256 EEPROM");
MODULE_LICENSE("GPL");
MODULE_ALIAS("w1-family-" __stringify(W1_FAMILY_DS28E15));

