#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

OTHER_MENU:=Other modules
define KernelPackage/sdhci-mt7620
  SUBMENU:=Other modules
  TITLE:=MT7620 SDCI
  DEPENDS:=@(TARGET_ramips_mt7620||TARGET_ramips_mt7628||TARGET_ramips_mt7621||TARGET_ramips_mt7688) +kmod-sdhci
  KCONFIG:= \
	CONFIG_MTK_MMC \
	CONFIG_MTK_AEE_KDUMP=n \
	CONFIG_MTK_MMC_CD_POLL=n
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/host/mtk-mmc/mtk_sd.ko
  AUTOLOAD:=$(call AutoProbe,mtk_sd,1)
endef

$(eval $(call KernelPackage,sdhci-mt7620))

I2C_RALINK_MODULES:= \
  CONFIG_I2C_RALINK:drivers/i2c/busses/i2c-ralink

define KernelPackage/i2c-ralink
  $(call i2c_defaults,$(I2C_RALINK_MODULES),59)
  TITLE:=Ralink I2C Controller
  DEPENDS:=@TARGET_ramips @(!TARGET_ramips_mt7621) kmod-i2c-core
endef

define KernelPackage/i2c-ralink/description
 Kernel modules for enable ralink i2c controller.
endef

$(eval $(call KernelPackage,i2c-ralink))


I2C_MT7621_MODULES:= \
  CONFIG_I2C_MT7621:drivers/i2c/busses/i2c-mt7621

define KernelPackage/i2c-mt7621
  $(call i2c_defaults,$(I2C_MT7621_MODULES),59)
  TITLE:=MT7621 I2C Controller
  DEPENDS:=@TARGET_ramips @(TARGET_ramips_mt7628||TARGET_ramips_mt7688||TARGET_ramips_mt7621) kmod-i2c-core
endef

define KernelPackage/i2c-mt7621/description
 Kernel modules for enable mt7621 i2c controller.
endef

$(eval $(call KernelPackage,i2c-mt7621))



define KernelPackage/sound-mt7620
  TITLE:=MT7620 PCM/I2S Alsa Driver
  DEPENDS:=@TARGET_ramips_mt7620 +kmod-sound-soc-core +kmod-regmap @BROKEN
  KCONFIG:= \
	CONFIG_SND_MT7620_SOC_I2S \
	CONFIG_SND_MT7620_SOC_WM8960
  FILES:= \
	$(LINUX_DIR)/sound/soc/ralink/snd-soc-mt7620-i2s.ko \
	$(LINUX_DIR)/sound/soc/ralink/snd-soc-mt7620-wm8960.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8960.ko
  AUTOLOAD:=$(call AutoLoad,90,snd-soc-wm8960 snd-soc-mt7620-i2s snd-soc-mt7620-wm8960)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-mt7620/description
 Alsa modules for ralink i2s controller.
endef

$(eval $(call KernelPackage,sound-mt7620))


define KernelPackage/sound-mtk
  TITLE:=Mediatek I2S Alsa Driver
  DEPENDS:= +kmod-sound-soc-core +kmod-regmap +kmod-i2c-mt7621 @(TARGET_ramips_mt7628||TARGET_ramips_mt7688||TARGET_ramips_mt7620)
  KCONFIG:= \
	CONFIG_SND_MT76XX_SOC \
	CONFIG_SND_MT76XX_I2S \
	CONFIG_SND_MT76XX_PCM \
	CONFIG_SND_SOC_WM8960
  FILES:= \
	$(LINUX_DIR)/sound/soc/mtk/ralink_gdma.ko \
	$(LINUX_DIR)/sound/soc/mtk/snd-soc-mt76xx-i2s-ctl.ko \
	$(LINUX_DIR)/sound/soc/mtk/snd-soc-mt76xx-i2s.ko \
	$(LINUX_DIR)/sound/soc/mtk/snd-soc-mt76xx-pcm.ko \
	$(LINUX_DIR)/sound/soc/mtk/snd-soc-mt76xx-machine.ko \
	$(LINUX_DIR)/sound/soc/mtk/i2c_wm8960.ko \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-wm8960.ko
  AUTOLOAD:=$(call AutoLoad,90,ralink_gdma snd-soc-wm8960 i2c_wm8960 snd-soc-mt76xx-i2s-ctl snd-soc-mt76xx-i2s snd-soc-mt76xx-pcm snd-soc-mt76xx-machine)
  $(call AddDepends/sound)
endef

define KernelPackage/sound-mtk/description
 Alsa modules for ralink i2s controller.
endef

$(eval $(call KernelPackage,sound-mtk))

define KernelPackage/fbtft
  SUBMENU:=Video Support
  TITLE:=Framebuffer support
  DEPENDS:=@TARGET_ramips @(TARGET_ramips_mt7628||TARGET_ramips_mt7688||TARGET_ramips_mt7620) @DISPLAY_SUPPORT
  KCONFIG:=CONFIG_FB_TFT \
  CONFIG_FB_TFT_ILI9341
  FILES:=$(LINUX_DIR)/drivers/video/fbtft/fbtft.ko
  AUTOLOAD:=$(call AutoProbe,fbtft)
endef

define KernelPackage/fbtft/description
 Kernel support for spitft framebuffers
endef

$(eval $(call KernelPackage,fbtft))


