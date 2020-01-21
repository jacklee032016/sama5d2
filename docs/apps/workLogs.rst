Work logs for Application Development
#########################################


12.18, 2019
---------------------------
* PTP UTC 37 seconds to suit the requirement of PTP server in Radio Canada;
* Poll PTP state and offset, and update FPGA register differently when PTP is locked or not;
* APIs updates:
   * show PTP offset (nanosecond);
   * show and modify TTL field for audio/video streams
   * Need FPGA support to update to the streams;
* TTL field of SDP will change as the API update it;
* Test PTP offset in every 2 seconds for 5 days:
   * stable offset is smaller than +-50 us;
   

12.06, 2019
---------------------------
* web pages update;
* RTP Payload Type of video/audio streams can be configured to FPGA by API in TX;
* 


10.31, 2019
---------------------------


10.29, 2019
---------------------------
#. Factory Default: 
    * blink after 6 seconds;
	* Restore default parameters;
#. increase log level from DEBUG to WARNING to minimize log messages
#. RTC message comments: modify kernel image:
    * kernel version: Oct 29 13:21:15 EDT 2019
#. SDP:
    * check media and create SESSION ID;
	* Only updaye SESSION ID only media changed;
	* 'interlaced' when video is interlace;
#. Crash flash when modifying MAC address
#. Read PTP status and reply with IP Command and REST API
#. control on PTP daemon:
    * rename PTP daemon as muxPtpd;
	* start PTP daemon with parameters of "domain number";
	* start and stop PTP daemon dynamically;
#. New configuration of FPGA:
    * Add "ColorSpaceConvert" to convert all color space tp YCrCb422 and 10 bits depth;
	* Add "FPS Configuration" with 4 options of "1|2|Double|Split";
	* Add "AudioInput" and "AudioPath" configuration for audio in TX;
#. Audio Channels fix to 8, no matter what read from HDMI chip;
#. remove code about IR;
#. After blink, LED should be in state of ON;
#. Default TX/RX IP: 66/67 if DHCP server is not available when DHCP is configured


05.10, 2019
---------------------------
* Finish multicast group join and leave;
* Add code of FPGA from 767;

  
05.02, 2019, todo list
---------------------------
Task can be done in X86 and before hardare configuratoin:

* test all interfaces: IP command and REST;
* optimize code of protocol;
* update JSON object when it start up or update;
* different operations based on TX/RX and board type;
* actions and reply data after data received from IP command and REST;
   * FPGA configuratoin on X86 platform, just virtualize the I2C operations;
   * Multicast address: join and leave groups;
* bash scripts for tests of IP command and REST;
