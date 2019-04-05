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
		

int cmnMuxControllerAddEvent(char *cmd, 	int method, void *dataConn);


int cmnMuxCtrlDataHandle( struct DATA_CONN *dataConn );

cJSON *cmnMuxJsonLoadConfiguration(char *cfgFileName);

int	cmnMuxJsonHandle4Plugin(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);
int	cmnMuxJsonHandle4SetMedia(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);
int	cmnMuxJsonHandle4GetMedia(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);
int	cmnMuxJsonHandle4DownloadMedia(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);

int	cmnMuxJsonHandle4SysAdmin(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);

int cmnMuxCtrlResponse(struct DATA_CONN *dataConn, void *buf, int size);



#endif

