/*
* Local header for libMux
*/

#ifndef	___CMN_MUX_H__
#define	___CMN_MUX_H__


#define	CLIENT_TYPE_IS_IPCMD(type) \
		((type) == CTRL_LINK_TCP || (type) == CTRL_LINK_UDP )


#define	CONN_IS_IPCMD(ctrlConn)	\
		CLIENT_TYPE_IS_IPCMD((ctrlConn)->type)


#define	DATA_CONN_IS_IPCMD(dataConn)	\
		CONN_IS_IPCMD((dataConn)->ctrlConn )

#define	REPLY_DATACONN_OK( dataConn, resultObj)	\
	do{	(dataConn)->resultObject = (resultObj); 	\
		if( (dataConn)->resultObject == NULL){DATA_CONN_ERR(dataConn, IPCMD_ERR_SERVER_INTERNEL_ERROR, "No data is found for URI: '%s'", dataConn->cmd );} \
		else {(dataConn)->errCode = IPCMD_ERR_NOERROR; } \
			}while(0)


//MUX_DEBUG_JSON_OBJ((dataConn)->resultObject); 
		

int cmnMuxControllerAddEvent(char *cmd, 	int method, void *dataConn);


int cmnMuxCtrlDataHandle( struct DATA_CONN *dataConn );


int	cmnMuxJsonHandle4Plugin(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);
int	cmnMuxJsonHandle4DownloadMedia(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);

int	cmnMuxJsonHandle4SysAdmin(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);


/* handlers for data connection */
void cmnMuxDataConnClose(struct DATA_CONN *dataConn);
int cmnMuxDataConnAuthen(struct DATA_CONN *dataConn, char *user, char *pwd);

int cmnMuxDataConnIpCmdOutput(struct DATA_CONN *dataConn);
int cmnMuxDataConnRestOutput(struct DATA_CONN *dataConn);

int cmnMuxDataConnIpCmdInput(struct DATA_CONN *dataConn);
int cmnMuxDataConnRestInput(struct DATA_CONN *dataConn);



void cmnMuxClearConfig(EXT_RUNTIME_CFG *rxCfg);
int	cmnMuxObjectParseSystem(struct DATA_CONN *dataConn, cJSON *dataObj);
int	cmnMuxObjectParseVideo(struct DATA_CONN *dataConn, cJSON *dataObj);
int	cmnMuxObjectParseAudio(struct DATA_CONN *dataConn, cJSON *dataObj);
int	cmnMuxObjectParseAnc(struct DATA_CONN *dataConn, cJSON *dataObj);
int	cmnMuxObjectParseSdp(struct DATA_CONN *dataConn, cJSON *dataObj);
int	cmnMuxObjectParseRs232(struct DATA_CONN *dataConn, cJSON *dataObj);
int cmnMuxObjectParseSecurity(struct DATA_CONN *dataConn, cJSON *dataObj);
int cmnMuxObjectParseOthers(struct DATA_CONN *dataConn, cJSON *dataObj);



void cmnMuxCfgDebugData(EXT_RUNTIME_CFG *cfg);
int cmnMuxSystemConfig(MuxMain *muxMain);


#define	MUX_DEBUG_BROKER			EXT_DBG_ON

#endif

