
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdint.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#include "muxFpga.h"

/* device : bus, channel and slave address */
static FpgaI2cDevice	_fpgaDevTxSystem =
{
	.bus		= 0,
	.channel	= I2C_CHAN_4_AQLX107GK7,
	.slaveAddress = EXT_FPAG_ADDRESS_SYSTEM,
#if FPGA_DEBUG
	.name = "TxSystem",
#endif

};

static FpgaI2cDevice	_fpgaDevTxPtp =
{
	.bus		= 0,
	.channel	= I2C_CHAN_4_AQLX107GK7,
	.slaveAddress = EXT_FPAG_ADDRESS_PTP,
#if FPGA_DEBUG
	.name = "TxPtp",
#endif
};


static FpgaI2cDevice	_fpgaDevTxNetwork =
{
	.bus		= 0,
	.channel	= I2C_CHAN_4_AQLX107GK7,
	.slaveAddress = EXT_FPAG_ADDRESS_NETWORK,
#if FPGA_DEBUG
	.name = "TxNetwork",
#endif
};


static FpgaI2cDevice	_fpgaDevRxSystem =
{
	.bus		= 0,
	.channel	= I2C_CHAN_4_AQLX107GK7,
	.slaveAddress = EXT_FPAG_ADDRESS_SYSTEM,
#if FPGA_DEBUG
	.name = "RxSysten",
#endif
};

static FpgaI2cDevice	_fpgaDevRxPtp =
{
	.bus		= 0,
	.channel	= I2C_CHAN_4_AQLX107GK7,
	.slaveAddress = EXT_FPAG_ADDRESS_PTP,
#if FPGA_DEBUG
	.name = "RxPtp",
#endif
};


static FpgaI2cDevice	_fpgaDevRxNetwork =
{
	.bus		= 0,
	.channel	= I2C_CHAN_4_AQLX107GK7,
	.slaveAddress = EXT_FPAG_ADDRESS_NETWORK,
#if FPGA_DEBUG
	.name = "RxNetwork",
#endif
};

/*************************** TX definitions ***********************/
static StreamRegisterAddress _txStreamVideo =
{
	.ip = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 88,
#if FPGA_DEBUG
		.name = "txVideoIp",
#endif
	},
	.mac =
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 64,
#if FPGA_DEBUG
		.name = "txVideoMac",
#endif
	},
	.port = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 104,
#if FPGA_DEBUG
		.name = "txVideoPort",
#endif
	},
	.rtpPayload = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 37,
#if FPGA_DEBUG
		.name = "txVideoRtpPayload",
#endif
	}	
};


static StreamRegisterAddress _txStreamAudio =
{
	.ip = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 92,
#if FPGA_DEBUG
		.name = "txAudioIp",
#endif
	},
	.mac =
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 70,
#if FPGA_DEBUG
		.name = "txAudioMac",
#endif
	},
	.port = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 106,
#if FPGA_DEBUG
		.name = "txAudioPort",
#endif
	},
	.rtpPayload = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 38,
#if FPGA_DEBUG
		.name = "txAudioRtpPayload",
#endif
	}	
};


static StreamRegisterAddress _txStreamAnc =
{
	.ip = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 96,
#if FPGA_DEBUG
		.name = "txAncIp",
#endif
	},
	.mac =
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 76,
#if FPGA_DEBUG
		.name = "txAncMac",
#endif
	},
	.port = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 108,
#if FPGA_DEBUG
		.name = "txAncPort",
#endif
	},
	.rtpPayload = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 39,
#if FPGA_DEBUG
		.name = "txAncRtpPayload",
#endif
	}	
};

#if EXT_FPGA_AUX_ON	
static StreamRegisterAddress _txStreamAux =
{
	.ip = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 100,
#if FPGA_DEBUG
		.name = "txAuxIp",
#endif
	},
	.mac =
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 82,
#if FPGA_DEBUG
		.name = "txAuxMac",
#endif
	},
	.port = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 110,
#if FPGA_DEBUG
		.name = "txAuxPort",
#endif
	},
	.rtpPayload = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = INVALIDATE_VALUE_U8,
#if FPGA_DEBUG
		.name = "txAuxRtpPayload",
#endif
	}	
};
#endif

static MediaRegisterAddress _txMediaAddress =
{
	.width = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_WIDTH,
#if FPGA_DEBUG
		.name = "txMediaWidth",
#endif
	},

	.height = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_HEIGHT,
#if FPGA_DEBUG
		.name = "txMediaHeight",
#endif
	},
	
	.framerate = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_FRAMERATE,
#if FPGA_DEBUG
		.name = "txMediaFrameRate",
#endif
	},
	
	.colorSpace = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_SAMPLING,
#if FPGA_DEBUG
		.name = "txMediaColorSpace",
#endif
	},
	
	.vDepth = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_DEPTH,
#if FPGA_DEBUG
		.name = "txMediaVDepth",
#endif
	},
	
	.intl = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_INTLC_SEGM,
#if FPGA_DEBUG
		.name = "txMediaIntlc",
#endif
	},


/* audio */	
	.channels = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_AUDIO_CHANNELS,
#if FPGA_DEBUG
		.name = "txMediaChannel",
#endif
	},
	
	.audioRate = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_AUDIO_RATE,
#if FPGA_DEBUG
		.name = "txMediaAudioRate",
#endif
	},
	
	.pktSize = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_AUDIO_PKT_SIZE,
#if FPGA_DEBUG
		.name = "txMediaPktSize",
#endif
	}
	
};


static FpgaVersionAddress _txVerAddress = 
{
	.version = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_VERSION,
#if FPGA_DEBUG
		.name = "txVersion",
#endif
	},

	.revision = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_REVISION,
#if FPGA_DEBUG
		.name = "txRevision",
#endif
	},
	
	.year = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_YEAR,
#if FPGA_DEBUG
		.name = "txYear",
#endif
	},
	.month = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_MONTH,
#if FPGA_DEBUG
		.name = "txMonth",
#endif
	},
	.day = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_DAY,
#if FPGA_DEBUG
		.name = "txDay",
#endif
	},
	.hour = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_HOUR,
#if FPGA_DEBUG
		.name = "txHour",
#endif
	},
	.minute = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = EXT_FPGA_REG_MINUTE,
#if FPGA_DEBUG
		.name = "txMinute",
#endif
	}
};

static TxRegisterMap _TX_ADDRESS =
{
	.reset = 
	{
		.device = &_fpgaDevTxSystem,
		.offset = 0,
#if FPGA_DEBUG
		.name = "txReset",
#endif
	},
	
	.localMac =
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 46,
#if FPGA_DEBUG
		.name = "txLocalMac",
#endif
	},
	
	.localIp = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 52,
#if FPGA_DEBUG
		.name = "txLocalIp",
#endif
	},

	.localPortV = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 56,
#if FPGA_DEBUG
		.name = "txLocalPortV",
#endif
	},
	
	.localPortA = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 58,
#if FPGA_DEBUG
		.name = "txLocalPortA",
#endif
	},
	
	.localPortAnc =
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 60,
#if FPGA_DEBUG
		.name = "txLocalPortAnc",
#endif
	},
	
#if EXT_FPGA_AUX_ON	
	.localPortAuc = 
	{
		.device = &_fpgaDevTxNetwork,
		.offset = 62,
#if FPGA_DEBUG
		.name = "txLocalPortAuc",
#endif
	},
#endif

	/* dest address and port for every stream */
	.streamVideo = &_txStreamVideo,
	.streamAudio = &_txStreamAudio,
	.streamAnc = &_txStreamAnc,
#if EXT_FPGA_AUX_ON	
	.streamAux = &_txStreamAux,
#endif

	.media = &_txMediaAddress,

	.version = &_txVerAddress,
};


/***************************************  RX  *****************/

static StreamRegisterAddress _rxStreamVideo =
{
	.ip = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 46,
#if FPGA_DEBUG
		.name = "rxVideoIp",
#endif
	},
	.mac =
	{
		.device = &_fpgaDevRxNetwork,
		.offset = INVALIDATE_VALUE_U8,
#if FPGA_DEBUG
		.name = "rxVideoMac",
#endif
	},
	.port = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 62,
#if FPGA_DEBUG
		.name = "rxVideoPort",
#endif
	},
	.rtpPayload = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 114,
#if FPGA_DEBUG
		.name = "rxVideoRtpPayload",
#endif
	}	
};


static StreamRegisterAddress _rxStreamAudio =
{
	.ip = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 49,
#if FPGA_DEBUG
		.name = "rxAudioIp",
#endif
	},
	.mac =
	{
		.device = &_fpgaDevRxNetwork,
		.offset = INVALIDATE_VALUE_U8,
#if FPGA_DEBUG
		.name = "rxAudioMac",
#endif
	},
	.port = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 76,
#if FPGA_DEBUG
		.name = "rxAudioPort",
#endif
	},
	.rtpPayload = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 115,
#if FPGA_DEBUG
		.name = "rxAudioRtpPayload",
#endif
	}	
};


static StreamRegisterAddress _rxStreamAnc =
{
	.ip = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 88,
#if FPGA_DEBUG
		.name = "rxAncIp",
#endif
	},
	.mac =
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 76,
#if FPGA_DEBUG
		.name = "rxAncMac",
#endif
	},
	.port = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 80,
#if FPGA_DEBUG
		.name = "rxAncPort",
#endif
	},
	.rtpPayload = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 116,
#if FPGA_DEBUG
		.name = "rxAncRtpPayload",
#endif
	}	
};

#if EXT_FPGA_AUX_ON	
static StreamRegisterAddress _rxStreamAux =
{
	.ip = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 91,
#if FPGA_DEBUG
		.name = "rxAuxIp",
#endif
	},
	.mac =
	{
		.device = &_fpgaDevRxNetwork,
		.offset = INVALIDATE_VALUE_U8,
#if FPGA_DEBUG
		.name = "rxAuxMac",
#endif
	},
	.port = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 84,
#if FPGA_DEBUG
		.name = "rxAuxPort",
#endif
	},
	.rtpPayload = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 114,
#if FPGA_DEBUG
		.name = "rxAuxRtpPayload",
#endif
	}	
};
#endif


static MediaRegisterAddress _rxMediaAddress =
{
	.width = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_WIDTH,
#if FPGA_DEBUG
		.name = "rxMediaWidth",
#endif
	},

	.height = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_HEIGHT,
#if FPGA_DEBUG
		.name = "rxMediaHeight",
#endif
	},
	
	.framerate = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_FRAMERATE,
#if FPGA_DEBUG
		.name = "rxMediaFrameRate",
#endif
	},
	
	.colorSpace = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_SAMPLING,
#if FPGA_DEBUG
		.name = "rxMediaColorSpace",
#endif
	},
	
	.vDepth = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_DEPTH,
#if FPGA_DEBUG
		.name = "rxMediaVDepth",
#endif
	},
	
	.intl = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_INTLC_SEGM,
#if FPGA_DEBUG
		.name = "rxMediaIntlc",
#endif
	},


/* audio */	
	.channels = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_AUDIO_CHANNELS,
#if FPGA_DEBUG
		.name = "rxMediaChannel",
#endif
	},
	
	.audioRate = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_AUDIO_RATE,
#if FPGA_DEBUG
		.name = "rxMediaAudioRate",
#endif
	},
	
	.pktSize = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_AUDIO_PKT_SIZE,
#if FPGA_DEBUG
		.name = "rxMediaPktSize",
#endif
	}
	
};


static FpgaVersionAddress _rxVerAddress = 
{
	.version = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_VERSION,
#if FPGA_DEBUG
		.name = "rxVersion",
#endif
	},

	.revision = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_REVISION,
#if FPGA_DEBUG
		.name = "rxRevision",
#endif
	},
	
	.year = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_YEAR,
#if FPGA_DEBUG
		.name = "rxYear",
#endif
	},
	.month = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_MONTH,
#if FPGA_DEBUG
		.name = "rxMonth",
#endif
	},
	.day = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_DAY,
#if FPGA_DEBUG
		.name = "rxDay",
#endif
	},
	.hour = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_HOUR,
#if FPGA_DEBUG
		.name = "rxHour",
#endif
	},
	.minute = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = EXT_FPGA_REG_MINUTE,
#if FPGA_DEBUG
		.name = "rxMinute",
#endif
	}
};


static RxRegisterMap _RX_ADDRESS =
{
	.reset = 
	{
		.device = &_fpgaDevRxSystem,
		.offset = 0,
#if FPGA_DEBUG
		.name = "rxReset",
#endif
	},
	
	.localMac =
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 56,
#if FPGA_DEBUG
		.name = "rxLocalMac",
#endif
	},
	
	.localIp = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = 52,
#if FPGA_DEBUG
		.name = "rxLocalIp",
#endif
	},

	.localPortV = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = INVALIDATE_VALUE_U8,
#if FPGA_DEBUG
		.name = "rxLocalPortV",
#endif
	},
	
	.localPortA = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = INVALIDATE_VALUE_U8,
#if FPGA_DEBUG
		.name = "rxLocalPortA",
#endif
	},
	
	.localPortAnc =
	{
		.device = &_fpgaDevRxNetwork,
		.offset = INVALIDATE_VALUE_U8,
#if FPGA_DEBUG
		.name = "rxLocalPortAnc",
#endif
	},
	
#if EXT_FPGA_AUX_ON	
	.localPortAuc = 
	{
		.device = &_fpgaDevRxNetwork,
		.offset = INVALIDATE_VALUE_U8,
#if FPGA_DEBUG
		.name = "rxLocalPortAuc",
#endif
	},
#endif

	/* dest address and port for every stream */
	.streamVideo = &_rxStreamVideo,
	.streamAudio = &_rxStreamAudio,
	.streamAnc = &_rxStreamAnc,
#if EXT_FPGA_AUX_ON	
	.streamAux = &_rxStreamAux,
#endif

	.media = &_rxMediaAddress,

	.version = &_rxVerAddress,
};


FpgaConfig 	_fpgaConfig =
{
	.txAddress = &_TX_ADDRESS,
	.rxAddress = &_RX_ADDRESS,

};


