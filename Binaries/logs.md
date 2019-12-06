# Work logs for Application Development


# 12.06, 2019: apps_install_7.3.0_arm_debug_2019_12_06.zip

* payloadType of audio/video: 
   * default 96/97;
   * modifiable with APIs;
   * write to FPGA(TX and RX);

* PTP configuration
   * modify parameters of PTP dynamically;
   * save parameters after reboot;
   * debug the problem of no response of PTP;
   * debug info of PTP;

* Factory default(RESET)
   * Independent process to poll button, guaranteeing it is always usable;
   * save current install package in an independent flash partition;
   * restore all (configurations and programs) into factory default, except MAC address;
   * remove the existed configuration data;
   * Led POWER restore to ON after it blinks;
   * debug the problem of different configuration about DHCP in system and APIs;
   * after RESET, system can begin work only after reboot twice;

* other bugs

* Integration of web pages
