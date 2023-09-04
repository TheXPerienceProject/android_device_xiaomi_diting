/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <libinit_dalvik_heap.h>
#include <libinit_variant.h>
#include <libinit_utils.h>

#include "vendor_init.h"

#define FINGERPRINT_GL "Xiaomi/diting/diting:13/SKQ1.220303.001/V14.0.2.0.TLFMIXM:user/release-keys"
#define FINGERPRINT_CN "Xiaomi/diting/diting:13/SKQ1.220303.001/V14.0.10.0.TLFCNXM:user/release-keys"
#define FINGERPRINT_JP "Xiaomi/ditingp_jp/diting:12/SKQ1.220303.001/V13.0.2.0.SLFJPXM:user/release-keys"
#define FINGERPRINT_RU "Xiaomi/ditingp_ru/diting:13/SKQ1.220303.001/V14.0.4.0.TLFRUXM:user/release-keys"

static const variant_info_t diting_global_info = {
    .hwc_value = "GL",
    .sku_value = "",

    .brand = "Xiaomi",
    .device = "diting",
    .marketname = "Xiaomi 12T Pro",
    .model = "22081212G",
    .mod_device = "diting_global",
    .build_fingerprint = FINGERPRINT_GL,
};

static const variant_info_t diting_info = {
    .hwc_value = "CN",
    .sku_value = "",

    .brand = "Xiaomi",
    .device = "diting",
    .marketname = "Redmi K50 Ultra",
    .model = "22081212C",
    .mod_device = "diting",
    .build_fingerprint = FINGERPRINT_CN,
};

static const variant_info_t ditingp_info = {
    .hwc_value = "JP",
    .sku_value = "",

    .brand = "Xiaomi",
    .device = "diting",
    .marketname = "Xiaomi 12T Pro",
    .model = "22081212R",
    .mod_device = "ditingp_jp",
    .build_fingerprint = FINGERPRINT_JP,
};

static const variant_info_t diting_ru_info = {
    .hwc_value = "RU",
    .sku_value = "",

    .brand = "Xiaomi",
    .device = "diting",
    .marketname = "Xiaomi 12T Pro",
    .model = "22081212UG",
    .mod_device = "ditingp_ru",
    .build_fingerprint = FINGERPRINT_RU,
};

static const std::vector<variant_info_t> variants = {
    diting_info,
    diting_global_info,
    diting_ru_info,
    ditingp_info,
};

void vendor_load_properties() {
    set_dalvik_heap();
    search_variant(variants);

    // SafetyNet workaround
    property_override("ro.boot.verifiedbootstate", "green");

    // SF
    property_override("vendor.display.enable_fb_scaling", "0");
}
