# SPDX-License-Identifier: GPL-2.0+

ifeq ($(MUX_BOARD), MUX_ATMEL_XPLAINED)
	dtb-$(CONFIG_TARGET_SAMA5D2_XPLAINED) += \
		at91-sama5d2_xplained.dtb
else 
	dtb-$(CONFIG_TARGET_SAMA5D2_XPLAINED) += \
		mux7xx.dts
endif

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
