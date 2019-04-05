#
# $Id$
#

include $(RULE_DIR)/Makefile.pre

CODE_SUBDIRS = \
		process

C_FLAGS += $(SHARED_CFLAGS) $(CGI_CFLAGS)

################################################
####   source file 
################################################

# name of all files to be compiled (don't include header files)
SRC += \
		process/libCgiSignal.c \
		process/libCgiUploadFile.c \
		process/libCgiSyscfg.c \
		process/libCgiServices.c \
		libCgiList.c \
		libCgiPages.c \
		libCgiUtils.c \
		libCgiCgi.c \
		libCgiNic.c \


TARGET_LIB:=libCgi
TARGET_LIB_SHARED:=$(TARGET_LIB).so
TARGET_LIB_STATIC:=$(TARGET_LIB).a


LOCAL_ALL:=

include $(ROOT_DIR)/Makefile.post
