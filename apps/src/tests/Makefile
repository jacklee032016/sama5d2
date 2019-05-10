#
#

include $(RULE_DIR)/Makefile.pre

CODE_SUBDIRS = .

C_FLAGS += $(SHARED_CFLAGS) $(CGI_CFLAGS)

# name of all files to be compiled (don't include header files)
SRC += \
		testMip.c \

#		restMuxInit.c \



CFLAGS += -fno-strict-aliasing

LDFLAGS := $(LDFLAGS) $(SHARED_LDFLAGS) -L$(HI_SHARED_LIB_DIR) -lrt -lm -ldl -lz -lpthread

TESTS = \
			testMip

		 
LOCAL_ALL:=$(TESTS) 

include $(ROOT_DIR)/Makefile.post


testMip:$(OBJ_DIR)/testMip.o  
		$(CC) -o $@ $^ -lCmnSys $(LDFLAGS)
		$(STRIP) $@
		$(MOVE) $@ $(BIN_DIR)/usr/bin

