#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixup_remove,
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'hardware/qcom-caf/sm8450',
    'hardware/qcom-caf/wlan',
    'hardware/xiaomi',
    'vendor/qcom/opensource/commonsys/display',
    'vendor/qcom/opensource/commonsys-intf/display',
    'vendor/qcom/opensource/dataservices',
]


def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None


lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (
        'libmisight',
        'libQnnHtp',
        'libQnnHtpV69Stub',
        'vendor.qti.hardware.dpmservice@1.0',
        'vendor.qti.hardware.dpmservice@1.1',
        'vendor.qti.hardware.qccsyshal@1.0',
        'vendor.qti.hardware.qccsyshal@1.1',
        'vendor.qti.hardware.qccvndhal@1.0',
        'vendor.qti.imsrtpservice@3.0',
        'vendor.qti.diaghal@1.0',
        'vendor.qti.hardware.iop@2.0',
        'vendor.qti.hardware.wifidisplaysession@1.0',
        'vendor.qti.qspmhal@1.0',
        'vendor.qti.qspmhal-V1-ndk',
        'vendor.xiaomi.hardware.misight-V1-ndk_platform',
        'vendor.xiaomi.hardware.misys@1.0.so',
        'vendor.xiaomi.hardware.misys@2.0.so',
        'vendor.xiaomi.hardware.misys@3.0.so',
        'vendor.xiaomi.hardware.misys@4.0.so',
        'vendor.qti.hardware.display.composer-service.xml',
        'com.qualcomm.qti.dpm.api@1.0',
    ): lib_fixup_vendor_suffix,
    (
        'cnss_diag',
        'libwpa_client'
        'libagmclient',
        'libagmmixer',
        'vendor.qti.hardware.pal@1.0-impl',
        'libadreno_app_profiles',
    ): lib_fixup_remove,
}

blob_fixups: blob_fixups_user_type = {
    'system_ext/lib64/libwfdmmsrc_system.so': blob_fixup()
        .add_needed('libgui_shim.so'),
    'system_ext/lib64/libwfdnative.so': blob_fixup()
        .add_needed('libbinder_shim.so')
        .add_needed('libinput_shim.so'),
    'system_ext/lib64/libwfdservice.so': blob_fixup()
        .replace_needed(
            'android.media.audio.common.types-V2-cpp.so',
            'android.media.audio.common.types-V4-cpp.so',
        ),
    ('vendor/etc/camera/diting_enhance_motiontuning.xml','vendor/etc/camera/diting_motiontuning.xml',): blob_fixup()
        .regex_replace('xml=version', 'xml version'),
    'vendor/etc/camera/pureView_parameter.xml': blob_fixup()
        .regex_replace(r'=([0-9]+)>', r'="\1">'),
    (
        'vendor/bin/hw/android.hardware.security.keymint-service-qti',
        'vendor/lib64/libqtikeymint.so',
    ): blob_fixup()
        .replace_needed(
            'android.hardware.security.keymint-V1-ndk_platform.so',
            'android.hardware.security.keymint-V1-ndk.so',
        )
        .replace_needed(
            'android.hardware.security.secureclock-V1-ndk_platform.so',
            'android.hardware.security.secureclock-V1-ndk.so',
        )
        .replace_needed(
            'android.hardware.security.sharedsecret-V1-ndk_platform.so',
            'android.hardware.security.sharedsecret-V1-ndk.so',
        )
        .add_needed('android.hardware.security.rkp-V3-ndk.so'),
    ('vendor/lib/c2.dolby.client.so', 'vendor/lib64/c2.dolby.client.so'): blob_fixup()
        .add_needed('dolbycodec_shim.so'),
    'vendor/lib64/hw/displayfeature.default.so': blob_fixup()
        .replace_needed(
            'libstagefright_foundation.so',
            'libstagefright_foundation-v33.so',
        ),
    ('vendor/bin/hw/dolbycodec2', 'vendor/bin/hw/vendor.dolby.hardware.dms@2.0-service', 'vendor/bin/hw/vendor.dolby.media.c2@1.0-service'): blob_fixup()
        .add_needed('libstagefright_foundation-v33.so'),
    ('vendor/lib64/hw/audio.primary.taro.so', 'vendor/lib64/hw/audio.primary.taro.so'): blob_fixup()
        .replace_needed(
            'libstagefright_foundation.so',
            'libstagefright_foundation-v33.so',
        ),
    'vendor/lib/libcodec2_hidl@1.0_vendor.so': blob_fixup()
        .replace_needed('libcodec2_vndk.so', 'libcodec2_vndk_vendor.so',)
        .fix_soname(),
    'vendor/lib/libcodec2_vndk_vendor.so': blob_fixup()
        .fix_soname()
        .add_needed('libui_shim.so'),
    'vendor/lib/c2.dolby.client.so': blob_fixup()
        .replace_needed('libcodec2_vndk.so', 'libcodec2_vndk_vendor.so',)
        .replace_needed('libcodec2_hidl@1.0.so', 'libcodec2_hidl@1.0_vendor.so',),
    'vendor/etc/msm_irqbalance.conf': blob_fixup()
        .replace_needed('IGNORED_IRQ=27,23,38', 'IGNORED_IRQ=27,23,38,115,332'),
    'vendor/etc/seccomp_policy/atfwd@2.0.policy': blob_fixup()
        .add_line_if_missing('gettid: 1'),
    'vendor/lib64/vendor.libdpmframework.so': blob_fixup()
        .add_needed('libhidlbase_shim.so')
    'vendor/bin/hw/android.hardware.neuralnetworks-shim-service-qti': blob_fixup()
        .replace_needed('android.hardware.neuralnetworks-V1-ndk_platform.so', 'android.hardware.neuralnetworks-V1-ndk.so')
        .replace_needed('android.hardware.common-V2-ndk_platform.so', 'android.hardware.common-V2-ndk.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'diting',
    'xiaomi',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
