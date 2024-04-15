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

$(call inherit-product, vendor/gapps/common/common-vendor.mk)

## Device identifier
PRODUCT_BRAND := Xiaomi
PRODUCT_DEVICE := diting
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_NAME := lineage_diting

# GMS
PRODUCT_GMS_CLIENTID_BASE := android-xiaomi

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_NAME=diting_global

# Matrix
MATRIXX_MAINTAINER := Kισżż
MATRIXX_CHIPSET := SM8475
MATRIXX_BATTERY := 5000mah
MATRIXX_DISPLAY := 1220x2712
WITH_GMS := true
WITH_GAPPS := true
EXTRA_GAPPS := true
BUILD_GOOGLE_DIALER := true
BUILD_GOOGLE_CONTACTS := true
BUILD_GOOGLE_MESSAGE := true
TARGET_HAS_UDFPS := true
PRODUCT_USE_SCUDO := true
# Blur
TARGET_ENABLE_BLUR := true
