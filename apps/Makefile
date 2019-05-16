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
RULE_DIR:=$(ROOT_DIR)/

# conditional build options: X86/ARM, Player/Server/Web


export ROOT_DIR
export RULE_DIR

export HISI_LINUX_ENV=ok
export EDITION
export PLUGINS



RELEASES_NAME=$(PROJECT_NAME)_$(GCC_VERSION)_$(ARCH)_$(EDITION)_$(BUILDTIME).zip
#.tar.gz  

export ARCH
export BUILDTIME
export RELEASES_NAME

include $(RULE_DIR)/Rules.mak

	SHARED_DIRS += shared/src/common
# SHARED_DIRS += shared/src/media
	SHARED_DIRS += shared/src/mux
	SHARED_DIRS += shared/src/sys

#	SHARED_DIRS += shared/libcgi




ifeq ($(PLUGINS),YES)
endif

#	APP_DIRS += src/web

	# Command Line client
	APP_DIRS += src/client
	# CGI/REST API
	# APP_DIRS += src/rest

	APP_DIRS += src/main
#	APP_DIRS += src/envs
	APP_DIRS += src/tests


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

#	@$(SHELL) $(ROOT_DIR)/buildver.sh $(ROOT_DIR)

package:clean
	cd ..; tar -cvjf $(PROJECT_NAME).$(BUILDTIME).tar.bz2 mux7xxApp
			
