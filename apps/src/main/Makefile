#
# $Id: Makefile,v 1.1.1.1 2006/11/29 18:13:58 lizhijie Exp $
#


include $(ROOT_DIR)/Makefile.pre


CODE_SUBDIRS += \
			sys
		

CFLAGS += $(SHARED_CFLAGS) 
################################################
####   source file 
################################################


ifeq ($(ARCH),arm)
	SRC += \
	
else
	SRC += \

endif

SYS_SRC += \
		sys/extSysInit.c \

		
SRC += \
		$(SYS_SRC) \
		muxMain.c		\


LOCAL_ALL=muxMgr 

include $(ROOT_DIR)/Makefile.post

LDFLAGS := -L$(BIN_DIR) -L$(BIN_DIR)/lib \
		$(LDFLAGS)

          

ifeq ($(ARCH),arm)
else
endif

LIBS += -lMuxPtp -lPtp4l -lm -lpthread -lrt -ldl 

# -Wl,-verbose -Wl,-static  -s 
$(LOCAL_ALL):$(OBJS)
		$(CC) -s -o $@ $(OBJS) -L$(ROOT_DIR)/Linux.bin.$(ARCH)/lib -lz -L$(HI_SHARED_LIB_DIR) -L$(HI_EXTERN_LIB_DIR) $(LIB_LDFLAGS) -lMux -lCmnSys $(SHARED_LDFLAGS) $(LIBS) 
		$(MOVE) $@ $(BIN_DIR)/sbin
