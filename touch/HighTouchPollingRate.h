/*
 * Copyright (C) 2022 The LineageOS Project
 * Copyright (C) 2011-2023-2025 The XPerience Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vendor/lineage/touch/1.0/IHighTouchPollingRate.h>
#include <string>
#include <thread>

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

using ::android::hardware::Return;

class HighTouchPollingRate : public IHighTouchPollingRate {
public:
    HighTouchPollingRate();
    // Métodos de ::vendor::lineage::touch::V1_0::IHighTouchPollingRate.
    Return<bool> isEnabled() override;
    Return<bool> setEnabled(bool enabled) override;

private:
    //std::string FindSysfsPath(const std::string& attribute_name);
    void monitorPollingRate();
    int getCurrentPollingRate();
    std::vector<std::string> getTouchEvents();
    std::thread monitoringThread;
    bool isScreenOn(); // Display status detection function
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
