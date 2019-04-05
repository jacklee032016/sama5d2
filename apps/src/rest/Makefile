#
#

include $(RULE_DIR)/Makefile.pre

CODE_SUBDIRS = .

C_FLAGS += $(SHARED_CFLAGS) $(CGI_CFLAGS)

# name of all files to be compiled (don't include header files)
SRC += \
		restMain.c \
		restMuxInit.c \


#		restSvrRuntime.c \


CFLAGS += -fno-strict-aliasing

LDFLAGS := -lCgi $(LDFLAGS) $(SHARED_LDFLAGS) -L$(HI_SHARED_LIB_DIR) -lrt -lm -ldl -lz -lpthread

CGIS = \
				muxApis

		 
LOCAL_ALL:=$(CGIS) 

include $(ROOT_DIR)/Makefile.post


muxApis:$(OBJS)  
		$(CC) -o $@ $^ $(LDFLAGS) -lCmnSys
		$(STRIP) $@
		$(MOVE) $@ $(CGI_BIN_DIR)/ 

