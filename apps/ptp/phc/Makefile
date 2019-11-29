#
# $Author$
#


include $(ROOT_DIR)/Makefile.pre

CODE_SUBDIRS = \
	.


CFLAGS += $(SHARED_CFLAGS) $(PTP_CFLAGS)

SRC:= \
	psysConfig.c \
	psysPmc.c \
	psysPps.c \
	psysMain.c \
		
	

LOCAL_ALL := \
		phc2sys \
		

include $(ROOT_DIR)/Makefile.post

LDFLAGS := -L$(BIN_DIR)/lib 

ifeq ($(ARCH),arm)
LDFLAGS += $(HI_LIB_PATHS) 
else
LDFLAGS += -ldl
endif

LDFLAGS += -lShared -lPtp4l -lpthread -lrt -lm
#-lCmnSys -lMux -lMuxPtp 

phc2sys:$(OBJS)
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/sbin  

