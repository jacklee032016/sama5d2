"""
Tests settings module
"""

DEBUG = True
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

"""
 following definition is for communication with C service
"""

REQUEST_METHOD_GET="GET"
REQUEST_METHOD_POST="POST"

SERVICE_URI_ROOT="/"
SERVICE_URI_SYSTEM="/system"
SERVICE_URI_RS232="/rs232"
SERVICE_URI_OETHERS="/others"
SERVICE_URI_SECURITYS="/security"

SERVICE_URI_VIDEO="/video"
SERVICE_URI_AUDIO="/audio"
SERVICE_URI_ANC="/anc"
SERVICE_URI_SDP_CONFIG="/sdp"

SERVICE_URI_SDP_VIDEO="/video/sdp"
SERVICE_URI_SDP_AUDIO="/audio/sdp"
SERVICE_URI_SDP_ANC="/anc/sdp"

SERVICE_DATA_FIELD_USERNAME="username"
SERVICE_DATA_FIELD_PASSWORD="passwd"
SERVICE_DATA_FIELD_DATA="data"
SERVICE_DATA_FIELD_METHOD="method"
SERVICE_DATA_FIELD_URI="uri"

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
