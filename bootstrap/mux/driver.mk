#
# Makefile for the contributed drivers
#
CONTRIB_DRIVERS_SRC:=$(TOPDIR)/contrib/driver

INCL  += -I$(CONTRIB_DRIVERS_SRC)/si5351b
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/si5351b/si5351b.o

INCL  += -I$(CONTRIB_DRIVERS_SRC)/mdio
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/mdio/mdio_drv.o

INCL  += -I$(CONTRIB_DRIVERS_SRC)/adv7619
INCL  += -I$(CONTRIB_DRIVERS_SRC)/adv7619/RX
INCL  += -I$(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB
INCL  += -I$(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL
INCL  += -I$(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G
INCL  += -I$(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/ADV7619
INCL  += -I$(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/ADV7619/HAL
INCL  += -I$(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/ADV7619/MACROS
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/wrapper.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/rx_afe_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/rx_cp_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/rx_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/rx_hdmi_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/rx_sdp_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/rx_sdp_vdp_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/rx_vdp_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/rx_5g_cp_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/rx_5g_gen_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/rx_5g_hdmi_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/rx_5g_no_supp_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/ADV7619/HAL/7619_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/HAL/5G/ADV7619/HAL/7619_ini_hal.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_afe.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_cdec.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_cec.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_cp.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_dpp_csc.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_gen.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_hdmi.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_isr.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_multi.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_sdp.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/adv7619/RX/LIB/rx_vdp.o

COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/sii1136/sii1136.o

#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/pwmc/pwmc.o

INCL  += -I$(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/include
INCL  += -I$(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/include/asicdrv
INCL  += -I$(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/include/asicdrv/basic



#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/rtk_api.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/rtk_i2c.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/rtk_error.o

#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_acl.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_dot1x.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_gpio.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_igmp.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_lock.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_lut.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_mem_ctrl.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_mib.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_misc.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_phy.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_port.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_rma.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_svlan.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_vlan.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/rtl8307h_asicdrv_wol.o

#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/basic/rtl8307h_reg_list.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/basic/rtl8307h_regField_list.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/basic/rtl8307h_table_list.o
#COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/rtl8307h_sdk/src/asicdrv/basic/rtl8307h_tableField_list.o

