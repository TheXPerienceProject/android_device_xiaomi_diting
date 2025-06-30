#
# Copyright (C) 2023 The Android Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/xperience/config/common.mk)

# Inherit from diting device.
$(call inherit-product, device/xiaomi/diting/device.mk)


## Device identifier
PRODUCT_BRAND := Xiaomi
PRODUCT_DEVICE := diting
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_NAME := xperience_diting
PRODUCT_MODEL := diting

# GMS
PRODUCT_GMS_CLIENTID_BASE := android-xiaomi

BUILD_FINGERPRINT := Xiaomi/diting/diting:12/SKQ1.230401.001/V816.0.8.0.ULFMIXM:user/release-keys

PRODUCT_PACKAGES += \
    LatinIME
