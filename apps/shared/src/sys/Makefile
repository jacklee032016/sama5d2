#
# $Id$
#

include $(ROOT_DIR)/Makefile.pre

CODE_SUBDIRS = \
				.


C_FLAGS += $(SHARED_CFLAGS)

ifeq ($(ARCH),arm)
else
endif

################################################
####   source file 
################################################


MUX_SRC += \
		cmnSysI2c.c \
		cmnSysI2cCtrl.c \
		cmnSysRawSpi.c \
		cmnSysSpi.c \
		cmnSysNet.c \
		cmnSysLeds.c \
		cmnSysCtrlDeps.c \
		cmnSysRs232.c \
		cmnSysFpgaDefs.c \
		cmnSysFpgaInit.c \
		cmnSysFpgaOps.c \
		cmnSysConfig.c \
		cmnSysJson.c \
		cmnSysNetlinkW1.c \
		cmnSysSecureChip.c \
		sha256.c \
		


SRC += \
		$(MUX_SRC) \
		
 
TARGET_LIB:=libCmnSys

TARGET_LIB_SHARED:=$(TARGET_LIB).so
TARGET_LIB_STATIC:=$(TARGET_LIB).a
		 
LOCAL_ALL:=

include $(ROOT_DIR)/Makefile.post

