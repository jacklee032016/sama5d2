#ifeq ($(PARAM_FILE), )
#        PARAM_FILE:=../../Makefile.param
#        include $(PARAM_FILE)
#endif

include 	$(PWD)/../drv.mak

# this is path of kernel source, $(PWD) is system environment variable
# PWD    := $(shell pwd)

EXTRA_CFLAGS=-I$(PWD)/../include

obj-m := w1Ds28e15.o

# w1Ds28e15-objs := muxSecurityChip.o sha256_software.o

default:
	@make -C $(LINUX_ROOT) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(COMPILER) modules

clean: 
	@make -C $(LINUX_ROOT) M=$(PWD) clean
