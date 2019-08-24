# SPDX-License-Identifier: GPL-2.0
ifeq ($(CONFIG_OF),y)

ifeq ($(MUX_BOARD), MUX_ATMEL_XPLAINED)
	dtb-$(CONFIG_SOC_SAM_V7) += \
		xplained.dtb 
else 
	ifeq ($(MUX_BOARD), MUX_BOARD_768)
		dtb-$(CONFIG_SOC_SAM_V7) += \
			mux768.dtb 
	else	
		dtb-$(CONFIG_SOC_SAM_V7) += \
			mux774_v2.dtb 
	endif		
endif


	
endif

dtstree		:= $(srctree)/$(src)
dtb-$(CONFIG_OF_ALL_DTBS) := $(patsubst $(dtstree)/%.dts,%.dtb, $(wildcard $(dtstree)/*.dts))

always		:= $(dtb-y)
clean-files	:= *.dtb
