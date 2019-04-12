include $(TOPDIR)/rules.mk

PKG_NAME:=spi_tft
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/spi_tft
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=spi_tft firmware
	SUBMENU:=XAG
	DEPENDS:= +libpthread +libstdcpp +librt
endef

define Package/spi_tft/description
	spi_tft firmware 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/spi_tft/install
	$(INSTALL_DIR) $(1)/usr/bin #$(1)/usr/lib $(1)/tft
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/simple-spitft $(1)/usr/bin/
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/libiconv.so.2 $(1)/usr/lib
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/*.fon  $(PKG_BUILD_DIR)/bin/test.* $(1)/tft
endef

$(eval $(call BuildPackage,spi_tft))