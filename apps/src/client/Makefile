#
# $Author$
#


include $(ROOT_DIR)/Makefile.pre

C_FLAGS += $(SHARED_CFLAGS) 

SRC:= \
		apiClient.c \
		


CODE_SUBDIRS = .

LOCAL_ALL := \
		apiClient \
		scKey \
		regRead \
		fwUpdate \
		ptpc \
		sfpCtrl \
		btnCtrl \

NOT_USED := \
		hello \
		

include $(ROOT_DIR)/Makefile.post


ifeq ($(ARCH),arm)
LDFLAGS += 
else
LDFLAGS += -LLinux.bin.$(ARCH)/lib -ldl
endif

LDFLAGS += -L$(BIN_DIR)/lib -lCmnSys -lMux $(LDFLAGS) -lMuxPtp -lPtp4l -lShared -lz -lpthread -lm


apiClient:$(OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

scKey:$(OBJ_DIR)/scKey.o
	$(CC) -s -o $@ $^ $(LDFLAGS) 
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

regRead:$(OBJ_DIR)/regRead.o
	$(CC) -s -o $@ $^ $(LDFLAGS) 
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

fwUpdate:$(OBJ_DIR)/flashUpdate.o
	$(CC) -s -o $@ $^ $(LDFLAGS) 
	$(MOVE) $@ $(BIN_DIR)/usr/bin  

ptpc:$(OBJ_DIR)/ptpc.o
	$(CC) -s -o $@ $^ $(LDFLAGS) -lCmnSys -lMux
	$(MOVE) $@ $(BIN_DIR)/usr/bin  
	
sfpCtrl:$(OBJ_DIR)/sfpCtrl.o
	$(CC) -s -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  
	
btnCtrl:$(OBJ_DIR)/btnCtrl.o
	$(CC) -s -o $@ $^ $(LDFLAGS)
	$(MOVE) $@ $(BIN_DIR)/usr/bin  
	
hello:$(OBJ_DIR)/hello.o
	$(CXX) -s -o $@ $^ 
	$(MOVE) $@ $(BIN_DIR)/usr/bin  
