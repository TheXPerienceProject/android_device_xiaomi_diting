#
# Copyright (C) 2023 The Android Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Inherit from diting device.
$(call inherit-product, device/xiaomi/diting/device.mk)

## Device identifier
PRODUCT_BRAND := Xiaomi
PRODUCT_DEVICE := diting
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_NAME := lineage_diting

# GMS
PRODUCT_GMS_CLIENTID_BASE := android-xiaomi

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="diting_global-user 14 UKQ1.230917.001 V816.0.5.0.ULFMIXM release-keys" \
    TARGET_DEVICE=$(PRODUCT_SYSTEM_DEVICE) \
    TARGET_PRODUCT=$(PRODUCT_SYSTEM_NAME)
    PRODUCT_NAME=diting_global

BUILD_FINGERPRINT := Xiaomi/diting_global/diting:14/UKQ1.230917.001/V816.0.5.0.ULFMIXM:user/release-keys

# RisingOS flags
PRODUCT_NO_CAMERA := false
TARGET_ENABLE_BLUR := true
RISING_MAINTAINER := Kισżż
RISING_CHIPSET="Snapdragon 8+ Gen1"
PRODUCT_BUILD_PROP_OVERRIDES += \
    RISING_MAINTAINER="Kισżż" \
    RISING_CHIPSET="Snapdragon 8+ Gen1"
TARGET_BOOT_ANIMATION_RES := 1440
WITH_GMS := true
TARGET_HAS_UDFPS := true
