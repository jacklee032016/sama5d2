# rule for every directory

# ARCH=ARM
# ARCH=X86

# support PC environments
ifndef  ARCH
ARCH=X86
endif

# now it is defined by poky SDK script
# ARCH=arm

ifeq ($(ARCH),X86)
else
	ARCH=arm
endif


BUILD_ROOT_HOME=/home/lzj/yocto/774/buildroot

ifeq ($(EDITION),release)
	C_FLAGS += -D__CMN_RELEASE__
else	
endif 


ifeq ($(ARCH),arm)
	C_FLAGS += -D__ARM_CMN__=1 -DARCH_ARM=1  -DARCH_X86=0 -DARCH_X86_32=0 
	CXXFLAGS = -D__ARM_CMN__=1 -DARCH_ARM=1  -DARCH_X86=0 -DARCH_X86_32=0 
	CROSS_COMPILER=$(BUILD_ROOT_HOME)/output/host/bin/arm-linux-
#	CROSS_COMPILER=$(BUILD_ROOT_HOME)/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-
	LDFLAGS+=  
	flag=
	C_FLAGS +=-DARM
	
else
	ARCH=X86
	C_FLAGS +=-D$(ARCH) -DARCH_X86=1 -DARCH_X86_32=1 -DARCH_ARM=0 -fPIC 
	EXTENSION=
endif

ifndef  CC
CC	= $(CROSS_COMPILER)gcc
endif
ifndef  CXX
CXX 	= $(CROSS_COMPILER)g++ 
endif
ifndef  STRIP
STRIP	= $(CROSS_COMPILER)strip
endif
ifndef  LD
LD	= $(CROSS_COMPILER)ld
endif
ifndef  RANLIB
RANLIB 	= $(CROSS_COMPILER)ranlib
endif
ifndef  AR
AR 	= $(CROSS_COMPILER)ar
endif

ASM = yasm

RM	= rm -r -f
MKDIR	= mkdir -p
MODE	= 700
OWNER	= root
CHOWN	= chown
CHMOD	= chmod
COPY	= cp
MOVE	= mv

LN		= ln -sf

#BUILDTIME := $(shell TZ=UTC date -u "+%Y_%m_%d-%H_%M")
BUILDTIME := $(shell TZ=CN date -u "+%Y_%m_%d")
GCC_VERSION := $(shell $(CC) -dumpversion )

BIN_DIR=$(ROOT_DIR)/Linux.bin.$(ARCH)
OBJ_DIR=Linux.obj.$(ARCH)

CGI_BIN_DIR=$(BIN_DIR)/var/www/apis


SHARED_HOME := $(ROOT_DIR)/shared


ifeq ($(PLUGINS),YES)
	CFLAGS += -DPLUGIN_SUPPORT
endif

CC_CFLAGS+= -Wno-deprecated

INPUT_FILE=$<
FILE_NAME=$(shell basename $(INPUT_FILE))

CFLAGS += -D__FILENAME__='"$(FILE_NAME)"' -DROOT_DIR='"$(ROOT_DIR)"' -I$(ROOT_DIR) $(INCLUDE_DIR) -I. -I./ \
			-fdata-sections -ffunction-sections 

# -fdata-sections -ffunction-sections : remove not used symbole when linking, but only usable in X86 static library, not ARM shared library
# 05.14, 2019


SHARED_CFLAGS += -I$(SHARED_HOME)/include -I$(SHARED_HOME)/src/sys/include
SHARED_LDFLAGS += -L$(SHARED_HOME)/Linux.bin.$(ARCH)/lib -lMux -lShared 

CGI_CFLAGS += -I$(SHARED_HOME)/libcgi/include
CGI_LDFLAGS += -lCgi

#
MISC_CFLAGS += -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE \
			-D_XOPEN_SOURCE=600 -DZLIB_CONST \
			-DHAVE_AV_CONFIG_H -std=c11 -fomit-frame-pointer -pthread -D_REENTRANT  -I$(FFMPEG_HOME)\

#MISC_CFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE \
#			-DZLIB_CONST \
#			-DHAVE_AV_CONFIG_H -std=c11 -fomit-frame-pointer -pthread -D_REENTRANT  -I$(FFMPEG_HOME)\

MISC_CFLAGS += -g -Wno-error -Wno-error=deprecated-declarations -Wdeprecated-declarations -Wno-deprecated  \
			-Wno-strict-overflow	-fwrapv \
			-Wdeclaration-after-statement -Wall -Wdisabled-optimization -Wpointer-arith \
			-Wredundant-decls -Wwrite-strings -Wtype-limits -Wundef -Wmissing-prototypes \
			-Wno-pointer-to-int-cast -Wstrict-prototypes -Wempty-body -Wno-parentheses \
			-Wno-switch -Wno-format-zero-length -Wno-pointer-sign \
			-O3 -fno-math-errno -fno-signed-zeros -fno-tree-vectorize -Werror=format-security \
			-Werror=return-type \
			-Werror=vla -Wformat -fdiagnostics-color=auto -Wno-maybe-uninitialized \

# -Wno-error : disable all warns as error
#-MMD -MF -MT\
#			 -Werror=missing-prototypes -Werror=implicit-function-declaration

# not support by ARM toolchains
# -Wno-unused-const-variable 


ifeq ($(ARCH),arm)

	#CFG_HI_CFLAGS := -Werror -Wall
	CFG_HI_CFLAGS := -Wall
	
	CFG_HI_CFLAGS+= -D_GNU_SOURCE -Wall -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections
	CFG_HI_CFLAGS+= -DCHIP_TYPE_$(CFG_HI_CHIP_TYPE) -DCFG_HI_SDK_VERSION=$(CFG_HI_SDK_VERSION)
	CFG_HI_CFLAGS+= $(CFG_HI_BOARD_CONFIGS)
	
	
	
	SYS_LIBS := -lpthread -lrt -lm -ldl -lstdc++


	HI_DEPEND_LIBS := -Wl,--start-group $(SYS_LIBS) $(DEPEND_LIBS) -Wl,--end-group
	
	TARGETS :=
	

else 
	ASM_FLAGS = -f elf  -g dwarf2 -I./ -I.// -I$(ROOT_DIR) -Pconfig.asm 

endif

PTP_CFLAGS+= -Wall -DVER=2.0 -D_GNU_SOURCE -DHAVE_CLOCK_ADJTIME -DHAVE_POSIX_SPAWN -DHAVE_ONESTEP_SYNC -I../include -I../mux
PTP_CFLAGS+= 

LDFLAGS := $(LDFLAGS) -L$(BIN_DIR)/lib -Wl,--gc-sections \
