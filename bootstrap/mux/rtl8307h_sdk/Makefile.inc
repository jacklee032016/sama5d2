
ifeq ($(CONFIG_LIB_RTL8305H),y)

CFLAGS_INC += -I$(TOP)/test_utils/test_01
CFLAGS_INC += -I$(TOP)/lib/rtl8305h_sdk/include
CFLAGS_INC += -I$(TOP)/lib/rtl8305h_sdk/include/asicdrv
CFLAGS_INC += -I$(TOP)/lib/rtl8305h_sdk/include/asicdrv/basic

lib-y += lib/rtl8305h.a

rtl8305h-y :=

include $(TOP)/lib/rtl8305h_sdk/src/Makefile.inc

RTL8305H_OBJS := $(addprefix $(BUILDDIR)/,$(rtl8305h-y))

-include $(RTL8305H_OBJS:.o=.d)

$(BUILDDIR)/lib/rtl8305h.a: $(RTL8305H_OBJS)
	@mkdir -p $(BUILDDIR)/lib
	$(ECHO) AR $@
	$(Q)$(AR) -cr $@ $^

endif
