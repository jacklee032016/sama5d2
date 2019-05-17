#
# $Author$
#


include $(RULE_DIR)/Makefile.pre

C_FLAGS += $(SHARED_CFLAGS) -DUSE_HOSTCC

SRC:= \
		crc32.c  fw_env.c  fw_env_main.c \
		


CODE_SUBDIRS = .

LOCAL_ALL := \
		fw \
		

include $(ROOT_DIR)/Makefile.post


ifeq ($(ARCH),arm)
LDFLAGS += 
else
LDFLAGS += -LLinux.bin.$(ARCH)/lib -ldl
endif

LDFLAGS += -L$(BIN_DIR)/lib $(LDFLAGS) -lz -lpthread -lm


fw:$(OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

