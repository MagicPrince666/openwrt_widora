include $(TOPDIR)/rules.mk

PKG_NAME:=artosyn
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/artosyn
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=artosyn firmwave
	SUBMENU:=XAG
	DEPENDS:= +libar8020 +libpthread +libstdcpp
endef

define Package/artosyn/description
	artosyn firmwave
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/artosyn/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/upgrade_demo/sample_upgrade $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/send_video/sample_transceiver $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/cmd_demo/sample_cmd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/uvc_video/uvc_video $(1)/usr/bin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/video_demo/vedio_demo $(1)/usr/bin/
endef

$(eval $(call BuildPackage,artosyn))