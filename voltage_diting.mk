#
# Copyright (C) 2023 The Android Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common voltage stuff.
$(call inherit-product, vendor/voltage/config/common_full_phone.mk)

# Inherit from diting device.
$(call inherit-product, device/xiaomi/diting/device.mk)

# $(call inherit-product-if-exists, vendor/google/gms/config.mk)

## Device identifier
PRODUCT_BRAND := Xiaomi
PRODUCT_DEVICE := diting
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_NAME := voltage_diting
PRODUCT_MODEL := diting

# GMS
PRODUCT_GMS_CLIENTID_BASE := android-xiaomi

BUILD_FINGERPRINT := Xiaomi/diting_global/diting:14/UKQ1.230917.001/V816.0.5.0.ULFMIXM:user/release-keys

# Include Pixel Launcher
INCLUDE_PIXEL_LAUNCHER := true
PRODUCT_NO_CAMERA := false
TARGET_ENABLE_BLUR := true
EXTRA_UDFPS_ANIMATIONS := true
TARGET_BOOT_ANIMATION_RES := 2560
TARGET_HAS_UDFPS := true
TARGET_PREBUILT_LAWNCHAIR_LAUNCHER := true
TARGET_DEFAULT_PIXEL_LAUNCHER := true
