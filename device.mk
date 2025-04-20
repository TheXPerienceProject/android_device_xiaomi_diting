#
# Copyright (C) 2023 The Android Open Source Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Inherit virtual_ab_ota_product.
$(call inherit-product, $(SRC_TARGET_DIR)/product/virtual_ab_ota/launch_with_vendor_ramdisk.mk)

# Enable project quotas and casefolding for emulated storage without sdcardfs.
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulated_storage.mk)

# Enforce generic ramdisk allow list
$(call inherit-product, $(SRC_TARGET_DIR)/product/generic_ramdisk.mk)

# Enable updating of APEXes
$(call inherit-product, $(SRC_TARGET_DIR)/product/updatable_apex.mk)

# Setup dalvik vm configs
$(call inherit-product, frameworks/native/build/phone-xhdpi-6144-dalvik-heap.mk)

# Call the proprietary setup.
$(call inherit-product, vendor/xiaomi/diting/diting-vendor.mk)

PRODUCT_SHIPPING_API_LEVEL := 31
BOARD_SHIPPING_API_LEVEL := 31

# AAPT
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xxhdpi

# A/B
AB_OTA_POSTINSTALL_CONFIG += \
    RUN_POSTINSTALL_system=true \
    POSTINSTALL_PATH_system=system/bin/otapreopt_script \
    FILESYSTEM_TYPE_system=ext4 \
    POSTINSTALL_OPTIONAL_system=true

AB_OTA_POSTINSTALL_CONFIG += \
    RUN_POSTINSTALL_vendor=true \
    POSTINSTALL_PATH_vendor=bin/checkpoint_gc \
    FILESYSTEM_TYPE_vendor=ext4 \
    POSTINSTALL_OPTIONAL_vendor=true

PRODUCT_PACKAGES += \
    checkpoint_gc \
    otapreopt_script

# AIDL NDK backend vendor
PRODUCT_PACKAGES += \
    android.hardware.common-V2-ndk_platform.vendor \
    android.hardware.gnss-V1-ndk_platform.vendor \
    android.hardware.identity-V3-ndk_platform.vendor \
    android.hardware.keymaster-V3-ndk_platform.vendor \
    android.hardware.light-V1-ndk_platform.vendor \
    android.hardware.memtrack-V1-ndk_platform.vendor \
    android.hardware.security.keymint-V1-ndk_platform.vendor \
    android.hardware.security.secureclock-V1-ndk_platform.vendor \
    android.hardware.security.sharedsecret-V1-ndk_platform.vendor

# ANT+
PRODUCT_PACKAGES += \
    com.dsi.ant@1.0.vendor

# Atrace
PRODUCT_PACKAGES += \
    android.hardware.atrace@1.0-service

# Audio
SOONG_CONFIG_NAMESPACES += android_hardware_audio
SOONG_CONFIG_android_hardware_audio += \
    run_64bit
SOONG_CONFIG_android_hardware_audio_run_64bit := true

PRODUCT_PACKAGES += \
    android.media.audio.common.types-V1-cpp.vendor \
    android.hardware.audio@7.1-impl \
    android.hardware.audio.effect@7.0-impl \
    android.hardware.audio.service \
    android.hardware.soundtrigger@2.3-impl \
    libaudioroute.vendor \
    libhapticgenerator

PRODUCT_PACKAGES += \
    audio.bluetooth.default \
    audio.r_submix.default \
    audio.usb.default

PRODUCT_PACKAGES += \
    libvolumelistener \
    libqcompostprocbundle \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libhfp_pal \
    libbatterylistener

PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.audio.pro.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.pro.xml \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml

AUDIO_HAL_DIR := hardware/qcom-caf/sm8450/audio/primary-hal

PRODUCT_COPY_FILES += \
    $(AUDIO_HAL_DIR)/configs/common/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(AUDIO_HAL_DIR)/configs/taro/microphone_characteristics.xml:$(TARGET_COPY_OUT_VENDOR)/etc/microphone_characteristics.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio/sku_cape/audio_policy_configuration.xml \
    $(LOCAL_PATH)/configs/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio/sku_cape_qssi/audio_policy_configuration.xml

# Automotive
PRODUCT_PACKAGES += \
    android.hardware.automotive.vehicle@2.0-manager-lib

# Authsecret
PRODUCT_PACKAGES += \
    android.hardware.authsecret@1.0.vendor

# Bluetooth
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0.vendor \
    android.hardware.bluetooth.audio-V2-ndk.vendor \
    android.hardware.bluetooth.audio-impl \
    libldacBT_abr \
    libldacBT_enc \
    vendor.qti.hardware.bluetooth_audio@2.1.vendor \
    vendor.qti.hardware.btconfigstore@1.0.vendor \
    vendor.qti.hardware.btconfigstore@2.0.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml

# Boot control
PRODUCT_PACKAGES += \
    android.hardware.boot@1.2-impl-qti \
    android.hardware.boot@1.2-impl-qti.recovery \
    android.hardware.boot@1.2-service

PRODUCT_PACKAGES_DEBUG += \
    bootctl

# Camera
$(call inherit-product-if-exists, vendor/xiaomi/camera/miuicamera.mk)

PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.7.vendor \
    android.hardware.graphics.common-V4-ndk \
    android.hardware.camera.device-V1-ndk.vendor \
    android.hardware.camera.metadata-V1-ndk.vendor \
    android.hardware.camera.provider-V1-ndk.vendor \
    camera.device@1.0-impl \
    libcamera2ndk_vendor \
    vendor.qti.hardware.camera.aon@1.0.vendor \
    vendor.qti.hardware.camera.postproc@1.0.vendor \
    libutilscallstack.vendor \
    libjpeg.vendor \
    libexif.vendor \
    libpiex_shim \
    libcamera_metadata.vendor \
    libyuv.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml \
    frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml

# Display
PRODUCT_PACKAGES += \
    android.hardware.graphics.mapper@4.0-impl-qti-display \
    vendor.qti.hardware.display.allocator-service \
    vendor.qti.hardware.display.composer-service

PRODUCT_PACKAGES += \
    init.qti.display_boot.rc \
    init.qti.display_boot.sh

PRODUCT_COPY_FILES += \
    hardware/qcom-caf/sm8450/display/config/snapdragon_color_libs_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/snapdragon_color_libs_config.xml \
    $(LOCAL_PATH)/configs/display/advanced_sf_offsets.xml:$(TARGET_COPY_OUT_VENDOR)/etc/display/advanced_sf_offsets.xml

# Doze
PRODUCT_PACKAGES += \
    Doze

PRODUCT_SYSTEM_EXT_PROPERTIES += \
    ro.sensor.pickup=xiaomi.sensor.pickup \
    ro.sensor.pickup.lower.value=2 \
    ro.sensor.proximity=true

# Capabilityconfigstore
PRODUCT_PACKAGES += \
    vendor.qti.hardware.capabilityconfigstore@1.0.vendor

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm@1.4.vendor \
    android.hardware.drm-service.clearkey \
    libdrm.vendor

# Dumpstate
PRODUCT_PACKAGES += \
    android.hardware.dumpstate@1.1.vendor

# Fastbootd
PRODUCT_PACKAGES += \
    android.hardware.fastboot@1.1-impl-mock \
    fastbootd

# Fingerprint
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint@2.3.vendor \
    android.hardware.biometrics.fingerprint-service.xiaomi \
    libudfpshandler \
    sensors.xiaomi

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.fingerprint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.fingerprint.xml

# Fstab
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/charger_fw_fstab.qti:$(TARGET_COPY_OUT_VENDOR)/etc/charger_fw_fstab.qti

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/fstab.qcom:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.qcom \
    $(LOCAL_PATH)/rootdir/etc/fstab.qcom:$(TARGET_COPY_OUT_RECOVERY)/root/first_stage_ramdisk/fstab.qcom \
    $(LOCAL_PATH)/rootdir/etc/fstab.qcom:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.qcom

# Fastbootd
PRODUCT_PACKAGES += \
    android.hardware.fastboot@1.1-impl-mock \
    fastbootd

# Gatekeeper
PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0.vendor

# GPS
PRODUCT_PACKAGES += \
    android.hardware.gnss@2.1-impl-qti \
    android.hardware.gnss-aidl-impl-qti \
    android.hardware.gnss-aidl-service-qti \
    android.hardware.gnss@2.1.vendor \
    android.hardware.gnss-V2-ndk.vendor \
    android.hardware.power@1.3 \
    android.hardware.power@1.3.vendor \
    libgrpc++_unsecure.vendor

PRODUCT_PACKAGES += \
    libbatching \
    libgeofencing \
    libgnss

PRODUCT_PACKAGES += \
    apdr.conf \
    batching.conf \
    gnss_antenna_info.conf \
    gps.conf \
    izat.conf \
    lowi.conf \
    sap.conf

PRODUCT_PACKAGES += \
    gnss@2.0-base.policy \
    gnss@2.0-xtra-daemon.policy

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml

# Graphics
PRODUCT_PACKAGES += \
    android.hardware.graphics.composer@2.4.vendor \
    android.hardware.graphics.allocator-V1-ndk \
    android.hardware.graphics.allocator-V1-ndk.vendor \
    android.hardware.graphics.common-V3-ndk \
    android.hardware.graphics.common-V3-ndk.vendor \
    android.hardware.graphics.common-V4-ndk \
    android.hardware.graphics.common-V4-ndk.vendor \
    libgralloc.qti \
    libgui_vendor \
    libqdMetaData \
    libtinyxml \
    libtinyxml2 \
    vendor.display.config@1.11.vendor \
    vendor.qti.hardware.display.allocator@1.0.vendor \
    vendor.qti.hardware.display.allocator@3.0.vendor \
    vendor.qti.hardware.display.allocator@4.0.vendor \
    vendor.qti.hardware.display.composer@1.0.vendor \
    vendor.qti.hardware.display.composer@2.0.vendor \
    vendor.qti.hardware.display.composer@3.0.vendor \
    vendor.qti.hardware.display.composer@3.1.vendor \
    vendor.qti.hardware.display.config-V1-ndk_platform.vendor \
    vendor.qti.hardware.display.config-V2-ndk_platform.vendor \
    vendor.qti.hardware.display.config-V3-ndk_platform.vendor \
    vendor.qti.hardware.display.config-V4-ndk_platform.vendor \
    vendor.qti.hardware.display.config-V5-ndk_platform.vendor \
    vendor.qti.hardware.display.config-V6-ndk_platform.vendor \
    vendor.qti.hardware.display.demura@2.0.vendor \
    vendor.qti.hardware.display.mapper@1.0.vendor \
    vendor.qti.hardware.display.mapper@1.1.vendor \
    vendor.qti.hardware.display.mapper@2.0.vendor \
    vendor.qti.hardware.display.mapper@3.0.vendor \
    vendor.qti.hardware.display.mapper@4.0.vendor \
    vendor.qti.hardware.display.mapperextensions@1.1.vendor \
    vendor.qti.hardware.display.mapperextensions@1.2.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute-0.xml \
    frameworks/native/data/etc/android.hardware.vulkan.level-1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level-1.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version-1_1.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_3.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version-1_3.xml \
    frameworks/native/data/etc/android.software.opengles.deqp.level-2021-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.opengles.deqp.level.xml \
    frameworks/native/data/etc/android.software.vulkan.deqp.level-2021-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.vulkan.deqp.level.xml

# Health
PRODUCT_PACKAGES += \
    android.hardware.health@2.1-impl-diting \
    android.hardware.health@2.1-service

# HotwordEnrollement
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/permissions/hotword-hiddenapi-package-whitelist.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/sysconfig/hotword-hiddenapi-package-whitelist.xml \
    $(LOCAL_PATH)/configs/permissions/privapp-permissions-hotword.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/privapp-permissions-hotword.xml

# HIDL
PRODUCT_PACKAGES += \
    android.hidl.manager@1.0 \
    android.hidl.manager@1.0.vendor \
    android.hidl.memory@1.0-impl \
    android.hidl.memory.block@1.0 \
    android.hidl.memory.block@1.0.vendor \
    android.hidl.allocator@1.0.vendor \
    libhidltransport \
    libhidltransport.vendor \
    libhidlbase_shim \
    libhwbinder \
    libhwbinder.vendor

# HotwordEnrollement
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/permissions/hotword-hiddenapi-package-whitelist.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/sysconfig/hotword-hiddenapi-package-whitelist.xml \
    $(LOCAL_PATH)/configs/permissions/privapp-permissions-hotword.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/privapp-permissions-hotword.xml

# Input
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/idc/uinput-fpc.idc:$(TARGET_COPY_OUT_SYSTEM)/usr/idc/uinput-fpc.idc \
    $(LOCAL_PATH)/configs/idc/uinput-goodix.idc:$(TARGET_COPY_OUT_SYSTEM)/usr/idc/uinput-goodix.idc \
    $(LOCAL_PATH)/configs/keylayout/uinput-fpc.kl:$(TARGET_COPY_OUT_SYSTEM)/usr/keylayout/uinput-fpc.kl \
    $(LOCAL_PATH)/configs/keylayout/uinput-goodix.kl:$(TARGET_COPY_OUT_SYSTEM)/usr/keylayout/uinput-goodix.kl

# IPACM
PRODUCT_PACKAGES += \
    ipacm \
    IPACM_cfg.xml \
    IPACM_Filter_cfg.xml \
    libipanat \
    liboffloadhal

# IR
PRODUCT_PACKAGES += \
    android.hardware.ir@1.0-impl \
    android.hardware.ir@1.0-service

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.consumerir.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.consumerir.xml

# JSON
PRODUCT_PACKAGES += \
    libjson \
    libjsoncpp \
    libjsoncpp_ndk \
    libjsoncpp.vendor

# Keymaster
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.keystore.app_attest_key.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.keystore.app_attest_key.xml

# Lineage Health
PRODUCT_PACKAGES += \
    vendor.lineage.health-service.default

# Logging
SPAMMY_LOG_TAGS := \
    MiStcImpl \
    SDM \
    SDM-histogram \
    SRE \
    WifiHAL \
    cnss-daemon \
    libcitsensorservice@2.0-impl \
    libsensor-displayalgo \
    libsensor-parseRGB \
    libsensor-ssccalapi \
    sensors \
    vendor.qti.hardware.display.composer-service \
    vendor.xiaomi.sensor.citsensorservice@2.0-service

    ifneq ($(TARGET_BUILD_VARIANT),eng)
PRODUCT_VENDOR_PROPERTIES += \
    $(foreach tag,$(SPAMMY_LOG_TAGS),log.tag.$(tag)=E)
endif

# Media
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_video.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video_le.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media/codecs/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_codecs_c2_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_c2_audio.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_codecs_cape_vendor.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_cape_vendor.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_codecs_performance_cape_vendor.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance_cape_vendor.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_codecs_performance_taro_vendor.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance_taro_vendor.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_codecs_system_default.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_system_default.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_codecs_taro_vendor.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_taro_vendor.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_codecs_vendor_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_vendor_audio.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_profiles.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_profiles_cape.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_cape.xml \
    $(LOCAL_PATH)/configs/media/codecs/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml

PRODUCT_PACKAGES += \
    android.hardware.media.c2@1.0.vendor \
    libavservices_minijail \
    libavservices_minijail_vendor \
    libavservices_minijail.vendor \
    libcodec2_hidl@1.0.vendor \
    libcodec2_hidl@1.1.vendor \
    libcodec2_hidl@1.2.vendor \
    libcodec2_soft_common.vendor \
    libcodec2_vndk.vendor \
    libpalclient \
    libsfplugin_ccodec_utils.vendor \
    libopus.vendor \
    libstagefright_amrnb_common.vendor \
    libstagefright_enc_common.vendor \
    libstagefright_softomx_plugin.vendor \
    libstagefright_softomx.vendor \
    libstagefright_omx.vendor \
    libvorbisidec.vendor \
    libvpx.vendor

PRODUCT_PACKAGES += \
    init.qti.media.rc \
    init.qti.media.sh

# Misc
PRODUCT_PACKAGES += \
    libnetutils.vendor \
    libpng.vendor \
    libssl.vendor \
    libcurl.vendor \
    libsqlite.vendor \
    liblz4.vendor

# misc 
PRODUCT_PACKAGES += \
    vendor.xiaomi.hardware.mlipay@1.1.vendor \
    vendor.xiaomi.hardware.mtdservice@1.2.vendor

# Network
PRODUCT_PACKAGES += \
    android.system.net.netd@1.1.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml

# Neural networks
PRODUCT_PACKAGES += \
    android.hardware.neuralnetworks-V1-ndk_platform.vendor

# NFC
PRODUCT_PACKAGES += \
    android.hardware.nfc@1.2.vendor \
    libchrome.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.ese.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.ese.xml \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/android.hardware.nfc.uicc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.uicc.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.android.nfc_extras.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.nxp.mifare.xml

# Overlays
PRODUCT_PACKAGES += \
    FrameworksResOverlayDiting \
    FrameworksResOverlayDitingGlobal \
    SettingsOverlayDiting \
    SystemUIOverlayDiting \

PRODUCT_PACKAGES += \
    AospWifiResOverlayDitingChina \
    AospWifiResOverlayDitingGlobal \
    SettingsOverlayGlobal \
    SettingsOverlayChina \
    SettingsProviderOverlayChina \
    SettingsProviderOverlayGlobal

# Overlays
PRODUCT_PACKAGES += \
    CarrierConfigResCommon \
    DitingNfcOverlay \
    FrameworksResCommon \
    SettingsResCommon \
    SystemUIResCommon \
    TelecommResCommon \
    TelephonyResCommon \
    WifiResCommon

PRODUCT_PACKAGES += \
    ApertureResOverlay \
    FrameworksResTarget_Vendor \
    FrameworksResXiaomi \
    SettingsProviderResXiaomi \
    UpdaterOverlay \
    WifiResTarget \
    WifiResTarget_spf

DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay-lineage

# Overlay
PRODUCT_PACKAGES += \
    DeviceAsWebcamResXiaomi

# Parts
PRODUCT_PACKAGES += \
    XiaomiDolby \
    XiaomiParts

# Partitions
PRODUCT_BUILD_SUPER_PARTITION := false
PRODUCT_USE_DYNAMIC_PARTITIONS := true

# Perf
PRODUCT_PACKAGES += \
    libpsi.vendor \
    libtflite \
    vendor.qti.hardware.perf@2.3 \
    vendor.qti.hardware.perf@2.3.vendor \
    vendor.qti.hardware.qspa-V1-ndk

# Power
PRODUCT_PACKAGES += \
    android.hardware.power-service-qti

PRODUCT_COPY_FILES += \
    vendor/qcom/opensource/power/config/taro/powerhint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/powerhint.xml

# Product
PRODUCT_USE_SCUDO := true

# QTI
PRODUCT_PACKAGES += \
    libvndfwk_detect_jni.qti_vendor \
    libqti_vndfwk_detect.vendor \
    vendor.qti.hardware.systemhelper@1.0.vendor

# QXR
PRODUCT_PACKAGES += \
    android.hardware.common-V2-ndk.vendor

# RenderScript
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# RIL
PRODUCT_PACKAGES += \
    android.hardware.radio.config@1.3.vendor \
    android.hardware.radio.deprecated@1.0.vendor \
    android.hardware.radio-V1-ndk.vendor \
    android.hardware.radio.config-V1-ndk.vendor \
    android.hardware.radio.data-V1-ndk.vendor \
    android.hardware.radio.messaging-V1-ndk.vendor \
    android.hardware.radio.modem-V1-ndk.vendor \
    android.hardware.radio.network-V1-ndk.vendor \
    android.hardware.radio.sim-V1-ndk.vendor \
    android.hardware.radio.voice-V1-ndk.vendor \

# Protobuf CPP
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full-3.9.1-vendorcompat \
    libprotobuf-cpp-lite-3.9.1-vendorcompat

PRODUCT_PACKAGES += \
    Ims \
    QtiTelephony

# Rootdir
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/bin/init.class_main.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.class_main.sh \
    $(LOCAL_PATH)/rootdir/bin/init.kernel.post_boot-cape.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.kernel.post_boot-cape.sh \
    $(LOCAL_PATH)/rootdir/bin/init.kernel.post_boot-diwali.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.kernel.post_boot-diwali.sh \
    $(LOCAL_PATH)/rootdir/bin/init.kernel.post_boot-taro.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.kernel.post_boot-taro.sh \
    $(LOCAL_PATH)/rootdir/bin/init.kernel.post_boot.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.kernel.post_boot.sh \
    $(LOCAL_PATH)/rootdir/bin/init.qcom.class_core.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.qcom.class_core.sh \
    $(LOCAL_PATH)/rootdir/bin/init.qcom.early_boot.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.qcom.early_boot.sh \
    $(LOCAL_PATH)/rootdir/bin/init.qcom.post_boot.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.qcom.post_boot.sh \
    $(LOCAL_PATH)/rootdir/bin/init.qcom.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.qcom.sh \
    $(LOCAL_PATH)/rootdir/bin/init.qti.kernel.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.qti.kernel.sh \
    $(LOCAL_PATH)/rootdir/bin/init.qti.write.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.qti.write.sh \
    $(LOCAL_PATH)/rootdir/bin/init.xiaomi_sm8475.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.xiaomi_sm8475.sh \
    $(LOCAL_PATH)/rootdir/bin/vendor_modprobe.sh:$(TARGET_COPY_OUT_VENDOR)/bin/vendor_modprobe.sh \
    $(LOCAL_PATH)/rootdir/etc/init.qcom.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.qcom.rc \
    $(LOCAL_PATH)/rootdir/etc/init.qti.kernel.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.qti.kernel.rc \
    $(LOCAL_PATH)/rootdir/etc/init.recovery.qcom.rc:$(TARGET_COPY_OUT_RECOVERY)/root/init.recovery.qcom.rc \
    $(LOCAL_PATH)/rootdir/etc/init.sm8475.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.sm8475.rc \
    $(LOCAL_PATH)/rootdir/etc/init.target.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.target.rc \
    $(LOCAL_PATH)/rootdir/etc/init.xiaomi.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.xiaomi.rc \
    $(LOCAL_PATH)/rootdir/etc/ueventd-odm.rc:$(TARGET_COPY_OUT_ODM)/etc/ueventd.rc \
    $(LOCAL_PATH)/rootdir/etc/ueventd.qcom.rc:$(TARGET_COPY_OUT_VENDOR)/etc/ueventd.rc

# Secure element
PRODUCT_PACKAGES += \
    android.hardware.secure_element@1.2.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.se.omapi.ese.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.se.omapi.ese.xml \
    frameworks/native/data/etc/android.hardware.se.omapi.uicc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.se.omapi.uicc.xml

# Sensors
PRODUCT_PACKAGES += \
    android.hardware.sensors@2.1 \
    android.hardware.sensors@2.0-ScopedWakelock \
    android.hardware.sensors@2.0-ScopedWakelock.vendor \
    android.frameworks.sensorservice@1.0 \
    android.frameworks.sensorservice@1.0.vendor \
    android.hardware.sensors-service.xiaomi-multihal \

PRODUCT_PACKAGES += \
    sensors.xiaomi

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/sensors/hals.conf:$(TARGET_COPY_OUT_ODM)/etc/sensors/hals.conf

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_cape/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_cape/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_cape/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_cape/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_cape/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_cape/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_cape/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_diwali/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_diwali/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_diwali/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_diwali/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_diwali/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_diwali/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_diwali/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_taro/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_taro/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_taro/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_taro/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_taro/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_taro/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_taro/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_ukee/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_ukee/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_ukee/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_ukee/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_ukee/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_ukee/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/sku_ukee/android.hardware.sensor.stepdetector.xml

# Servicetracker
PRODUCT_PACKAGES += \
    vendor.qti.hardware.servicetracker@1.2.vendor

# Shipping API
PRODUCT_SHIPPING_API_LEVEL := 31

# Shims
PRODUCT_PACKAGES += \
    libui_shim \
    libui_shim.vendor

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH) \
    hardware/xiaomi \
    kernel/xiaomi/sm8450 \
    kernel/xiaomi/sm8450-modules

# Target platform
TARGET_BOARD_PLATFORM := taro

# Telephony
PRODUCT_PACKAGES += \
    extphonelib \
    extphonelib-product \
    extphonelib.xml \
    extphonelib_product.xml \
    ims-ext-common \
    ims_ext_common.xml \
    qti-telephony-hidl-wrapper \
    qti-telephony-hidl-wrapper-prd \
    qti_telephony_hidl_wrapper.xml \
    qti_telephony_hidl_wrapper_prd.xml \
    qti-telephony-utils \
    qti-telephony-utils-prd \
    qti_telephony_utils.xml \
    qti_telephony_utils_prd.xml \
    telephony-ext \
    libsysutils.vendor \
    XiaomiEuicc

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.ims.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.ims.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.telephony.euicc.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/android.hardware.telephony.euicc.xml \
    device/xiaomi/diting/rootdir/etc/init.diting.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.diting.rc

# Thermal
PRODUCT_PACKAGES += \
    android.hardware.thermal@2.0 \
    android.hardware.thermal@2.0.vendor

# Touchscreen
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml

PRODUCT_PACKAGES += \
    vendor.xperience.touch@1.0-service.diting

# USB
PRODUCT_HAS_GADGET_HAL := true
PRODUCT_PACKAGES += \
    android.hardware.usb-service.qti \
    libusbhost.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml

# Update engine
PRODUCT_PACKAGES += \
    update_engine \
    update_engine_sideload \
    update_verifier

PRODUCT_PACKAGES_DEBUG += \
    update_engine_client

# Vendor service manager
PRODUCT_PACKAGES += \
    vndservicemanager

# Verified boot
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.verified_boot.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.verified_boot.xml

# Vibrator
PRODUCT_PACKAGES += \
    vendor.qti.hardware.vibrator.service

PRODUCT_COPY_FILES += \
    vendor/qcom/opensource/vibrator/excluded-input-devices.xml:$(TARGET_COPY_OUT_VENDOR)/etc/excluded-input-devices.xml

# WiFi
PRODUCT_PACKAGES += \
    android.hardware.wifi-service \
    android.hardware.wifi.hostapd@1.0.vendor \
    hostapd \
    libwpa_client \
    libwifi-hal-ctrl \
    libwifi-hal-qcom \
    wpa_cli \
    wpa_supplicant \
    wpa_supplicant.conf \
    libwifi-system-iface.vendor

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.aware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.aware.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.wifi.rtt.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.rtt.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/wifi/icm.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/icm.conf \
    $(LOCAL_PATH)/configs/wifi/passpointProfile.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/passpointProfile.conf \
    $(LOCAL_PATH)/configs/wifi/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf \
    $(LOCAL_PATH)/configs/wifi/vendor_cmd.xml:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/vendor_cmd.xml \
    $(LOCAL_PATH)/configs/wifi/qca6490/WCNSS_qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/qca6490/WCNSS_qcom_cfg.ini \
    $(LOCAL_PATH)/configs/wifi/qca6750/WCNSS_qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/qca6750/WCNSS_qcom_cfg.ini \
    $(LOCAL_PATH)/configs/wifi/WCNSS_qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/WCNSS_qcom_cfg.ini \
    $(LOCAL_PATH)/configs/wifi/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf

# WiFi Display
PRODUCT_PACKAGES += \
    libprocessgroup.vendor \
    libnl \
    libnl.vendor \
    libwfdaac_vendor
