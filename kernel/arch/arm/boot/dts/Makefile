# SPDX-License-Identifier: GPL-2.0
ifeq ($(CONFIG_OF),y)

dtb-$(CONFIG_SOC_SAM_V7) += \
	mux774_v2.dtb 

#	at91-sama5d2_xplained.dtb 
	
#	at91-kizbox2.dtb \
#	at91-nattis-2-natte-2.dtb \
#	at91-sama5d27_som1_ek.dtb \
#	at91-sama5d2_ptc_ek.dtb \
#	at91-sama5d3_xplained.dtb \
#	at91-tse850-3.dtb \
#	sama5d31ek.dtb \
#	sama5d33ek.dtb \
#	sama5d34ek.dtb \
#	sama5d35ek.dtb \
#	sama5d36ek.dtb \
#	sama5d36ek_cmp.dtb \
#	at91-sama5d4_ma5d4evk.dtb \
#	at91-sama5d4_xplained.dtb \
#	at91-sama5d4ek.dtb \
#	at91-sama5d4ek_isi.dtb \
#	at91-vinco.dtb
	
endif

dtstree		:= $(srctree)/$(src)
dtb-$(CONFIG_OF_ALL_DTBS) := $(patsubst $(dtstree)/%.dts,%.dtb, $(wildcard $(dtstree)/*.dts))

always		:= $(dtb-y)
clean-files	:= *.dtb
