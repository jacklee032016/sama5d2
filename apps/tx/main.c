#include <pthread.h>
#include "rx_lib.h"
#include <stdlib.h>
#include "FPGA.h"
#include "UDP.h"
#include "multithread.h"
#include "controler.h"
#include "wrapper.h"
#include "sha256_software.h"
#include "serial.h"
#include "led.h"
#include "muxbutton.h"
#include "spi.h"
#ifndef SIMPLE_ADV_PACKETS  
#include "ADV7619_io_map_fct.h"
#include "EP91A6SX.h"
#endif

extern void EXP_reset();
extern void HDMI_reset();
#ifndef SIMPLE_ADV_PACKETS  
 extern void RX_set_internal_EDID();
 extern void rx_init(unsigned char * ptr);
 extern void parse_avi_infoframe(unsigned char * avi_iframe, unsigned char * colorspace, unsigned char * vic);
 extern int initialize_adv(RX_COLOR_DEPTH DeepColorMode, unsigned char iframe_colorspace, unsigned short tmds_freq, unsigned char *video_config_reg);
#endif
extern int paramInitDone;
extern int MacStrToBin(char *str, unsigned  char *mac);

extern int initUart();
extern int deinitUart();
extern int initRS232PassThrough();
extern int deinitRS232PassThrough();
extern int rs232PassThrough();
extern int sendtoUart(int timeout, char * data, char *reply);

#ifdef IR_PASSTHROUGH_ENABLE
 extern int initIR();
 extern void deinitIR();
 extern int initIRPassThrough();
 extern int deinitIRPassThrough();
 extern int irPassThrough();
 extern int setChipIRMode(char * mode);
#endif
extern char g_multicast_ip[16];
extern void *power_led();

extern int spi0_file;
extern unsigned char g_flag_LOGs;

int data_valid_10G;

unsigned long g_num_info_frame;

int main(void)
{
	UINT16 tmdsfreq = 0;
	UCHAR tmdsfreqfrac = 0;
	UINT16 tmdsfreq_prev = 0;
	UCHAR tmdsfreqfrac_prev = 0;
	UINT16 RxRevision;
		
	BOOL ManualMode;
	BOOL TmdsDetected;
	int Ret;
	int isRS232DevOn = 0;
	int isIRDevOn = 0;
	unsigned char stream_en[2];
	unsigned char spi_buf[16];
	
	g_flag_LOGs = 0x0F;
	g_num_info_frame = 0;
	connection_enable_flag = 0;
	multicast_ip_change_flag = 0;
	strncpy(g_multicast_ip, "239.0.1.254", 15);
	power_led_mode = LED_MODE_ON;
	data_valid_10G = 0;
	
	//disable FPGA video audio stream
	stream_en[0] = 0;
	stream_en[1] = 0;
	I2C_Write(0x60, 0x03, stream_en, 1);
	
	printf("TX app ver 000005.\n");
	
	if (!SHA256_App_Check())
		printf("SHA check ok!\n");
	else
		printf("SHA check error!\n");	
	
	Ret = initUart();
	if (Ret == -1)		
		printf("No RS232 device found\n");	
	else 
	{
		Ret = initRS232PassThrough();
		if (Ret == -1) 			
			printf("Init RS232 passthrough failed\n");		
		else 		
			isRS232DevOn = 1;		
	}
#ifdef IR_PASSTHROUGH_ENABLE
	Ret = initIR();
	if (Ret == -1) 
	{
		printf("No IR device found\n");
	}
	else 
	{
		Ret = initIRPassThrough();
		if (Ret == -1) 
		{
			printf("Init IR passthrough failed\n");
		}
		else 
		{
			isIRDevOn = 1;
		}
	}
#endif		
	
	
	fpga_init();	
	usleep(100000);	

#ifndef SIMPLE_ADV_PACKETS  
 	ATV_ERR ErrVal = ATVERR_OK;
	//Get the chip revision
	ErrVal = ADIAPI_RxGetChipRevision(&RxRevision);
	if (ErrVal != ATVERR_OK)
	{
		printf("Error reading RxRevision\n");
	}
	
	// Initialize the operating mode	
	ErrVal = ADIAPI_RxInit(RX_OPMODE_HDMI);	
	if (ErrVal != ATVERR_OK)
	{
		printf("Error initializing default operating mode \n");
	}
	//rx_init((UCHAR *)RxInit_Table_RGB_444_2x24);
     
	ADIAPI_RxHdmiReqBgMeas(PORT_B);
	//    ADIAPI_RxHdmiSetSelectedPort(PORT_B, TRUE);
	sleep(1);
	ADIAPI_RxHdmiGetBgTmdsFreq(PORT_B, &tmdsfreq, &tmdsfreqfrac);       
		                   
	//Intilaze the internal EDID
	RX_set_internal_EDID();	
	EXP_reset();
	sleep(3);
	EP91A6SX_Init();
#endif 
	
	thread_muxbutton_ret = pthread_create(&thread_muxbutton, NULL, muxbtn_routine, NULL);	
    	
	//thread_watchdog_ret = pthread_create(&thread_watchdog, NULL, watchdog_routine, NULL);	
	thread_tx_server_ret = pthread_create(&thread_tx_server, NULL, tx_server_routine, NULL);
	while (paramInitDone == 0) 
	{
		usleep(10 * 1000);		
	}	
	thread_ADV_ret = pthread_create(&thread_ADV, NULL, ADV_routine, NULL);
	
	thread_link_10G_ret = pthread_create(&thread_link_10G, NULL, link_10G_routine, NULL);	
	
	if (isRS232DevOn)		
		thread_rs232_ret = pthread_create(&thread_rs232, NULL, (void*)rs232PassThrough, NULL);			
	
	
#ifdef IR_PASSTHROUGH_ENABLE
 	if (isIRDevOn)
 		thread_ir_ret = pthread_create(&thread_ir, NULL, (void*)irPassThrough, NULL);
#endif
	
	thread_power_led_ret = pthread_create(&thread_power_led, NULL, (void*)power_led, NULL);			
	
	pthread_join(thread_muxbutton, NULL);
	//pthread_join(thread_watchdog, NULL);
	pthread_join(thread_tx_server, NULL);
	pthread_join(thread_ADV, NULL);
	pthread_join(thread_rs232, NULL);
#ifdef IR_PASSTHROUGH_ENABLE
 	pthread_join(thread_ir, NULL);
#endif
	pthread_join(thread_power_led, NULL);
	pthread_join(thread_link_10G, NULL);
}


int save_adv_null_log()
{
	FILE *fp;
	fp = fopen(ADV_LOG_FILE, "w");
	if(NULL == fp)
	{
		printf("Cannot open ADV log file. Error!\n");
		return -1;		
	}
	else
	{
		fprintf(fp, "Parameter not available now.\n");
		fclose(fp);
		return 0;
	}
}

void *ADV_routine()
{
	int Ret;
	unsigned char spi_buf[16];

	UINT16 tmdsfreq = 0;
	UCHAR tmdsfreqfrac = 0;
	UINT16 tmdsfreq_prev = 0;
	UCHAR tmdsfreqfrac_prev = 0;	
	unsigned int CRC_Value;	
	unsigned char fpga_data[57];
	int i, j = 0;
	int adv_fpga_init_done = 0;
	static UCHAR video_config_reg;
	//	static BOOL raw_mode;
		static BOOL raw_mode = FALSE; //i
		int sent_flag = 5;
	unsigned char data_to_send[512];
	unsigned char avstream_en[2];
	UCHAR iframe_colorspace = 0;
	UCHAR iframe_colorspace_prev = 0;
	UCHAR iframe_VIC;
	UCHAR iframe_VIC_prev;
	UCHAR ret;	
	UCHAR buffer[128], tx_data[6];
	UCHAR stream_enable = 0;
	RX_COLOR_DEPTH DeepColorMode = 0;
	RX_COLOR_DEPTH DeepColorMode_prev = 0;
	RX_CS_COLOR ColorSpace = 0;
	RX_RANGE range = 0;
	RX_CSC CSC = RX_CP_CSC;
	ATV_ERR ErrVal = ATVERR_OK;
	UCHAR	PktBuf_vsinfo[MAX_HDMI_PACKET_SIZE];
	UCHAR	PktBuf_aviinfo[MAX_HDMI_PACKET_SIZE];
	UCHAR	PktBuf_spdinfo[MAX_HDMI_PACKET_SIZE];
	UCHAR	PktBuf_audioinfo[MAX_HDMI_PACKET_SIZE];
	UCHAR	PktBuf_mpeginfo[MAX_HDMI_PACKET_SIZE];
	UCHAR	PktBuf_acp[MAX_HDMI_PACKET_SIZE];
	UCHAR	PktBuf_isrc1[MAX_HDMI_PACKET_SIZE];
	UCHAR	PktBuf_isrc2[MAX_HDMI_PACKET_SIZE];
	UCHAR	PktBuf_gamut[MAX_HDMI_PACKET_SIZE];

	BOOL CsumErr_vsinfo;
	BOOL CsumErr_aviinfo;
	BOOL CsumErr_spdinfo;
	BOOL CsumErr_audioinfo;
	BOOL CsumErr_mpeginfo;
	BOOL CsumErr_acp;
	BOOL CsumErr_isrc1;
	BOOL CsumErr_isrc2;
	BOOL CsumErr_gamut;	
	RX_HDMI_HSYNC HsyncInfo;
	RX_HDMI_VSYNC VsyncInfo;
	
	struct sockaddr_in addr;
	int addrlen, tx_sock, cnt;
	struct ip_mreq mreq;	
	//char local_multicast_ip[16];
	
	unsigned short x1 = 0, x2 = 0, x3 = 0;
	unsigned short x;
	unsigned int p, pixel_clk_freq = 0;	
	
	int video_led_fd;
	FILE *fp_adv_log;
	
	video_led_fd = open("/dev/videoled", O_RDWR);
	if (video_led_fd < 0)
	{
		printf("can not open /dev/videoled!\n");		
	}	
	//set video led OFF when power on.
	ioctl(video_led_fd, VIDEO_LED_OFF, 0);		
	//init local_multicast_ip
	//strncpy(local_multicast_ip, "239.0.1.254", 15);	
	
	//Mux cec function init
#ifndef SIMPLE_ADV_PACKETS
	ADIAPI_MUX_RxCEC_Init();		
	ADIAPI_RxHdmiSetDeepColorMode(0, 0);	
	usleep(500000);
	ADIAPI_RxHdmiGetDeepColorMode(&DeepColorMode);
	raw_mode = initialize_adv(DeepColorMode, iframe_colorspace, tmdsfreq, &video_config_reg);		
#endif	
	//init socket
	tx_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (tx_sock < 0) 
	{
		perror("ADV_routine socket create error\n");	
	}
	/*
	unsigned int yes = 1;            //MODIFICATION TO ORIGINAL
	if (setsockopt(tx_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) 
	{
		perror("Reusing ADDR failed");	
	}
	*/
	
	bzero((char *)&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(INFO_FRAME_PORT);
	addrlen = sizeof(addr);	
	
	mreq.imr_multiaddr.s_addr = inet_addr(g_multicast_ip);         												
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);         		
	if (setsockopt(tx_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) 
	{
		perror("setsockopt mreq");
	}  	
	
	while (1)
	{	
#ifdef SPI_TEST_ENABLE
		if (spi0_file) {
			spi_buf[0] = 0xFF;
			spi_buf[1] = 0xFF;
			Ret = spi_read(spi0_file, SPI_TST_ADDR, spi_buf, 2); //reading the address 0xE60E
			if(Ret == RES_SPI_SUCCESS) {
				if ((g_flag_LOGs & 0x08) != 0) {
					printf("SPI read: 0x%02X%02X\n", spi_buf[0], spi_buf[1]);	
				}
			}
			else {
				printf("SPI test error: can't read SPI.\n");		
			}
		}
#endif
		pthread_mutex_lock(&i2c_bus_lock);		
#ifndef SIMPLE_ADV_PACKETS
		ADIAPI_RxIsr();
		ADIAPI_RxHouseKeeping();		
		
		//cec function		
		ADIAPI_MUX_RxCEC_Send_Msg_Out();				
		
		ADIAPI_RxHdmiGetBgTmdsFreq(PORT_B, &tmdsfreq, &tmdsfreqfrac);		
#endif		
		memset(PktBuf_aviinfo, 0, sizeof(PktBuf_aviinfo));
#ifndef SIMPLE_ADV_PACKETS
		ADIAPI_RxHdmiGetPacket(AVI_INFO_PACKET, PktBuf_aviinfo, &CsumErr_aviinfo);
		parse_avi_infoframe(PktBuf_aviinfo, &iframe_colorspace, & iframe_VIC);
				
				if((tmdsfreq != tmdsfreq_prev) ||
					(tmdsfreq == 0) || 
					(tmdsfreqfrac_prev != tmdsfreqfrac) || 
					(iframe_colorspace != iframe_colorspace_prev))
		{
			//printf("HDMI NOT stable. tmdsfreq=%d; tmdsfreq_prev=%d; colorspace=%d; colorspace_prev=%d \n", tmdsfreq, tmdsfreq_prev, iframe_colorspace, iframe_colorspace_prev);						
			tmdsfreq_prev = tmdsfreq;
			tmdsfreqfrac_prev = tmdsfreqfrac;
			iframe_colorspace_prev = iframe_colorspace;			
			//dbg			I2C_Read(0x60, 0x03, avstream_en, 1);
			//dbg			avstream_en[0] &= 0xFE;
			//dbg			I2C_Write(0x60, 0x03, avstream_en, 1);						
			pthread_mutex_unlock(&i2c_bus_lock);
			//printf("HDMI not stable. Disable FPGA stream.\n");			
			save_adv_null_log();
			adv_fpga_init_done = 0;					
			ioctl(video_led_fd, VIDEO_LED_OFF, 0);	
			data_valid_10G = 0;			
			usleep(500000);
			continue;
		}	
		//HDMI stable now		
		if(adv_fpga_init_done == 0)
#endif
		{
#ifndef SIMPLE_ADV_PACKETS
			printf("HDMI valid. Init adv & fpga\n");
			ADIAPI_RxHdmiSetDeepColorMode(0, 0);
			usleep(500000);
			ADIAPI_RxHdmiGetDeepColorMode(&DeepColorMode);
			raw_mode = initialize_adv(DeepColorMode, iframe_colorspace, tmdsfreq, &video_config_reg);			
						printf("tmdsfreq is : %d\n", tmdsfreq);
			printf("tmdsfreqfrac is : %d\n", tmdsfreqfrac);
			printf("DeepColorMode is : %d\n", DeepColorMode);
			printf("iframe_colorspace is : %d\n", iframe_colorspace);									
			printf("iframevic: %d\n", PktBuf_aviinfo[6]);
			//wait for info frame value stable in chip
			usleep(800000);
#endif
			//read all the info frame and fpga setting
			//init data structure					
			memset(PktBuf_vsinfo, 0, sizeof(PktBuf_vsinfo));
			memset(PktBuf_aviinfo, 0, sizeof(PktBuf_aviinfo));
			memset(PktBuf_spdinfo, 0, sizeof(PktBuf_spdinfo));
			memset(PktBuf_audioinfo, 0, sizeof(PktBuf_audioinfo));
			memset(PktBuf_mpeginfo, 0, sizeof(PktBuf_mpeginfo));
			memset(PktBuf_acp, 0, sizeof(PktBuf_acp));
			memset(PktBuf_isrc1, 0, sizeof(PktBuf_isrc1));
			memset(PktBuf_isrc2, 0, sizeof(PktBuf_isrc2));
			memset(PktBuf_gamut, 0, sizeof(PktBuf_gamut));			
			memset(data_to_send, 0, sizeof(data_to_send));
			
#ifndef SIMPLE_ADV_PACKETS
						ADIAPI_RxHdmiGetPacket(VS_INFO_PACKET, PktBuf_vsinfo, &CsumErr_vsinfo);
						ADIAPI_RxHdmiGetPacket(AVI_INFO_PACKET, PktBuf_aviinfo, &CsumErr_aviinfo);
						ADIAPI_RxHdmiGetPacket(SPD_INFO_PACKET, PktBuf_spdinfo, &CsumErr_spdinfo);
						ADIAPI_RxHdmiGetPacket(AUDIO_INFO_PACKET, PktBuf_audioinfo, &CsumErr_audioinfo);
						ADIAPI_RxHdmiGetPacket(MPEG_INFO_PACKET, PktBuf_mpeginfo, &CsumErr_mpeginfo);
						ADIAPI_RxHdmiGetPacket(ACP_PACKET, PktBuf_acp, &CsumErr_acp);
						ADIAPI_RxHdmiGetPacket(ISRC1_PACKET, PktBuf_isrc1, &CsumErr_isrc1);
						ADIAPI_RxHdmiGetPacket(ISRC2_PACKET, PktBuf_isrc2, &CsumErr_isrc2);
						ADIAPI_RxHdmiGetPacket(GAMUT_PACKET, PktBuf_gamut, &CsumErr_gamut);	
						ADIAPI_RxHdmiGetHsyncInfo(&HsyncInfo);
						ADIAPI_RxHdmiGetVsyncInfo(&VsyncInfo);		
#endif
			//fpga_data = read_fpga();
			read_fpga_data(fpga_data);
			
			data_to_send[0] = tmdsfreq & 0xFF;   //lsb
			data_to_send[1] = tmdsfreq >> 8;   //msb
			data_to_send[2] = tmdsfreqfrac;    // fraction
			// Tx fpga vide config
			data_to_send[3] = fpga_data[1];
			
			// X active
			x = fpga_data[7] << 8 | fpga_data[6];
			if (TRUE == raw_mode) 
				x = x * 2;
			data_to_send[4] = x & 0xFF;
			data_to_send[5] = x >> 8;
			
			//Y Active
			data_to_send[6] = fpga_data[8]; 
			data_to_send[7] = fpga_data[9];

			// X timing params
			p = fpga_data[13] << 24 | fpga_data[12] << 16 | fpga_data[11] << 8 | fpga_data[10];
			x1 = p & 0x07FF;
			if (TRUE == raw_mode) 
				x1 = x1 * 2;
			x2 = p >> 11  & 0x03FF;
			if (TRUE == raw_mode) 
				x2 = x2 * 2;
			x3 = p >> 21 & 0x03FF;
			if (TRUE == raw_mode) 
				x3 = x3 * 2;
			p = x3 << 21 | x2 << 11 | x1;
			data_to_send[8] = p & 0xFF; 
			data_to_send[9] = p >> 8 & 0xFF;   
			data_to_send[10] = p >> 16 & 0xFF;  
			data_to_send[11] = p >> 24;
			
			// Y timing params
			data_to_send[12] = fpga_data[14];    
			data_to_send[13] = fpga_data[15];   
			data_to_send[14] = fpga_data[16];    
			data_to_send[15] = fpga_data[17]; 
		
			// TX fpga frequency
			//pixel_clk_freq = (fpga_data[20] << 16 | fpga_data[19] << 8 | fpga_data[18]) * 2;
			pixel_clk_freq = (fpga_data[20] << 16 | fpga_data[19] << 8 | fpga_data[18]);
			if (TRUE == raw_mode) 
				pixel_clk_freq = (fpga_data[20] << 16 | fpga_data[19] << 8 | fpga_data[18]) * 2;
					
			x1 = pixel_clk_freq / 1000;
			x2 = (pixel_clk_freq % 1000);
		
			data_to_send[16] = x1 & 0xFF;  // LSB decimal	
			data_to_send[17] = x1 >> 8;    // MSB decimal
			data_to_send[18] = x2 & 0xFF;  // LSB fraction
			data_to_send[19] = x2 >> 8;    // MSB FRACTION
						
			for(int i = 0 ; i < 32 ; i++)
			{
				data_to_send[20 + i]       = PktBuf_vsinfo[i];
				data_to_send[20 + i + 32]  = PktBuf_aviinfo[i];
				data_to_send[20 + i + 64]  = PktBuf_spdinfo[i];
				data_to_send[20 + i + 96]  = PktBuf_audioinfo[i];
				data_to_send[20 + i + 128] = PktBuf_mpeginfo[i];
				data_to_send[20 + i + 160] = PktBuf_acp[i];
				data_to_send[20 + i + 192] = PktBuf_isrc1[i];
				data_to_send[20 + i + 224] = PktBuf_isrc2[i];
				data_to_send[20 + i + 256] = PktBuf_gamut[i];
			}
			data_to_send[309] = connection_enable_flag;			
			
			I2C_Read(0x68, 0x00, buffer, 45);
			
			//TX IP
			for(int i = 0 ; i < 4 ; i++)
			{
				data_to_send[310 + i] = buffer[20 + i];
			}
			
			for (int i = 0; i < 6; i++)
			{
				data_to_send[314 + i] = buffer[4 + i];
			}
			data_to_send[320] = video_config_reg;
			data_to_send[321] = DeepColorMode;
			data_to_send[322] = iframe_colorspace;
			
			data_to_send[TX_COM_MSG_MAX_DATA_SIZE - 4] = (g_num_info_frame >> 24) & 0xFF;
			data_to_send[TX_COM_MSG_MAX_DATA_SIZE - 3] = (g_num_info_frame >> 16) & 0xFF;
			data_to_send[TX_COM_MSG_MAX_DATA_SIZE - 2] = (g_num_info_frame >> 8) & 0xFF;
			data_to_send[TX_COM_MSG_MAX_DATA_SIZE - 1] = (g_num_info_frame) & 0xFF;
			
			CRC_Value = crc32b_server(data_to_send);			
			data_to_send[TX_COM_MSG_MAX_DATA_SIZE + 0] = CRC_Value >> 24 & 0xFF;
			data_to_send[TX_COM_MSG_MAX_DATA_SIZE + 1] = CRC_Value >> 16 & 0xFF;
			data_to_send[TX_COM_MSG_MAX_DATA_SIZE + 2] = CRC_Value >> 8 & 0xFF;
			data_to_send[TX_COM_MSG_MAX_DATA_SIZE + 3] = CRC_Value & 0xFF;			
			
			adv_fpga_init_done = 1;			
			//enable FPGA stream
			if(connection_enable_flag == 1)
			{
				//dbg				I2C_Read(0x60, 0x03, avstream_en, 1);
				//dbg				avstream_en[0] |= 0x3;
				//dbg				I2C_Write(0x60, 0x03, avstream_en, 1);
								ioctl(video_led_fd, VIDEO_LED_ON, 0);
				data_valid_10G = 1;				
			}
#ifndef SIMPLE_ADV_PACKETS
			//save adv log
			fp_adv_log = fopen(ADV_LOG_FILE, "w");
			if (NULL == fp_adv_log)
				printf("Cannot open ADV log file.Error!\n");
			else
			{
				fprintf(fp_adv_log, "\nTMDS clock : %d.%dMHz\n", tmdsfreq, tmdsfreqfrac);
				fprintf(fp_adv_log, "Video Horizontal info:\n");
				fprintf(fp_adv_log, "H_LinesTotal : %u\n", HsyncInfo.LinesTotal);
				fprintf(fp_adv_log, "H_LinesActive : %u\n", HsyncInfo.LinesActive);
				fprintf(fp_adv_log, "H_FrontPorch : %u\n", HsyncInfo.FrontPorch);
				fprintf(fp_adv_log, "H_SyncPulse : %u\n", HsyncInfo.SyncPulse);
				fprintf(fp_adv_log, "H_BackPorch : %u\n", HsyncInfo.BackPorch);
				if (HsyncInfo.Polarity == 0)
					fprintf(fp_adv_log, "H_Polarity : Neg\n");
				else
					fprintf(fp_adv_log, "H_Polarity : Pos\n");
			
				fprintf(fp_adv_log, "H_FrontPorch : %u\n", HsyncInfo.FrontPorch);
				fprintf(fp_adv_log, "\nVideo Vertical info:\n");
				fprintf(fp_adv_log, "V_LinesTotal : %u\n", VsyncInfo.LinesTotal);
				fprintf(fp_adv_log, "V_LinesActive : %u\n", VsyncInfo.LinesActive);
				fprintf(fp_adv_log, "V_FrontPorch : %u\n", VsyncInfo.FrontPorch);
				fprintf(fp_adv_log, "V_SyncPulse : %u\n", VsyncInfo.SyncPulse);
				fprintf(fp_adv_log, "V_BackPorch : %u\n", VsyncInfo.BackPorch);
				fprintf(fp_adv_log, "V_F1LinesTotal : %u\n", VsyncInfo.F1LinesTotal);				
				fprintf(fp_adv_log, "V_F1LinesActive : %u\n", VsyncInfo.F1LinesActive);	
				fprintf(fp_adv_log, "V_F1FrontPorch : %u\n", VsyncInfo.F1FrontPorch);	
				fprintf(fp_adv_log, "V_F1SyncPulse: %u\n", VsyncInfo.F1SyncPulse);					
				fprintf(fp_adv_log, "V_F1BackPorch : %u\n", VsyncInfo.F1BackPorch);	
				if (VsyncInfo.Interlaced == 0)
					fprintf(fp_adv_log, "Progressive\n");
				else
					fprintf(fp_adv_log, "interlaced\n");
				if (VsyncInfo.Polarity == 0)
					fprintf(fp_adv_log, "V_Polarity : Neg\n");	
				else
					fprintf(fp_adv_log, "V_Polarity : Pos\n");		
				
				fprintf(fp_adv_log, "\nPacket vs info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_vsinfo[i]);

				fprintf(fp_adv_log, "\nPacket avi info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_aviinfo[i]);				
				
				fprintf(fp_adv_log, "\nPacket spd info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_spdinfo[i]);				
				
				fprintf(fp_adv_log, "\nPacket audio info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_audioinfo[i]);				
				
				fprintf(fp_adv_log, "\nPacket mpeg info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_mpeginfo[i]);

				fprintf(fp_adv_log, "\nPacket acp info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_acp[i]);

				fprintf(fp_adv_log, "\nPacket isrc1 info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_isrc1[i]);

				fprintf(fp_adv_log, "\nPacket isrc2 info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_isrc2[i]);

				fprintf(fp_adv_log, "\nPacket gamut info:\n");
				for (i = 0; i < MAX_HDMI_PACKET_SIZE; i++)
					fprintf(fp_adv_log, "%02x ", PktBuf_gamut[i]);
				
				fclose(fp_adv_log);
			}
#endif
		}
		
		//send igmp message
		if(multicast_ip_change_flag == 1)
		{
			printf("update igmp message.\n");
			/*
			mreq.imr_multiaddr.s_addr = inet_addr(local_multicast_ip);         
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			if (setsockopt(tx_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) 
			{
				perror("setsockopt mreq");		    			
			}
			*/
			close(tx_sock);			
			//strncpy(local_multicast_ip, g_multicast_ip, 15);
			//init socket
			tx_sock = socket(AF_INET, SOCK_DGRAM, 0);
			if (tx_sock < 0) 
			{
				perror("ADV_routine socket create error\n");	
			}
			/*
			unsigned int yes = 1;            //MODIFICATION TO ORIGINAL
			if (setsockopt(tx_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) 
			{
				perror("Reusing ADDR failed");	
			}
			*/
			bzero((char *)&addr, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin_port = htons(INFO_FRAME_PORT);
			addrlen = sizeof(addr);	
	
			mreq.imr_multiaddr.s_addr = inet_addr(g_multicast_ip);         												
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);         		
			if (setsockopt(tx_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) 
			{
				perror("setsockopt mreq");
			}  		
			//Enable FPGA video audio stream			
			if(connection_enable_flag == 1)
			{
				//dbg				I2C_Read(0x60, 0x03, avstream_en, 1);
				//dbg				avstream_en[0] |= 0x3;
				//dbg				I2C_Write(0x60, 0x03, avstream_en, 1);
								ioctl(video_led_fd, VIDEO_LED_ON, 0);	
				data_valid_10G = 1;
				printf("Multicast igmp enable FPGA stream.\n");
			}
			multicast_ip_change_flag = 0;
		}
			
		addr.sin_addr.s_addr = inet_addr(g_multicast_ip);
		cnt = sendto(tx_sock, data_to_send, TX_COM_PACKET_SIZE, 0, (struct sockaddr *) &addr, addrlen);			
		if (cnt < 0) 		
			perror("info frame send error");	
		else {
			if ((g_flag_LOGs & 0x01) != 0) {
				if (((g_num_info_frame % 10) == 0) || (g_num_info_frame <= 10))
					printf("Info frame %d sent successfully.\n", g_num_info_frame);
				if (g_num_info_frame == 10)
					printf("Next every 10th Info frame will be printed.\n");
				if ((g_num_info_frame % 60) == 0) {
					printf("Info Frame (%d) Data:\n", (int)g_num_info_frame);		
					printf("   ");		
					for (int j = 0; j < 16; j++) {
						printf(" %02x", j);		
					}
					printf("\n");
					printf("  |");		
					for (int j = 0; j < 16; j++) {
						printf("---");		
					}
					printf("\n");
					for (int i = 0; i < 32; i++) {
						printf("%02x|", i);		
						for (int j = 0; j < 16; j++) {
							printf(" %02x", data_to_send[i * 16 + j]);		
						}
						printf("\n");		
					}
				}
			}
			
			g_num_info_frame++;
			if (g_num_info_frame >= 1000000)
				g_num_info_frame = 0;
		}
		
		tmdsfreq_prev = tmdsfreq;
		tmdsfreqfrac_prev = tmdsfreqfrac;
		iframe_colorspace_prev = iframe_colorspace;
		pthread_mutex_unlock(&i2c_bus_lock);		
		usleep(1000000);		
	}
	close(video_led_fd);
}

unsigned int crc32b_server(unsigned char *message) {
	int i, j;
	unsigned int byte, crc, mask;

	i = 0;
	crc = 0xFFFFFFFF;
	while (i < TX_COM_MSG_MAX_DATA_SIZE) {
		byte = message[i];              // Get next byte.
		crc = crc ^ byte;
		for (j = 7; j >= 0; j--) {
			// Do eight times.
		mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		i = i + 1;
	}
	return ~crc;
}




