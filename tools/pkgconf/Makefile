#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=pkgconf
PKG_VERSION:=1.9.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://distfiles.dereferenced.org/pkgconf
PKG_HASH:=5fb355b487d54fb6d341e4f18d4e2f7e813a6622cf03a9e87affa6a40565699d

include $(INCLUDE_DIR)/host-build.mk

unexport PKG_CONFIG

HOSTCC := $(HOSTCC_NOCACHE)

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
	mv $(STAGING_DIR_HOST)/bin/pkg-config $(STAGING_DIR_HOST)/bin/pkg-config.real
	$(INSTALL_BIN) ./files/pkg-config $(STAGING_DIR_HOST)/bin/pkg-config
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/pkg-config.real $(STAGING_DIR_HOST)/bin/pkg-config
	$(call Host/Clean/Default)
endef

$(eval $(call HostBuild))
