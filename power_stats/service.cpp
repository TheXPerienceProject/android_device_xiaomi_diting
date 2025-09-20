/*
 * Copyright (C) 2025 The XPerience Project
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * Main entry point for Xiaomi Stats AIDL Service
 * Registers the stats service with servicemanager and enters binder loop
 */

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android-base/logging.h>
#include <log/log.h>

#include "Stats.h"

using aidl::android::hardware::stats::Stats;

int main() {
    // Configure binder thread pool
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    
    // Create Xiaomi-specific stats service instance
    std::shared_ptr<Stats> statsService = ndk::SharedRefBase::make<Stats>();

    // Register service with instance name "xiaomi"
    const std::string instance = std::string() + Stats::descriptor + "/xiaomi";
    binder_status_t status = AServiceManager_addService(statsService->asBinder().get(), instance.c_str());
    
    if (status != STATUS_OK) {
        ALOGE("Failed to register Xiaomi stats service: %d", status);
        return 1;
    }

    ALOGI("Xiaomi Stats AIDL Service started successfully (instance: %s)", instance.c_str());
    
    // Enter binder thread pool (this call doesn't return)
    ABinderProcess_joinThreadPool();
    
    return 1; // Should not reach this point
}