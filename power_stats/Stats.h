/*
 * Copyright (C) 2025 The XPerience Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/power/stats/BnPowerStats.h>
#include <android-base/logging.h>
#include <log/log.h>

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace stats {

// SoC type enumeration for Snapdragon platforms
enum class SocType {
    SM8350,  // Snapdragon 888/888+
    SM8450,  // Snapdragon 8 Gen 1
    SM8550,  // Snapdragon 8 Gen 2  
    SM8650,  // Snapdragon 8 Gen 3
    SM8750,  // Snapdragon 8 Elite
    SM7325,  // Snapdragon 778G+
    UNKNOWN  // Unknown or unsupported platform
};

/**
 * Xiaomi-specific Power Stats AIDL service implementation for Snapdragon platforms
 * Provides real hardware statistics for SM8350, SM8450, SM8550, SM8650, SM8750
 * Uses ro.board.platform and ro.soc.model for precise SoC detection
 * Implements AIDL IPowerStats interface version 2
 */
class Stats : public BnPowerStats {
public:
    Stats() = default;
    ~Stats() = default;

    // Methods from aidl::android::hardware::power::stats::IPowerStats
    ndk::ScopedAStatus getEnergyConsumerInfo(std::vector<EnergyConsumer>* _aidl_return) override;
    ndk::ScopedAStatus getEnergyConsumed(const std::vector<int32_t>& in_energyConsumerIds,
                                         std::vector<EnergyConsumerResult>* _aidl_return) override;
    ndk::ScopedAStatus getPowerEntityInfo(std::vector<PowerEntity>* _aidl_return) override;
    ndk::ScopedAStatus getStateResidency(const std::vector<int32_t>& in_powerEntityIds,
                                         std::vector<StateResidencyResult>* _aidl_return) override;
    ndk::ScopedAStatus getEnergyMeterInfo(std::vector<Channel>* _aidl_return) override;
    ndk::ScopedAStatus readEnergyMeter(const std::vector<int32_t>& in_channelIds,
                                       std::vector<EnergyMeasurement>* _aidl_return) override;
};

}  // namespace stats
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
