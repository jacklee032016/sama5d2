#
# $Id$
#

include $(ROOT_DIR)/Makefile.pre

CODE_SUBDIRS = \
			api


C_FLAGS += $(SHARED_CFLAGS)

ifeq ($(ARCH),arm)
else
endif

################################################
####   source file 
################################################


MUX_SRC += \
		sdpMain.c \
		sdpReceiver.c \
		sdpFsm.c \
		sdpParse.c \
		cmnMuxUtils.c \
		cmnMuxMainConfig.c	\
		cmnMuxSaveConfig.c \
		api/cmnMuxBroker.c \
		api/cmnMuxManager.c \
		api/cmnMuxDataConn.c \
		api/cmnMuxJsonCmds.c \
		api/cmnMuxJsonUtils.c \
		api/cmnMuxJsonHandlers.c \
		api/cmnMuxJsonObjectParse.c \
		api/cmnMuxCtrl.c \
		api/cmnMuxClient.c \
		extThreadBtn.c \
		cmnMuxThreadClocks.c \

#		cmnMuxDs.c \
#		cmnMuxConfigWeb.c	\
#		cmnMuxPlayerConfig.c	\
#		cmnMuxPlaylist.c \


SRC += \
		$(MUX_SRC) \
		
 
TARGET_LIB:=libMux

TARGET_LIB_SHARED:=$(TARGET_LIB).so
TARGET_LIB_STATIC:=$(TARGET_LIB).a
		 
LOCAL_ALL:=

include $(ROOT_DIR)/Makefile.post

