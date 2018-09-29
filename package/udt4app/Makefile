include $(TOPDIR)/rules.mk

PKG_NAME:=udt4app
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/udt4app
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=udt4app
	SUBMENU:=XAG
	DEPENDS:= +libstdcpp +libpthread +libudt
endef

define Package/udt4app/description
	udt4app firmware
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/udt4app/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/appserver $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/appclient $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sendfile $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/recvfile $(1)/usr/bin/
endef

$(eval $(call BuildPackage,udt4app))