#
#

include $(RULE_DIR)/Makefile.pre

CODE_SUBDIRS = .

C_FLAGS += $(SHARED_CFLAGS) $(CFLAGS_DRIVER)

# name of all files to be compiled (don't include header files)
SRC += \
		testMip.c \
		testSecure.c \
		testMain.c \
		evtest.c \



CFLAGS += -fno-strict-aliasing

LDFLAGS := $(LDFLAGS) $(SHARED_LDFLAGS) -L$(HI_SHARED_LIB_DIR) -lrt -lm -ldl -lz -lpthread

TESTS = \
			testMux \
			evtest

		 
LOCAL_ALL:=$(TESTS) 

include $(ROOT_DIR)/Makefile.post


testMux:$(OBJ_DIR)/testMip.o $(OBJ_DIR)/testMain.o $(OBJ_DIR)/testSecure.o 
		$(CC) -o $@ $^ -lCmnSys $(LDFLAGS)
		$(STRIP) $@
		$(MOVE) $@ $(BIN_DIR)/usr/bin


evtest:$(OBJ_DIR)/evtest.o
		$(CC) -o $@ $^ -lCmnSys $(LDFLAGS)
		$(STRIP) $@
		$(MOVE) $@ $(BIN_DIR)/usr/bin

