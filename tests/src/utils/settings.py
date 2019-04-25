"""
Tests settings module
"""

DEBUG = False
TEST_DEBUG = False
TESTING = True

FIRMWARE_TYPE_TFTP_OS="firmOs"
FIRMWARE_TYPE_TFTP_FPGA="firmFpga"


FIRMWARE_BIN_OS='../../an767/BIN/images.AN767/rtosLwipAN767.bin.bin'

FIRMWARE_BIN_FPGA_RX='/media/sf_rtos/Binary/top_Angelica_N_767_rx1.bin'
FIRMWARE_BIN_FPGA_TX='/media/sf_rtos/Binary/top_Angelica_N_767_rx2.bin'

IP_CMD_FIND="get_param"
IP_CMD_SET_PARAM = "set_param"
IP_CMD_RS232_DATA= "send_data_rs232"
IP_CMD_SECURE="security_check"

# seconds
IP_CMD_FIND_TIMEOUT=5

FIRMWARE_URL_OS="/mcuUpdate"
FIRMWARE_URL_FPGA="/fpgaUpdate"

STATIC_PAGES=[
    "/upgradeFpga.html",
    "/upgradeMcu.html",
    "/styles.css",
    "/load_html.js",
    "/logo.jpg",
    "/header_bg.png",
    "/loading.gif",
    "/404.html"
]

DYNAMIC_PAGES=[
    "/",
    "/info",
    "/media",
    "/",
    "/info",
    "/media",
    "/",
    "/info",
    "/media"
]

DYNAMIC_PAGES2=[
    "/mcuUpdate",
    "/fpgaUpdate",
    "/reboot"
]

SIMULATOR_GATEWAT="192.168.166.1"

FIELD_SYSTEM_PRODUCT="pName"
FIELD_SYSTEM_MODEL="model"
FIELD_SYSTEM_NAME="cName"
FIELD_SYSTEM_IP="ip"
FIELD_SYSTEM_MAC="mac"
FIELD_SYSTEM_DHCP="isDhcp"
FIELD_SYSTEM_DIP_BUTTON="isDipOn"

FIELD_VIDEO_IP="ip"
FIELD_VIDEO_PORT="port"
FIELD_VIDEO_SDP="sdp"
FIELD_VIDEO_HEIGHT="height"
FIELD_VIDEO_WIDTH="width"
FIELD_VIDEO_DEPTH="depth"
FIELD_VIDEO_FPS="fps"
FIELD_VIDEO_COLOR_SPACE="colorSpace"
FIELD_VIDEO_INTLCE="Intlce"

FIELD_AUDIO_IP="ip"
FIELD_AUDIO_PORT="port"
FIELD_AUDIO_SDP="sdp"
FIELD_AUDIO_SAMPLE="sample"
FIELD_AUDIO_CHANNEL="channel"
FIELD_AUDIO_DEPTH="depth"
FIELD_VIDEO_PKT_SIZE="pktsize"

FIELD_ANC_IP="ip"
FIELD_ANC_PORT="port"
FIELD_ANC_SDP="sdp"

FIELD_RS232_BAUDRATE="baudrate"
FIELD_RS232_DATABIT="databit"
FIELD_RS232_PARITY="parity"
FIELD_RS232_STOPBIT="stopbit"
FIELD_RS232_DATA="data"

FIELD_SECURITY_ID="get_id"
FIELD_SECURITY_STATUS="get_status"
FIELD_SECURITY_KEY="set_key"

FIELD_SDP_MEDIA="media"
FIELD_SDP_IP="ip"
FIELD_SDP_PORT="port"
FIELD_SDP_URI="uri"
