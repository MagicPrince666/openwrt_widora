include $(TOPDIR)/rules.mk

PKG_NAME:=http_server
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/http_server
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=http server firmware
	SUBMENU:=XAG
	DEPENDS:= +libstdcpp +boost +boost-thread +boost-filesystem +boost-system +libpthread +libopenssl +zlib +librt 
endef

define Package/http_server/description
	http_server firmware 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/http_server/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/www
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/http_server $(PKG_BUILD_DIR)/https_server $(1)/usr/bin/
	$(CP) -r $(PKG_BUILD_DIR)/web $(1)/www/
endef

$(eval $(call BuildPackage,http_server))