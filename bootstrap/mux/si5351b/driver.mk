# Makefile for AT91-Bootstrap drivers directory
# DIRS				+= drivers

DRIVERS:=
DRIVERS_SRC:=$(TOPDIR)/contrib/driver/si5351b


COBJS-$(CONFIG_TWI0)		+= $(DRIVERS_SRC)/si5351b_api.o

