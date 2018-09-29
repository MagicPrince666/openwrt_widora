include $(TOPDIR)/rules.mk

PKG_NAME:=pwm_servo
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/pwm_servo
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=pwm servo firmware
	SUBMENU:=XAG
	DEPENDS:= +libpthread +libstdcpp
endef

define Package/pwm_servo/description
	uart use pwm_servo firmware 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/pwm_servo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/servo $(1)/usr/bin/
endef

$(eval $(call BuildPackage,pwm_servo))