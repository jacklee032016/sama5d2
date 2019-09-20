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
#include "libCmnSys.h"
#include "mux7xx.h"

#include <termios.h>
	   
#define _MAX_CMD_LENGTH		256

struct termios oldtio;

static HwRs232Ctrl		_rs232Ctrl =
{
	.fd = -1,
};

#define	RS232_CHECK_FD()	\
	{if(_rs232Ctrl.fd <= 0 ){MUX_ERROR("RS232 is not initialized"); return EXIT_FAILURE;} }


void rs232StartRx(void)
{/* init IRQ, index of buffer */
}

void rs232StopRx(void)
{/* Disable interrupt  */
}


int cmnSysRs232Read(unsigned char *data, unsigned short size, int timeoutMs)
{
	HwRs232Ctrl		*rs232Ctrl = &_rs232Ctrl;
//	int timeoutMs = 950; 
	int len = 0;
//	unsigned char *d = data;

	RS232_CHECK_FD();
#if EXT_RS232_DEBUG
	printf("RS232: input:'");
#endif
#if 0
	unsigned int i = 0;
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
#endif

	fd_set rdfds;
	struct timeval tv;
	int ret;

	FD_ZERO(&rdfds);
	FD_SET(rs232Ctrl->fd, &rdfds);
	
	tv.tv_sec = 0;
	tv.tv_usec = timeoutMs * 1000;
	ret = select(rs232Ctrl->fd + 1, &rdfds, NULL, NULL, &tv);
	if (ret < 0)
	{
#if EXT_RS232_DEBUG
		printf("'");
#endif
		MUX_ERROR("RS232: select on RS232 failed: %m");
		return -2;
	}
	else if (ret == 0)
	{
#if EXT_RS232_DEBUG
		printf("'");
#endif
		MUX_INFO("RS232: Timeout after %d ms", timeoutMs);
		return -2;
	}
	else
	{
		if (FD_ISSET(rs232Ctrl->fd, &rdfds))
		{
			char tmp[_MAX_CMD_LENGTH];
			memset(tmp, 0, sizeof(tmp) );
			
			len = read(rs232Ctrl->fd, tmp, _MAX_CMD_LENGTH);				
			if (len < 0)
			{
				MUX_INFO("RS232: receiving failed: %m");
			}
			else if (len == 0)
			{
				MUX_INFO("RS232: Nothing received");
			}
			else
			{
				printf("%.*s", len, tmp );
				memcpy(data, tmp, (len >= size)?size: len);
			}
		}
	}

	
#if EXT_RS232_DEBUG
	printf("'"EXT_NEW_LINE);
#endif

	return len;
}



int cmnSysRs232Write(unsigned char *data, unsigned short size)
{
	HwRs232Ctrl		*rs232Ctrl = &_rs232Ctrl;
//	unsigned char *d = data;
	
	RS232_CHECK_FD();
#if EXT_RS232_DEBUG
	printf("RS232: output:'");
#endif

#if 0
	unsigned int i;
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
#else

	/* clear all for next reply */
	if(tcflush(rs232Ctrl->fd, TCIOFLUSH)) /* TCIOFLUSH: Flush both.*/
	{
		MUX_WARN("RS232: Flash RX/TX failed: %m");
	}


	int ret = write(rs232Ctrl->fd, data, size);
	if (ret == -1) 
	{
		MUX_ERROR("RS232: Failed to write to serial:%s", strerror(errno) );
	} 
	else 
	{
#if EXT_RS232_DEBUG
		printf("%s", data);
#endif
	}
#endif

#if EXT_RS232_DEBUG
	printf("'"EXT_NEW_LINE);
#endif

	return ret;
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
#else
	HwRs232Ctrl		*rs232Ctrl = &_rs232Ctrl;

	int baudrate;
	struct termios newtio;
	
	RS232_CHECK_FD();
	switch (runCfg->rs232Cfg.baudRate)
	{
		case EXT_BAUDRATE_300:
			baudrate = B300;
			break;
		case EXT_BAUDRATE_600:
			baudrate = B600;
			break;
		case EXT_BAUDRATE_1200:
			baudrate = B1200;
			break;
		case EXT_BAUDRATE_2400:
			baudrate = B2400;
			break;
		case EXT_BAUDRATE_4800:
			baudrate = B4800;
			break;
		case EXT_BAUDRATE_9600:
			baudrate = B9600;
			break;
		case EXT_BAUDRATE_19200:
			baudrate = B19200;
			break;
		case EXT_BAUDRATE_38400:
			baudrate = B38400;
			break;
		case EXT_BAUDRATE_57600:
			baudrate = B57600;
			break;
		case EXT_BAUDRATE_115200:
			baudrate = B115200;
			break;
		default:
			baudrate = B9600;
			break;
	}
	
	tcgetattr(rs232Ctrl->fd, &newtio);
	tcgetattr(rs232Ctrl->fd, &oldtio);
	
	bzero(&newtio, sizeof(newtio));

	//setting   c_cflag
	newtio.c_cflag &= ~CSIZE;
	switch (runCfg->rs232Cfg.charLength )
	{
		case EXT_RS232_CHAR_LENGTH_5:
			newtio.c_cflag |= CS5;
			break;
		case EXT_RS232_CHAR_LENGTH_6:
			newtio.c_cflag |= CS6;
			break;
		case EXT_RS232_CHAR_LENGTH_7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
		default:
			newtio.c_cflag |= CS8;
			break;
	}
	
	switch (runCfg->rs232Cfg.parityType)
	{
		case EXT_RS232_PARITY_ODD:
			newtio.c_cflag |= (PARODD | PARENB); //Odd | enable parity
			newtio.c_iflag |= INPCK;             //Enable parity checking
			break;
			
		case EXT_RS232_PARITY_EVEN:
			newtio.c_cflag |= PARENB;     	// Enable parity
			newtio.c_cflag &= ~PARODD;		//Even
			newtio.c_iflag |= INPCK;      	// enable parity checking
			break;
			
		case EXT_RS232_PARITY_NONE:
		default:/* none */
			newtio.c_cflag &= ~PARENB;   // Clear parity enable
			newtio.c_iflag &= ~INPCK;     // Enable parity checking
			break;
	}
	
	switch (runCfg->rs232Cfg.stopbits )
	{
		case EXT_RS232_STOP_BITS_1:
			newtio.c_cflag &= ~CSTOPB; // 1
			break;
		case EXT_RS232_STOP_BITS_2:
			newtio.c_cflag |= CSTOPB; // 2
			break;
		default:
			newtio.c_cflag &= ~CSTOPB;
			break;
	}

	newtio.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	newtio.c_iflag &= ~(IGNPAR | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY);
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	newtio.c_oflag &= ~OPOST;


	/*
	 initialize all control characters
	 default values can be found in /usr/include/termios.h, and are given
	 in the comments, but we don't need them here
	 */
	//newtio.c_cc[VMIN] = 0;
	//newtio.c_cc[VTIME] = 20; /* inter-character timer unused */
	newtio.c_cc[VTIME] = 1;	//timeout/100/2;
	newtio.c_cc[VMIN] = _MAX_CMD_LENGTH - 1;	//0;
	//newtio.c_cflag |= (CLOCAL | CREAD);
	//newtio.c_oflag |= OPOST;
	//newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
	//newtio.c_lflag |=ECHO;
	cfsetispeed(&newtio, baudrate);
	cfsetospeed(&newtio, baudrate);
	tcflush(rs232Ctrl->fd, TCIOFLUSH);

	if (tcsetattr(rs232Ctrl->fd, TCSANOW, &newtio) != 0)
	{
		MUX_ERROR("RS232: configuration failed: %m");
		return EXIT_FAILURE;
	}
#endif

	MUX_DEBUG("RS232 configured: baudrate: %d; bits: %d; parity: %s; stopbits: %d", 
		runCfg->rs232Cfg.baudRate, runCfg->rs232Cfg.charLength, CMN_FIND_RS_PARITY(runCfg->rs232Cfg.parityType), runCfg->rs232Cfg.stopbits);

	return EXIT_SUCCESS;
}


/* called when startup. init pin and default params  */
int cmnSysRs232Init(EXT_RUNTIME_CFG *runCfg)
{
	HwRs232Ctrl		*rs232Ctrl = &_rs232Ctrl;

	_checkRs232Params(runCfg);

	if( rs232Ctrl->fd > 0 )
	{
		MUX_WARN("RS232 has been initialized" );
		return EXIT_SUCCESS;
	}

	rs232Ctrl->fd = open(MUX_DEV_RS232, O_RDWR | O_NOCTTY /*| O_NDELAY*/);
	if(rs232Ctrl->fd == -1)
	{
		MUX_ERROR("Can't Open Serial Port "MUX_DEV_RS232": %s", strerror(errno) );
		return EXIT_FAILURE;
	}

	/* Configure USART */
#if EXT_RS232_DEBUG
	printf("Configure params of RS232..."EXT_NEW_LINE);
#endif
	cmnSysRs232Config(runCfg);

	cmnSysRs232Write((unsigned char *)"RS232 OK", 8);

	return EXIT_SUCCESS;
}




