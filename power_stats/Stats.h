/*
 * Copyright (C) 2025 The XPerience Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/stats/BnStats.h>
#include <android-base/logging.h>

namespace aidl {
namespace android {
namespace hardware {
namespace stats {

/**
 * Xiaomi-specific Stats AIDL service implementation
 * Provides system statistics for Xiaomi devices including CPU, GPU, memory, thermal and power data
 */
class Stats : public BnStats {
public:
    Stats() = default;
    ~Stats() = default;

    /**
     * Retrieves CPU statistics including tick counts for various states
     * @param[out] _aidl_return CpuStats structure filled with current CPU metrics
     * @return ndk::ScopedAStatus OK on success, error otherwise
     */
    ndk::ScopedAStatus getCpuStats(CpuStats* _aidl_return) override;

    /**
     * Retrieves GPU utilization and frequency statistics
     * @param[out] _aidl_return GpuStats structure with GPU performance data
     * @return ndk::ScopedAStatus OK on success, error otherwise
     */
    ndk::ScopedAStatus getGpuStats(GpuStats* _aidl_return) override;

    /**
     * Retrieves system memory usage statistics
     * @param[out] _aidl_return MemoryStats structure with memory allocation data
     * @return ndk::ScopedAStatus OK on success, error otherwise
     */
    ndk::ScopedAStatus getMemoryStats(MemoryStats* _aidl_return) override;

    /**
     * Retrieves thermal sensor readings from device sensors
     * @param[out] _aidl_return ThermalStats structure with temperature data
     * @return ndk::ScopedAStatus OK on success, error otherwise
     */
    ndk::ScopedAStatus getThermalStats(ThermalStats* _aidl_return) override;

    /**
     * Retrieves power-related statistics including battery information
     * @param[out] _aidl_return PowerStats structure with power consumption data
     * @return ndk::ScopedAStatus OK on success, error otherwise
     */
    ndk::ScopedAStatus getPowerStats(PowerStats* _aidl_return) override;

private:
    /**
     * Internal method to read CPU statistics from kernel interfaces
     * @param[out] stats CpuStats structure to populate
     * @return bool true if successful, false otherwise
     */
    bool readCpuStats(CpuStats* stats);

    /**
     * Internal method to read GPU statistics from vendor-specific interfaces
     * @param[out] stats GpuStats structure to populate
     * @return bool true if successful, false otherwise
     */
    bool readGpuStats(GpuStats* stats);

    /**
     * Internal method to read memory statistics from /proc/meminfo
     * @param[out] stats MemoryStats structure to populate
     * @return bool true if successful, false otherwise
     */
    bool readMemoryStats(MemoryStats* stats);

    /**
     * Internal method to read thermal statistics from thermal zones
     * @param[out] stats ThermalStats structure to populate
     * @return bool true if successful, false otherwise
     */
    bool readThermalStats(ThermalStats* stats);

    /**
     * Internal method to read power statistics from power supply subsystem
     * @param[out] stats PowerStats structure to populate
     * @return bool true if successful, false otherwise
     */
    bool readPowerStats(PowerStats* stats);
};

}  // namespace stats
}  // namespace hardware
}  // namespace android
}  // namespace aidl