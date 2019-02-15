include $(TOPDIR)/rules.mk

PKG_NAME:=uvc_aoa
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=xlinkhs for uvc to android aoa
	SUBMENU:=XAG
	DEPENDS:= +libusb-1.0 +libpthread +libstdcpp
endef

define Package/$(PKG_NAME)/description
	uvc_aoa firmware 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uvc_aoa $(1)/usr/bin/
endef

$(eval $(call BuildPackage,uvc_aoa))