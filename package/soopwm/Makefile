#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtk-soopwm
PKG_RELEASE:=XAG-SOOPWM-1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mtk-soopwm
  SUBMENU:=XAG_KERNELMODULES
  TITLE:=mtk soopwm
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/sooall_pwm.ko
  KCONFIG:=
  AUTOLOAD:=$(call AutoProbe,sooall_pwm)
endef

define KernelPackage/mtk-soopwm/description
 mtk soopwm
endef

EXTRA_KCONFIG:= \
    CONFIG_MTK_SOOPWM=m

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

$(eval $(call KernelPackage,mtk-soopwm))
