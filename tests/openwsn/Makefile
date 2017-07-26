APPLICATION = openwsn_test
include ../Makefile.tests_common

BOARD=iotlab-m3

USEPKG += openwsn

USEMODULE += at86rf2xx

CFLAGS += -DTHREAD_STACKSIZE_MAIN=10240
CFLAGS += -DDAGROOT
CFLAGS += -DDEVELHELP

include $(RIOTBASE)/Makefile.include
