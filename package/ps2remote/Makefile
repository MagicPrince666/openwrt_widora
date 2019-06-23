include $(TOPDIR)/rules.mk

PKG_NAME:=ps2wireless
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=ps2 wireles firmware
	SUBMENU:=XAG
	DEPENDS:= +libpthread +libstdcpp
endef

define Package/$(PKG_NAME)/description
	ps2 wireles firmware
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ps2 $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))