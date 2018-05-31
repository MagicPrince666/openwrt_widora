#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#USBNET_DIR:=net/usb
USB_MENU:=USB Support

USBNET_DIR:=net/usb

PKG_NAME:=usb-net-simcom-wwan
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=$(USB_MENU)
  TITLE:=SIMCOM 7600 WWAN driver
  FILES:=$(PKG_BUILD_DIR)/simcom_wwan.ko
  KCONFIG:=
  #FILES:= $(LINUX_DIR)/drivers/net/usb/simcom_wwan.ko
  AUTOLOAD:=$(call AutoProbe,simcom_wwan)
  #AUTOLOAD:=$(call AutoProbe,90,simcom_wwan,1)
  DEPENDS:=+kmod-usb-net +kmod-usb-wdm
  #$(call AddDepends/usb-net,+kmod-usb-wdm)
endef

define KernelPackage/usb-net-simcom-wwan/description
 Kernel support for Simcom 7600 4G and LTE modems
endef

EXTRA_KCONFIG:= \
	CONFIG_USB_NET_SIMCOM_SIM7600=m

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

$(eval $(call KernelPackage,usb-net-simcom-wwan))
