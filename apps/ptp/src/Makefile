#
# $Id$
#

include $(ROOT_DIR)/Makefile.pre

CODE_SUBDIRS = \
					.


C_FLAGS += $(SHARED_CFLAGS) -Wall -DVER=2.0 -D_GNU_SOURCE -DHAVE_CLOCK_ADJTIME -DHAVE_POSIX_SPAWN -DHAVE_ONESTEP_SYNC -I../include 


################################################
####   source file 
################################################
PORTABL_SRC += \

SRC += \
			util.c \
			sk.c \
			rtnl.c \
			ptpMgtClientCmn.c \
			tlv.c \
			tc.c \
			msg.c \
			config.c \
			clockcheck.c \
			clockadj.c \
			servo.c \
			servoLinreg.c \
			servoPi.c \
			servoNtpShm.c \
			ptpPort.c \
			ptpPortInit.c \
			ptpPortStates.c \
			ptpPortMgmt.c \
			ptpPortMsgMgmt.c \
			ptpPortMsgOrdinary.c \
			ptpPortMsgTransp.c \
			ptpPortDevOrdinary.c \
			ptpPortDevTransp.c \
			unicast_service.c \
			unicast_client.c \
			ptpClock.c \
			ptpClockInit.c \
			ptpClockMgmt.c \
			ptpClockPoll.c \
			ptpClockSubscription.c \
			ptpTransport.c \
			ptpTransRaw.c \
			ptpTransUdp.c \
			ptpTransUdp6.c \
			ptpTransUds.c \
			ptpUtils.c \
			ptpSysDeps.c \
			ptpTimestampProc.c \
			bmc.c \
		
 
TARGET_LIB:=libPtp4l

TARGET_LIB_SHARED:=$(TARGET_LIB).so
TARGET_LIB_STATIC:=$(TARGET_LIB).a
		 
LOCAL_ALL:=

include $(ROOT_DIR)/Makefile.post

