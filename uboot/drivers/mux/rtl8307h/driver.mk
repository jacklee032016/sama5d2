#
# Makefile for the contributed drivers
#
CONTRIB_DRIVERS_SRC:=$(TOPDIR)/contrib/driver/rtl8307h_sdk

INCL  += $(CONTRIB_DRIVERS_SRC)/include
INCL  += $(CONTRIB_DRIVERS_SRC)/include/asicdrv

COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/src/rtk_api.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/src/rtk_i2c.o
COBJS-$(CONFIG_TWI0)		+= $(CONTRIB_DRIVERS_SRC)/src/rtk_error.o

