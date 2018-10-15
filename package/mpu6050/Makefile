include $(TOPDIR)/rules.mk

PKG_NAME:=mpu6050
PKG_VERSION:=XAG-huangliquan-1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/mpu6050
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=mpu6050 firmware
	SUBMENU:=XAG
	DEPENDS:=
endef

define Package/mpu6050/description
	mpu6050 firmware 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/mpu6050/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mpu6050 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mpu6050))