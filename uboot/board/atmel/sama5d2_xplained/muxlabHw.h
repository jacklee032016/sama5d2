
#ifndef	__MUXLAB_HW_H__
#define	__MUXLAB_HW_H__

#define	EXT_I2C_PCA9554_CS_MAC				(4)	/* CS0, MAC address; not used */
#define	EXT_I2C_PCA9554_CS_SENSOR			(5)	/* CS1, sensor and EEPROM  */
#define	EXT_I2C_PCA9554_CS_SWITCH			(6)	/* CS2, FPGA */
#define	EXT_I2C_PCA9554_CS_BOOTROM			(7)	/* CS3, Ethernet switch */

#define	EXT_I2C_PCA9554_CS_NONE				(0xFF) /* connect to I2C of MCU directly */

/* folllowing devices attached on I2C of LM1983 */
#define	EXT_I2C_ADDRESS_SENSOR             		(0x30 >> 1)
#define	EXT_I2C_ADDRESS_AT24MAC             		(0xAE >> 1)
#define	EXT_I2C_ADDRESS_XC7A75T             		(0xFF >> 1)	/* T.B.D. */
#define	EXT_I2C_ADDRESS_RTL8035				0x54


/* registers for LM95245 sensor */
#define	EXT_I2C_SENSOR_LOCAL_TEMP_MSB		0x00
#define	EXT_I2C_SENSOR_LOCAL_TEMP_LSB		0x30
#define	EXT_I2C_SENSOR_MANUFACTURE_ID		0xFE
#define	EXT_I2C_SENSOR_REVISION_ID			0xFF

#define PCA9544_ADDR	(0xE0>>1)


#define I2C_MUX_PCA_ADDR	(0xE0>>1)


int extDdrMemoryTest(void);

#endif

