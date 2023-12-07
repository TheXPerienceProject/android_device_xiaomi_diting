/*
 * Copyright (C) 2022 The LineageOS Project
 * Copyright (C) 2011-2023 The XPerience Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch@1.0-service.diting"

#include "HighTouchPollingRate.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <cutils/properties.h>
#include <fstream>
#include <iostream>
#include <sys/ioctl.h>

#define SET_CUR_VALUE 0
#define TOUCH_GAME_MODE 0
#define TOUCH_MAGIC 't'
#define TOUCH_IOC_SETMODE _IO(TOUCH_MAGIC, SET_CUR_VALUE)
#define TOUCH_DEV_PATH "/dev/xiaomi-touch"
#define TOUCH_ID 0

using android::base::GetBoolProperty;
using android::base::SetProperty;

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

const std::string kHighTouchPollingPath = "TOUCH_DEV_PATH";
static constexpr const char *kHighTouchPollingProp = "persist.vendor.htp_klzz_enabled";

HighTouchPollingRate::HighTouchPollingRate() {
    setEnabled(GetBoolProperty(kHighTouchPollingProp,SET_CUR_VALUE));
}

Return<bool> HighTouchPollingRate::isEnabled() {

   return  GetBoolProperty(kHighTouchPollingProp,SET_CUR_VALUE);

}

Return<bool> HighTouchPollingRate::setEnabled(bool enabled) {
    int fd = open(TOUCH_DEV_PATH, O_RDWR);
    int arg[3] = { TOUCH_ID, TOUCH_GAME_MODE, enabled ? 1 : 0};
    ioctl(fd, TOUCH_IOC_SETMODE, &arg);
    close(fd);
    return enabled;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
