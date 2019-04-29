include $(TOPDIR)/rules.mk

PKG_NAME:=tcp_transfer
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=xag tcp file transfer
	SUBMENU:=XAG
	DEPENDS:= +libstdcpp
endef

define Package/$(PKG_NAME)/description
	xag xlinkhs file transfer
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcp_transfer_s $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcp_transfer_c $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))