#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=spi-ch341-usb
PKG_RELEASE:=PRINCE-USB-SPI-1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=prince
  TITLE:=usb to spi for ch341
  DEPENDS:=+kmod-usb-core
  FILES:=$(PKG_BUILD_DIR)/spi-ch341-usb.ko
  KCONFIG:=
  AUTOLOAD:=$(call AutoProbe,spi-ch341-usb)
endef

define KernelPackage/$(PKG_NAME)/description
 usb to spi for ch341
endef

EXTRA_KCONFIG:= \
    CONFIG_SPI_CH341_USB=m

EXTRA_CFLAGS:= \
    $(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
    $(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
    ARCH="$(LINUX_KARCH)" \
    CROSS_COMPILE="$(TARGET_CROSS)" \
    SUBDIRS="$(PKG_BUILD_DIR)" \
    EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
    $(EXTRA_KCONFIG)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
	$(MAKE_OPTS) \
	modules
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
