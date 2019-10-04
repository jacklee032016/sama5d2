#
# $Author$
#


include $(ROOT_DIR)/Makefile.pre

CODE_SUBDIRS = \
	.


CFLAGS += -Wall -DVER=2.0 -D_GNU_SOURCE -DHAVE_CLOCK_ADJTIME -DHAVE_POSIX_SPAWN -DHAVE_ONESTEP_SYNC -I../include

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

LDFLAGS += -lPtp4l -lpthread -lrt -lm


phc2sys:$(OBJS)
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/sbin  

