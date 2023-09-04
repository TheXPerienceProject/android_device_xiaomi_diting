#
# Copyright (C) 2023 The Android Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common DerpFest stuff.
$(call inherit-product, vendor/xperience/config/common.mk)

# Inherit from diting device.
$(call inherit-product, device/xiaomi/diting/device.mk)

# Device includes
EXTRA_UDFPS_ANIMATIONS := true

## Device identifier
PRODUCT_DEVICE := diting
PRODUCT_NAME := xperience_diting
PRODUCT_MANUFACTURER := Xiaomi

# GMS
PRODUCT_GMS_CLIENTID_BASE := android-xiaomi
SYSTEM_DOESNT_HAVE_VENDOR_OSS := true
TARGET_USES_PREBUILT_VENDOR_SEPOLICY := true
