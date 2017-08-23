APPLICATION = driver_soft_spi
include ../Makefile.tests_common

BOARD ?= stm32f4discovery

BOARD_WHITELIST := stm32f4discovery samr21-xpro

USEMODULE += soft_spi
USEMODULE += periph_common

DISABLE_MODULE += auto_init

ifneq (,$(filter samr21-xpro,$(BOARD)))
  DISABLE_MODULE += sam0_common_periph
endif

include $(RIOTBASE)/Makefile.include
