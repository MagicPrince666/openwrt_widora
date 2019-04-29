include $(TOPDIR)/rules.mk

PKG_NAME:=sonny_ps2
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=sonny ps2 wileress firmware
	SUBMENU:=XAG
	DEPENDS:=
endef

define Package/$(PKG_NAME)/description
	sonny ps2 wileress firmware 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sonny_ps2 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))