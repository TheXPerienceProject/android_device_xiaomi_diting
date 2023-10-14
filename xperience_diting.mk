#
# Copyright (C) 2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/xperience/config/common.mk)

# Inherit from diting device
$(call inherit-product, device/xiaomi/diting/device.mk)

# Device includes
EXTRA_UDFPS_ANIMATIONS := true

PRODUCT_DEVICE := diting
PRODUCT_NAME := xperience_diting
PRODUCT_BRAND := Xiaomi
PRODUCT_MODEL := diting
PRODUCT_MANUFACTURER := xiaomi

PRODUCT_GMS_CLIENTID_BASE := android-xiaomi

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="diting-user 13 SKQ1.220303.001 V14.0.3.0.TLFMIXM release-keys"

BUILD_FINGERPRINT := Xiaomi/diting/diting:13/SKQ1.220303.001/V14.0.3.0.TLFMIXM:user/release-keys
