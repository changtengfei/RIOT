APPLICATION = openwsn_sctimer
include ../Makefile.tests_common

CFLAGS += -DTHREAD_STACKSIZE_MAIN=10240
CFLAGS += -DTHREAD_EXTRA_STACKSIZE_PRINTF=4096


USEPKG += openwsn
USEMODULE += at86rf2xx

USEMODULE += ps

CFLAGS += -DDEVELHELP

include $(RIOTBASE)/Makefile.include
