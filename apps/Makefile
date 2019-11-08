# $Id$

# released or debug version, different on debug and log info£¬2007.03.15
# must be 'release' or 'debug'
EDITION=debug
# EDITION=release

# PLUGINS:=NO

# default: build all as plugins
ifeq ($(PLUGINS),NO)

else
	PLUGINS=YES
endif


PROJECT_NAME:=mux7xxApp

ROOT_DIR:=$(shell pwd)
RULE_DIR:=$(ROOT_DIR)/../

# conditional build options: X86/ARM, Player/Server/Web


export ROOT_DIR
export RULE_DIR

export HISI_LINUX_ENV=ok
export EDITION
export PLUGINS

BUILDTIME := $(shell TZ=CN date -u "+%Y_%m_%d")
GCC_VERSION := $(shell $(CC) -dumpversion )

INSTALL_NAME=$(PROJECT_NAME)_install_$(GCC_VERSION)_$(ARCH)_$(EDITION)_$(BUILDTIME).zip
UPDATE_NAME=$(PROJECT_NAME)_update_$(GCC_VERSION)_$(ARCH)_$(EDITION)_$(BUILDTIME).zip
#.tar.gz  

export ARCH
export BUILDTIME
export INSTALL_NAME
export UPDATE_NAME


	SHARED_DIRS += shared/src/common

	SHARED_DIRS += shared/src/mux
	SHARED_DIRS += shared/src/sys

# libraries PTP related
	APP_DIRS += ptp/src
	APP_DIRS += ptp/mux

ifeq ($(PLUGINS),YES)
endif

# Command Line client
	APP_DIRS += src/client


	APP_DIRS += src/main
	APP_DIRS += src/uenvs
	APP_DIRS += src/tests


	APP_DIRS += ptp/programs
	APP_DIRS += ptp/phc

SUBDIRS += $(SHARED_DIRS)

SUBDIRS += $(APP_DIRS)


all: 
	for i in $(SUBDIRS) ; do ( cd $$i && $(MAKE) $@ ) ; done

clean: 
	rm -rf muxMedia_*
	rm -rf mux7xxApp*
	rm -rf pkg
	rm -rf Linux.bin.* 
	rm -rf *.log
	- find . -name Linux.obj.* -prune -exec rm -r -f {} \;
	
# all	
install:
	@$(SHELL) $(ROOT_DIR)/buildver.sh $(ROOT_DIR)
	@$(SHELL) $(ROOT_DIR)/install.sh $(ROOT_DIR)/Linux.bin.$(ARCH) $(ROOT_DIR)/releases  


update:
	@$(SHELL) $(ROOT_DIR)/buildver.sh $(ROOT_DIR)
	@$(SHELL) $(ROOT_DIR)/update.sh $(ROOT_DIR)/Linux.bin.$(ARCH) $(ROOT_DIR)/releases  


#	@$(SHELL) $(ROOT_DIR)/buildver.sh $(ROOT_DIR)

package:clean
	cd ..; tar -cvjf $(PROJECT_NAME).$(BUILDTIME).tar.bz2 mux7xxApp
			
