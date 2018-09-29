#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtk-kdma
PKG_RELEASE:=XAG-KDMA-1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mtk-kdma
  SUBMENU:=XAG_KERNELMODULES
  TITLE:=mtk kernel dma
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/kdma.ko
  KCONFIG:=
  AUTOLOAD:=$(call AutoProbe,kdma)
endef

define KernelPackage/mtk-kdma/description
 mtk kernel dma
endef

EXTRA_KCONFIG:= \
    CONFIG_MTK_KDMA=m

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

$(eval $(call KernelPackage,mtk-kdma))
