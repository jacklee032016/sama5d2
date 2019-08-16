/*
* RS232, from USART1 --> FPGA UART --> RS232
* Jun.8th, 2018  Jack Lee
* port to Linux, 05.11, 2019
*/


#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>	
#include <unistd.h>
#include <string.h>

#include "libCmn.h"
#include "extSysParams.h"
#include "mux7xx.h"


void rs232StartRx(void)
{/* init IRQ, index of buffer */
}

void rs232StopRx(void)
{/* Disable interrupt  */
}


int cmnSys232Read(unsigned char *data, unsigned short size)
{
	unsigned int i = 0;
//	unsigned char *d = data;
	
#if EXT_RS232_DEBUG
	printf("RS232: input:'");
#endif
	for(i=0; i< size; i++)
	{
//		usart_serial_getchar(USART1, (data+i));

#if EXT_RS232_DEBUG
		printf("0x%x ", *(data+i));
#endif
		if( *(data+i) == '\n' || *(data+i) == '\r' )
		{
			return i;
		}

	}
	
#if EXT_RS232_DEBUG
	printf("'"EXT_NEW_LINE);
#endif

	return size;
}


int cmnSysRs232Write(unsigned char *data, unsigned short size)
{
	unsigned int i;
//	unsigned char *d = data;
	
#if EXT_RS232_DEBUG
	printf("RS232: output:'");
#endif
	for(i=0; i< size; i++)
	{
#if 1// def X86
		if(0)
#else
		if( usart_serial_putchar(USART1, *(data+i)) == 0)
#endif			
		{
			EXT_ERRORF("Write RS232 failed on #%u", i);
			return EXIT_FAILURE;
		}
#if EXT_RS232_DEBUG
		printf("%c ", *(data+i));
#endif
	}
	
#if EXT_RS232_DEBUG
	printf("'"EXT_NEW_LINE);
#endif

	return EXIT_SUCCESS;
}

static void _checkRs232Params(EXT_RUNTIME_CFG *runCfg)
{
	if(  runCfg->rs232Cfg.baudRate != EXT_BAUDRATE_9600 && 
		runCfg->rs232Cfg.baudRate != EXT_BAUDRATE_19200 && 
		runCfg->rs232Cfg.baudRate != EXT_BAUDRATE_38400 && 
		runCfg->rs232Cfg.baudRate != EXT_BAUDRATE_57600 &&  
		runCfg->rs232Cfg.baudRate != EXT_BAUDRATE_115200 )
	{
		runCfg->rs232Cfg.baudRate = EXT_BAUDRATE_115200;
		EXT_ERRORF("RS232 configuration is not correct, please reset or configure it");
	}
}

/** 
 * \brief Initializes the Usart in master mode.
 *
 * \param p_usart  Base address of the USART instance.
 * \param opt      Options needed to set up RS232 communication (see
 * \ref usart_options_t).
 */
int cmnSysRs232Config(EXT_RUNTIME_CFG *runCfg)
{

#if 0

	sam_usart_opt_t _settings;

#if EXT_RS232_DEBUG
	printf("RS232: Parity:");
#endif
	if(runCfg->rs232Cfg.parityType == EXT_RS232_PARITY_ODD )
	{
		_settings.parity_type = US_MR_PAR_ODD;
#if EXT_RS232_DEBUG
		printf("ODD");
#endif
	}
	else if(runCfg->rs232Cfg.parityType == EXT_RS232_PARITY_EVEN )
	{
		_settings.parity_type = US_MR_PAR_EVEN;
#if EXT_RS232_DEBUG
		printf("EVEN");
#endif
	}
	else
	{/* default no parity */
		_settings.parity_type = US_MR_PAR_NO;
#if EXT_RS232_DEBUG
		printf("NO");
#endif
	}

#if EXT_RS232_DEBUG
	printf(EXT_NEW_LINE": CharBits:");
#endif
	if(runCfg->rs232Cfg.charLength == EXT_RS232_CHAR_LENGTH_5 )
	{
		_settings.char_length = US_MR_CHRL_5_BIT;
#if EXT_RS232_DEBUG
		printf("5");
#endif
	}
	else if(runCfg->rs232Cfg.charLength == EXT_RS232_CHAR_LENGTH_6 )
	{
		_settings.char_length = US_MR_CHRL_6_BIT;
#if EXT_RS232_DEBUG
		printf("6");
#endif
	}
	else if(runCfg->rs232Cfg.charLength == EXT_RS232_CHAR_LENGTH_7 )
	{
#if EXT_RS232_DEBUG
		printf("7");
#endif
		_settings.char_length = US_MR_CHRL_7_BIT;
	}
	else
	{/* default 8 bits */
#if EXT_RS232_DEBUG
		printf("8");
#endif
		_settings.char_length = US_MR_CHRL_8_BIT;
	}
	
#if EXT_RS232_DEBUG
	printf(EXT_NEW_LINE": StopBits:");
#endif
	if(runCfg->rs232Cfg.stopbits == EXT_RS232_STOP_BITS_2 )
	{
#if EXT_RS232_DEBUG
		printf("2");
#endif
		_settings.stop_bits = US_MR_NBSTOP_2_BIT;
	}
	else if(runCfg->rs232Cfg.stopbits == EXT_RS232_STOP_BITS_15 )
	{
#if EXT_RS232_DEBUG
		printf("1.5");
#endif
		_settings.stop_bits = US_MR_NBSTOP_1_5_BIT;
	}
	else
	{/* default 1 stop bit*/
#if EXT_RS232_DEBUG
		printf("1");
#endif
		_settings.stop_bits = US_MR_NBSTOP_1_BIT;
	}
	
	_settings.baudrate = runCfg->rs232Cfg.baudRate;

#if EXT_RS232_DEBUG
	printf(EXT_NEW_LINE"baudrate:%ld"EXT_NEW_LINE, _settings.baudrate );
#endif	

	_settings.channel_mode= US_MR_CHMODE_NORMAL;

	sysclk_enable_peripheral_clock(ID_USART1);

	/* Configure USART */
	usart_init_rs232(USART1, &_settings, sysclk_get_peripheral_hz())/* eg. 0x40028000U */;

	/* Enable the receiver and transmitter. */
	usart_enable_tx(USART1);
	usart_enable_rx(USART1);
#endif

	return EXIT_SUCCESS;
}


/* called when startup. init pin and default params  */
int cmnSysRs232Init(EXT_RUNTIME_CFG *runCfg)
{
#if EXT_RS232_DEBUG
	printf("Configure pins of RS232(USART)..."EXT_NEW_LINE);
#endif
	_checkRs232Params(runCfg);

#if 0
	/* Configure USART pins : connect to RS232 */
	ioport_set_pin_peripheral_mode(USART1_RXD_GPIO, USART1_RXD_FLAGS);
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;
	ioport_set_pin_peripheral_mode(USART1_TXD_GPIO, USART1_TXD_FLAGS);
	
	sysclk_enable_peripheral_clock(ID_USART1);
#endif

	/* Configure USART */
#if EXT_RS232_DEBUG
	printf("Configure params of RS232(USART)..."EXT_NEW_LINE);
#endif
	cmnSysRs232Config(runCfg);

	cmnSysRs232Write((unsigned char *)"RS232 OK", 8);

	return EXIT_SUCCESS;
}




