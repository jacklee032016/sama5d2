#
# $Id$
#

include $(RULE_DIR)/Makefile.pre

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
		cmnMuxPlaylist.c \
		cmnMuxFtpThread.c \
		cmnMuxUtils.c \
		cmnMuxMainConfig.c	\
		cmnMuxPlayerConfig.c	\
		cmnMuxSaveConfig.c \
		cmnMuxDs.c \
		api/cmnMuxBroker.c \
		api/cmnMuxController.c \
		api/cmnMuxDataConn.c \
		api/cmnMuxJsonUtils.c \
		api/cmnMuxJsonHandlers.c \
		api/cmnMuxJsonObjectParse.c \
		api/cmnMuxJsonMedia.c \
		api/cmnMuxCtrl.c \
		api/cmnMuxClient.c \
		cmnMuxConfigWeb.c	\


SRC += \
		$(MUX_SRC) \
		
 
TARGET_LIB:=libMux

TARGET_LIB_SHARED:=$(TARGET_LIB).so
TARGET_LIB_STATIC:=$(TARGET_LIB).a
		 
LOCAL_ALL:=

include $(ROOT_DIR)/Makefile.post

