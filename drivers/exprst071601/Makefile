#ifeq ($(PARAM_FILE), )
#        PARAM_FILE:=../../Makefile.param
#        include $(PARAM_FILE)
#endif

ARCH=arm
#COMPILER=/home/alex/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-
COMPILER=/home/alex/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-
LINUX_ROOT := /home/500768/laurent20180125/linux-at91

obj-m := exprst.o

default:
	@make -C $(LINUX_ROOT) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(COMPILER)  modules

clean: 
	@make -C $(LINUX_ROOT) M=$(PWD) clean


