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
int	cmnMuxJsonHandle4DownloadMedia(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);

int	cmnMuxJsonHandle4SysAdmin(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data);


/* handlers for data connection */
void cmnMuxDataConnClose(struct DATA_CONN *dataConn);
int cmnMuxDataConnAuthen(struct DATA_CONN *dataConn, char *user, char *pwd);

int cmnMuxDataConnIpCmdOutput(struct DATA_CONN *dataConn);
int cmnMuxDataConnRestOutput(struct DATA_CONN *dataConn);

int cmnMuxDataConnIpCmdInput(struct DATA_CONN *dataConn);
int cmnMuxDataConnRestInput(struct DATA_CONN *dataConn);


#endif

