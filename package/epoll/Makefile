include $(TOPDIR)/rules.mk

PKG_NAME:=epoll
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/epoll
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=uart epoll firmware
	SUBMENU:=XAG
	DEPENDS:= +libpthread +libstdcpp
endef

define Package/epoll/description
	uart use epoll firmware 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/epoll/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uart_epoll $(1)/bin/
endef

$(eval $(call BuildPackage,epoll))