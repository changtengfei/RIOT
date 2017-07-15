PKG_NAME=openwsn-fw
PKG_URL=https://github.com/openwsn-berkeley/openwsn-fw.git
PKG_VERSION=045a674ee3c86b904f2fafc5d036f89b85001128
PKG_BUILDDIR ?= $(BINDIRBASE)/pkg/$(BOARD)/$(PKG_NAME)
PKG_LICENSE=BSD-3-Clause

.PHONY: all

all: git-download
	"$(MAKE)" -C $(PKG_BUILDDIR)

include $(RIOTBASE)/pkg/pkg.mk
