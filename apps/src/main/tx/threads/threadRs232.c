//copy from 500762 RX
#include "serial.h"
#define MAX_CMD_LENGTH 256

char RS232_REMOTE_IP[16] = "10.0.0.175";
int RS232_REMOTE_PORT = 22222;
int RS232_IP_PORT = 22222;

int rs232PauseFlag = 1;
int rs232ExitFlag = 0;

int serial;
int rs232Sock;

void StrToHex(char *pbDest, char *pbSrc, int nLen)
{
	char h1, h2;
	char s1, s2;
	int i;

	for (i = 0; i < nLen; i++) {
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];
		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		pbDest[i] = (s1 << 4) | s2;
	}
}


void HexToStr(char *pbDest, char *pbSrc, int nLen)
{
	unsigned char ddl, ddh;
	int i;

	for (i = 0; i < nLen; i++) {
		ddh = 48 + (pbSrc[i] >> 4);
		ddl = 48 + (pbSrc[i] & 0xF);
		if (ddh > 57)
			ddh = ddh + 7;
		if (ddl > 57)
			ddl = ddl + 7;
		pbDest[i * 2] = ddh;
		pbDest[i * 2 + 1] = ddl;
	}

	pbDest[nLen * 2] = '\0';
}

struct termios oldtio;

int muxRs232Configure(MuxMain *muxMain, int timeout)
{
	int baudrate;
	struct termios newtio;
	
	switch (muxMain->rs232Ctrl.baud)
	{
		case 300:
			baudrate = B300;
			break;
		case 600:
			baudrate = B600;
			break;
		case 1200:
			baudrate = B1200;
			break;
		case 2400:
			baudrate = B2400;
			break;
		case 4800:
			baudrate = B4800;
			break;
		case 9600:
			baudrate = B9600;
			break;
		case 19200:
			baudrate = B19200;
			break;
		case 38400:
			baudrate = B38400;
			break;
		case 57600:
			baudrate = B57600;
			break;
		case 115200:
			baudrate = B115200;
			break;
		default:
			baudrate = B9600;
			break;
	}
	
	tcgetattr(muxMain->rs232Ctrl.fd, &newtio);
	tcgetattr(muxMain->rs232Ctrl.fd, &oldtio);
	
	bzero(&newtio, sizeof(newtio));

	//setting   c_cflag
	newtio.c_cflag &= ~CSIZE;
	switch (muxMain->rs232Ctrl.databits)
	{
		case 5:
			newtio.c_cflag |= CS5;
			break;
		case 6:
			newtio.c_cflag |= CS6;
			break;
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
		default:
			newtio.c_cflag |= CS8;
			break;
	}
	
	switch (parity)
	{
		case 'n':
		case 'N':
			newtio.c_cflag &= ~PARENB;   	// Clear parity enable
			newtio.c_iflag &= ~INPCK;     	// Disable parity checking
			break;
			
		case 'o':
		case 'O':
			newtio.c_cflag |= (PARODD | PARENB); //Odd | enable parity
			newtio.c_iflag |= INPCK;             //Enable parity checking
			break;
			
		case 'e':
		case 'E':
			newtio.c_cflag |= PARENB;     	// Enable parity
			newtio.c_cflag &= ~PARODD;		//Even
			newtio.c_iflag |= INPCK;      	// enable parity checking
			break;
			
		case 'S':
		case 's': // as no parity
			newtio.c_cflag &= ~PARENB;
			newtio.c_cflag &= ~CSTOPB;
			break;
			
		default:
			newtio.c_cflag &= ~PARENB;   // Clear parity enable
			newtio.c_iflag &= ~INPCK;     // Enable parity checking
			break;
	}
	
	switch (muxMain->rs232Ctrl.stopbits)
	{
		case 1:
			newtio.c_cflag &= ~CSTOPB; // 1
			break;
		case 2:
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
	newtio.c_cc[VMIN] = MAX_CMD_LENGTH - 1;	//0;
	//newtio.c_cflag |= (CLOCAL | CREAD);
	//newtio.c_oflag |= OPOST;
	//newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
	//newtio.c_lflag |=ECHO;
	cfsetispeed(&newtio, baudrate);
	cfsetospeed(&newtio, baudrate);
	tcflush(muxMain->rs232Ctrl.fd, TCIOFLUSH);

	if (tcsetattr(muxMain->rs232Ctrl.fd, TCSANOW, &newtio) != 0)
	{
		perror("SetupSerial");
		return -1;
	}
	
	return 0;
}

struct termios oldtio;

static int _muxThRs232Init(CmnThread *th, void *data)
{
	int res = EXIT_SUCCESS;
	MuxMain *muxMain = (MuxMain *)data;
	
	th->data = muxMain;
	
	muxMain->rs232Ctrl.fd = open(MUX_DEV_RS232, O_RDWR | O_NOCTTY /*| O_NDELAY*/);
	if(muxMain->rs232Ctrl.fd == -1)
	{
		MUX_ERROR("Can't Open Serial Port "MUX_DEV_RS232": %s", strerror(errno) );
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

int sendtoUart(int timeout, char * data, char *reply) {
	if (!data)
		return -1;

	int lens = strlen(data) / 2;
	if (lens > MAX_CMD_LENGTH)
		lens = MAX_CMD_LENGTH;
	char* toSend = malloc(lens + 2);
	StrToHex(toSend, data, lens);
	

	int res;
	res = write(serial, toSend, lens);
	free(toSend);
	if (res) {
		//printf("Write to %s: %s:%d\n", MODEMDEVICE, data, res);
	} else {
		printf("Failed to write to UART\n");
		return -1;
	}

	if (timeout) {

		fd_set rdfds;
		struct timeval tv;
		int ret;

		FD_ZERO(&rdfds);
		FD_SET(serial, &rdfds);
		tv.tv_sec = 0;
		tv.tv_usec = timeout * 1000;
		ret = select(serial + 1, &rdfds, NULL, NULL, &tv);
		if (ret < 0) {
			printf("select\n");
			return -2;
		} else if (ret == 0) {
			printf("timeout\n");
			return -2;
		} else {
			if (FD_ISSET(serial, &rdfds)) {
				char tmp[MAX_CMD_LENGTH];
				int len = 0;
				len = read(serial, tmp, MAX_CMD_LENGTH);				

				if (len < 0) {
					printf("Receive error\n");
					return -2;
				} else if (len == 0) {
					printf("Nothing received\n");
					return -2;
				} else {
					tmp[len] = 0;

					HexToStr(reply, tmp, len);
				}
			}
		}
	}

	return 0;
}

static void _muxThRs232Destory(struct _CmnThread *th)
{
	MuxMain *muxMain = (MuxMain *)th->data;
	int ret = 0;

	if (muxMain->rs232Ctrl.fd)
	{
		if (tcsetattr(muxMain->rs232Ctrl.fd, TCSANOW, &oldtio) != 0)
		{
			perror("Recover Serial config");
			ret = -1;
		}
	}
	
	close(muxMain->rs232Ctrl.fd);
	muxMain->rs232Ctrl.fd = 0;
	return ret;
}

int initRS232PassThrough()
{
	struct sockaddr_in addr;

	rs232Sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (rs232Sock < 0) {
		printf("Socket Error:%s\n", strerror(errno));
		return -1;
	}

	int yes;
	if (setsockopt(rs232Sock, SOL_SOCKET, SO_REUSEADDR, (char *) &yes, sizeof(yes)) < 0){
		printf("Reusing ADDR failed\n");
		//return -1;
	}

	bzero(&addr, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(RS232_IP_PORT);

	if (bind(rs232Sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0) {
		printf("RS232 Bind Error:%s\n", strerror(errno));
		return -2;
	}
	return 0;
}

int deinitRS232PassThrough()
{
	rs232ExitFlag = 1;
	sleep(1);
	return close(rs232Sock);
}

static int _muxThLedMain(CmnThread *th)
{
	int res = EXIT_SUCCESS;
	MuxMain *muxMain = (MuxMain *)th->data;
	int ret = 0;
	
	while (rs232ExitFlag == 0) 
	{		
		while(rs232PauseFlag == 1)
		{
			usleep(10000);			
		}
		
		fd_set rdfds;
		struct timeval tv;
		
		FD_ZERO(&rdfds);
		FD_SET(muxMain->rs232Ctrl.fd, &rdfds);
		FD_SET(rs232Sock, &rdfds);

		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		
		int nb = muxMain->rs232Ctrl.fd > rs232Sock ? muxMain->rs232Ctrl.fd + 1 : rs232Sock + 1;
		ret = select(nb, &rdfds, NULL, NULL, &tv);
		if (ret < 0) 
		{
			MUX_ERROR("Serial select error: %s", strerror(errno));
		}
		else if (ret == 0) 
		{
			MUX_INFO("serial Timeout");
		} 
		else 
		{
			if (FD_ISSET(muxMain->rs232Ctrl.fd, &rdfds)) 
			{				
				char tmp[MAX_CMD_LENGTH];
				memset(tmp, 0, sizeof(tmp));
				int len = 0;
				
				if ((len = read(muxMain->rs232Ctrl.fd, tmp, MAX_CMD_LENGTH)) < 0) 
				{
					MUX_ERROR("RS232 receive error: %s", strerror(errno) );
				} 
				else if (len == 0) 
				{
					MUX_INFO("RS232: Nothing received");
				} 
				else 
				{				
					struct sockaddr_in addr;
					bzero(&addr, sizeof(struct sockaddr_in));
					addr.sin_family = AF_INET;
					addr.sin_port = htons(RS232_REMOTE_PORT);
					
					if (inet_aton(RS232_REMOTE_IP, &addr.sin_addr) < 0) 
					{
						fprintf(stderr, "Ip error:%s\n", strerror(errno));
						continue;
					}
					printf("\nRS232 Send data to : %s\n", RS232_REMOTE_IP);
					sendto(rs232Sock, tmp, len, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));										
				}
			}

			if (FD_ISSET(rs232Sock, &rdfds)) 
			{
				char tmp[MAX_CMD_LENGTH];
				memset(tmp, 0, sizeof(tmp));
				int len = 0;
				struct sockaddr_in addr;
				socklen_t addrlen = sizeof(struct sockaddr_in);
				if ((len = recvfrom(rs232Sock, tmp, MAX_CMD_LENGTH, 0, (struct sockaddr *) &addr, &addrlen)) < 0) 
				{
					printf("Receive error\n");
				} 
				else if (len == 0) 
				{
					printf("Nothing received\n");
				} 
				else 
				{
					if (strcmp(inet_ntoa(addr.sin_addr), RS232_REMOTE_IP) == 0) 
					{
						int ret = write(muxMain->rs232Ctrl.fd, tmp, len);
						if (ret == -1) 
						{
							MUX_ERROR("RS232: Failed to write to serial:%s", strerror(errno) );
						} 
						else 
						{
							//printf("Send to rs232:%s\n", tmp);
						}
					}
				}
			}
		}
	}

	return 0;
}

CmnThread  threadMuxRs232 =
{
	name		:	"Button",
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_muxThRs232Init,
	mainLoop		:	_muxThRs232Main,
	eventHandler	:	NULL,
	destory		:	_muxThRs232Destory,
	data			:	NULL,
};

