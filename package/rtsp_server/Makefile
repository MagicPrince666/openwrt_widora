include $(TOPDIR)/rules.mk

PKG_NAME:=rtsp_server
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/rtsp_server
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=rtsp server
	DEPENDS:= +librt +libm +libdl +libstdcpp +libpthread
endef

define Package/rtsp_server/description
	rtsp server to h264 video
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/rtsp_server/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtsp_server $(1)/bin/
endef

$(eval $(call BuildPackage,rtsp_server))