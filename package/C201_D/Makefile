#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

USB_MENU:=USB Support


PKG_NAME:=usb-artosyn_usb_8020
PKG_RELEASE:=XAG-artosyn-1.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SUBMENU:=$(USB_MENU)
  TITLE:=artosyn usb 8020
  FILES:=$(PKG_BUILD_DIR)/artosyn_usb_8020.ko
  KCONFIG:=CONFIG_USB CONFIG_XPS_USB_HCD_XILINX=n CONFIG_USB_FHCI_HCD=n
  AUTOLOAD:=$(call AutoProbe,artosyn_usb_8020)
  #$(call AddDepends/usb,usbcore)
  DEPENDS:=@USB_SUPPORT +kmod-usb-core
endef

define KernelPackage/usb-artosyn_usb_8020/description
 artosyn usb 8020 modems
endef

EXTRA_KCONFIG:= \
	CONFIG_USB_ARTOSYN_USB_8020=m

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

$(eval $(call KernelPackage,usb-artosyn_usb_8020))
