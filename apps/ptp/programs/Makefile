#
# $Author$
#


include $(ROOT_DIR)/Makefile.pre

CODE_SUBDIRS = \
	.


CFLAGS += -Wall -DVER=2.0 -D_GNU_SOURCE -DHAVE_CLOCK_ADJTIME -DHAVE_POSIX_SPAWN -DHAVE_ONESTEP_SYNC -I../include

SRC:= \

	

LOCAL_ALL := \
		ptp4l \
		hwstamp_ctl \
		nsm \
		phc_ctl \
		pmc \
		timemaster \
		

include $(ROOT_DIR)/Makefile.post

LDFLAGS := -L$(BIN_DIR)/lib 

ifeq ($(ARCH),arm)
LDFLAGS += $(HI_LIB_PATHS) 
else
LDFLAGS += -ldl
endif

LDFLAGS += -lPtp4l -lpthread -lrt -lm


ptp4l:$(OBJ_DIR)/ptp4l.o 
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS) 
	$(MOVE) $@ $(BIN_DIR)/sbin  

hwstamp_ctl:$(OBJ_DIR)/hwstamp_ctl.o
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

nsm:$(OBJ_DIR)/nsm.o
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/bin  

phc2sys:$(OBJ_DIR)/phc2sys.o
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

phc_ctl:$(OBJ_DIR)/phc_ctl.o
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

pmc:$(OBJ_DIR)/pmc.o
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

timemaster:$(OBJ_DIR)/timemaster.o $(OBJS)
	$(CC) -s -Wl,--gc-sections -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

