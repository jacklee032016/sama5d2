/*
 * decoder.h
 *
 *  Created on: Mar 30, 2017
 *      Author: peter
 */

#ifndef _MUX_PLAYER_H_
#define _MUX_PLAYER_H_

// #include "hi_audio_codec.h"
// #include "hi_unf_common.h"
// #include "hi_unf_avplay.h"
// #include "hi_unf_demux.h"
// #include "hi_unf_so.h"
// #include "hi_unf_sound.h"
// #include "hi_unf_disp.h"
// #include "hi_unf_vo.h"
// #include "hi_go.h"
// #include "hi_adp_mpi.h"
// #include "hi_adp_hdmi.h"
// #include "hi_svr_player.h"
// #include "hi_svr_format.h"
// #include "hi_svr_metadata.h"
// #include "hi_svr_assa.h"
// #include "hi_unf_edid.h"

#define DEFAULT_NETWORK	"eth0"

#define NUM_CMD	(60)
#define KEY_LEN	(20)
#define VALUE_LEN (1024)

#define NUM_SET_PARAMETERS (6)
#define KEY_SET_PARAM_LEN	(45)


typedef enum _PROTOCOL {
	STREAM_PROTOCOL_BEGIN, MCAST, RTSP, RTMP, TS, HLS, FLV, MMS, STREAM_PROTOCOL_END
} STREAM_PROTOCOL;


typedef struct _HASH_MAP{
	const char key[KEY_LEN];
	char value[VALUE_LEN];
}HASH_MAP;

typedef struct _SET_PARAM {
	const char key[KEY_SET_PARAM_LEN];
}SET_PARAM;


#define MAX_URL_LEN	(1024)
#define IP_LEN	(15)
#define MUX_STREAM_NUM	(2)

// typedef struct _SYSTEM_STATE {
// 	HI_SVR_PLAYER_STATE_E play_state;
// //	STREAM_PROTOCOL protocol;
// 	HI_CHAR url[MAX_URL_LEN];
// //	HI_CHAR  port[5];
// //	HI_CHAR ip[IP_LEN];
// 	HI_CHAR VidResI[256];
// 	HI_U32 VidFpsI;
// 	HI_CHAR VidResO[256];
// 	HI_U32 VidFpsO;
// 	HI_U32 VidBitR;
// 	HI_FORMAT_VIDEO_TYPE_E VidCodec;
// 	HI_U32 AudFrq;
// 	HI_U32 AudBitR;
// 	HI_FORMAT_AUDIO_TYPE_E AudCodec;
// 	HI_BOOL hls_fast_start;
// 	HI_U32 lowDelay;
// 	HI_BOOL charset_support;
// 	HI_UNF_SYNC_REF_E sync_mode;
// 	HI_BOOL isRTSPph;
// 	HI_BOOL isRTMPph;
// } SYSTEM_STATE;
// 
// SYSTEM_STATE decoder_state[MUX_STREAM_NUM];

// int change_low_level_parameters(const char *key, char*value);

#endif /* SAMPLE_LOCALPLAY_MUX_PLAYER_H_ */
