include $(TOPDIR)/rules.mk

PKG_NAME:=libar8020
PKG_VERSION:=XAG-huangliquan-1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=
PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/libar8020
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=artosyn ar8020 library
  SUBMENU:=XAG_LIB
  DEPENDS:=
endef

define Package/libar8020/description
  artosyn ar8020 library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


TARGET_CFLAGS += \
$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)))

define Build/Configure
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/libar8020.h $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib*.so $(1)/usr/lib/
endef

define Package/libar8020/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libar8020.so $(1)/usr/lib
endef

$(eval $(call BuildPackage,libar8020))

