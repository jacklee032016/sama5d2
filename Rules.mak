# rule for all modules

############## Common for all modules
# support PC environments
# ARCH=
ARCH=arm


CPU_SAMA5D27=YES


# released or debug version, different on debug and log info£¬2007.03.15
# must be 'release' or 'debug'
EDITION=debug
#EDITION=release

ifeq ($(EDITION),release)
	C_FLAGS += -D__EXT_RELEASE__
else	
endif 

ifeq ($(ARCH),arm)
#	C_FLAGS += -D__ARM_CMN__=1 -DARCH_ARM=1  -DARCH_X86=0 -DARCH_X86_32=0 
	# CROSS_COMPILER=arm-linux-gnueabi-
	# LDFLAGS+=  
	# flag=
	# C_FLAGS +=-DARM -DARCH_ARM=1 
	
else
	ARCH=X86
#	C_FLAGS +=-D$(ARCH) -DARCH_X86=1 -DARCH_X86_32=1 -DARCH_ARM=0 
	EXTENSION=
endif


ifeq ($(CPU_SAMA5D27),YES)
	BOARD_NAME=Sama5d2Xpld
	CFLAGS += -DEXTLAB_BOARD=0 -DEXT_LAB
	# LINK_SCRIPT =$(OS_HOME)/src/linkers/q20flash.ld
else
	BOARD_NAME=AN767
	CFLAGS += -DEXTLAB_BOARD=1 -DEXT_LAB
	# LINK_SCRIPT =$(OS_HOME)/src/linkers/q21flash.ld
endif

BIN_DIR=$(ROOT_DIR)/BIN/images.$(BOARD_NAME)
#OBJ_DIR=$(ROOT_DIR)/BIN/objs.$(BOARD_NAME)
OBJ_DIR=objs


ifeq ($(ARCH),X86)
else
	ARCH=arm
endif


export BUILDTIME
export RELEASES_NAME

