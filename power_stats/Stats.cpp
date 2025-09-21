/*
 * Copyright (C) 2025 The XPerience Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Stats.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <android-base/file.h>
#include <android-base/strings.h>
#include <android-base/properties.h>
#include <ctime>
#include <log/log.h>

using android::base::ReadFileToString;
using android::base::Split;
using android::base::Trim;
using android::base::GetProperty;
using android::base::StartsWith;

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace stats {

// Common paths for Snapdragon
constexpr const char* kProcStat = "/proc/stat";
constexpr const char* kProcMeminfo = "/proc/meminfo";
constexpr const char* kGpuFreqPath = "/sys/class/kgsl/kgsl-3d0/gpuclk";
constexpr const char* kGpuBusyPath = "/sys/class/kgsl/kgsl-3d0/gpu_busy_percentage";
constexpr const char* kBatteryCapacity = "/sys/class/power_supply/battery/capacity";
constexpr const char* kBatteryCurrentNow = "/sys/class/power_supply/battery/current_now";
constexpr const char* kCpuFreqBase = "/sys/devices/system/cpu/cpu";

/**
 * Detects Snapdragon SoC type using platform and model properties
 * Uses ro.board.platform for code names and ro.soc.model for SM codes
 * 
 * @return Detected SocType enumeration
 */
SocType detectSocType() {
    std::string platform = GetProperty("ro.board.platform", "");
    std::string soc_model = GetProperty("ro.soc.model", "");
    
    ALOGD("Platform detection: platform='%s', soc_model='%s'", platform.c_str(), soc_model.c_str());

    // First try SOC model (most reliable)
    if (soc_model.find("SM8650") != std::string::npos) return SocType::SM8650;
    if (soc_model.find("SM8550") != std::string::npos) return SocType::SM8550;
    if (soc_model.find("SM8450") != std::string::npos) return SocType::SM8450;
    if (soc_model.find("SM8350") != std::string::npos) return SocType::SM8350;
    if (soc_model.find("SM8750") != std::string::npos) return SocType::SM8750;

    // Fallback to platform code names
    if (platform.find("pineapple") != std::string::npos) return SocType::SM8650;  // Snapdragon 8 Gen 3
    if (platform.find("kalama") != std::string::npos) return SocType::SM8550;     // Snapdragon 8 Gen 2
    if (platform.find("taro") != std::string::npos) return SocType::SM8450;       // Snapdragon 8 Gen 1
    if (platform.find("lahaina") != std::string::npos) return SocType::SM8350;    // Snapdragon 888
    if (platform.find("sun") != std::string::npos) return SocType::SM8750;        // Snapdragon 8 Elite

    ALOGW("Unknown SoC platform: %s, model: %s", platform.c_str(), soc_model.c_str());
    return SocType::UNKNOWN;
}

/**
 * Reads current CPU frequency for a specific core
 * 
 * @param cpu_id CPU core identifier (0-7 for big.LITTLE architectures)
 * @return Current frequency in Hz, 0 if unable to read
 */
int64_t readCpuFrequency(int cpu_id) {
    std::string freq_path = kCpuFreqBase + std::to_string(cpu_id) + "/cpufreq/scaling_cur_freq";
    std::string freq_str;
    
    if (ReadFileToString(freq_path, &freq_str)) {
        return std::stoll(Trim(freq_str)) * 1000; // Convert KHz to Hz
    }
    
    return 0;
}

/**
 * Reads GPU utilization percentage from KGSL interface
 * 
 * @return GPU utilization percentage (0-100), 0 if unable to read
 */
int32_t readGpuUsage() {
    std::string busy_str;
    if (ReadFileToString(kGpuBusyPath, &busy_str)) {
        return std::stoi(Trim(busy_str));
    }
    return 0;
}

/**
 * Reads current GPU frequency from KGSL interface
 * 
 * @return Current GPU frequency in Hz, 0 if unable to read
 */
int64_t readGpuFrequency() {
    std::string freq_str;
    if (ReadFileToString(kGpuFreqPath, &freq_str)) {
        return std::stoll(Trim(freq_str));
    }
    return 0;
}

/**
 * Scans all thermal zones and reads their current temperatures
 * 
 * @return Vector of temperature readings in millidegrees Celsius
 */
std::vector<int32_t> readThermalTemperatures() {
    std::vector<int32_t> temperatures;
    DIR* dir = opendir("/sys/class/thermal");
    
    if (!dir) {
        ALOGW("Failed to open thermal directory");
        return temperatures;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (StartsWith(entry->d_name, "thermal_zone")) {
            std::string temp_path = std::string("/sys/class/thermal/") + entry->d_name + "/temp";
            std::string temp_str;
            
            if (ReadFileToString(temp_path, &temp_str)) {
                temperatures.push_back(std::stoi(Trim(temp_str))); // in millidegrees Celsius
                ALOGD("Thermal zone %s: %s m°C", entry->d_name, temp_str.c_str());
            }
        }
    }
    closedir(dir);
    
    return temperatures;
}

/**
 * Reads current battery capacity percentage
 * 
 * @return Battery percentage (0-100), -1 if unable to read
 */
int32_t readBatteryCapacity() {
    std::string capacity_str;
    if (ReadFileToString(kBatteryCapacity, &capacity_str)) {
        return std::stoi(Trim(capacity_str));
    }
    return -1;
}

/**
 * Reads current battery current draw
 * 
 * @return Current in microamperes, 0 if unable to read
 */
int64_t readBatteryCurrent() {
    std::string current_str;
    if (ReadFileToString(kBatteryCurrentNow, &current_str)) {
        return std::stoll(Trim(current_str));
    }
    return 0;
}

// ==================== IPowerStats V2 Interface Implementation ====================

/**
 * Retrieves information about available energy consumers
 * Required by IPowerStats AIDL interface V2
 * 
 * @param[out] _aidl_return Vector of EnergyConsumer information
 * @return ndk::ScopedAStatus OK on success
 */
ndk::ScopedAStatus Stats::getEnergyConsumerInfo(std::vector<EnergyConsumer>* _aidl_return) {
    std::vector<EnergyConsumer> consumers;
    
    SocType soc = detectSocType();
    std::string soc_name;
    
    switch (soc) {
        case SocType::SM8350: soc_name = "SD888"; break;
        case SocType::SM8450: soc_name = "SD8Gen1"; break;
        case SocType::SM8550: soc_name = "SD8Gen2"; break;
        case SocType::SM8650: soc_name = "SD8Gen3"; break;
        case SocType::SM8750: soc_name = "SD8Elite"; break;
        default: soc_name = "Snapdragon";
    }
    
    // CPU energy consumer
    consumers.push_back({
        .id = 0,
        .ordinal = 0,
        .type = EnergyConsumerType::CPU_CLUSTER,
        .name = soc_name + " CPU"
    });
    
    // GPU energy consumer
    consumers.push_back({
        .id = 1,
        .ordinal = 0,
        .type = EnergyConsumerType::OTHER,
        .name = soc_name + " GPU"
    });
    
    // Mobile radio energy consumer
    consumers.push_back({
        .id = 2,
        .ordinal = 0,
        .type = EnergyConsumerType::MOBILE_RADIO,
        .name = soc_name + " Modem"
    });
    
    // WiFi energy consumer
    consumers.push_back({
        .id = 3,
        .ordinal = 0,
        .type = EnergyConsumerType::WIFI,
        .name = soc_name + " WiFi"
    });
    if (soc == SocType::SM8650 || soc == SocType::SM8750) {
        consumers.push_back({
            .id = 4,
            .ordinal = 1,
            .type = EnergyConsumerType::CPU_CLUSTER,
            .name = soc_name + " Prime Core"
        });

        consumers.push_back({
            .id = 5,
            .ordinal = 2,
            .type = EnergyConsumerType::CPU_CLUSTER,
            .name = soc_name + " Efficiency Cores"
        });
    }

    *_aidl_return = consumers;
    return ndk::ScopedAStatus::ok();
}

/**
 * Retrieves energy consumption data for specified consumers
 * TODO: Implement real energy estimation based on usage and frequency
 * 
 * @param in_energyConsumerIds List of consumer IDs to query
 * @param[out] _aidl_return Vector of EnergyConsumerResult data
 * @return ndk::ScopedAStatus OK on success
 */
ndk::ScopedAStatus Stats::getEnergyConsumed(const std::vector<int32_t>& in_energyConsumerIds,
                                           std::vector<EnergyConsumerResult>* _aidl_return) {
    std::vector<EnergyConsumerResult> results;
    int64_t current_time_ms = static_cast<int64_t>(std::time(nullptr)) * 1000;
    
    for (const auto& id : in_energyConsumerIds) {
        EnergyConsumerResult result = {
            .id = id,
            .timestampMs = current_time_ms,
            .energyUWs = 0, // TODO: Implement real energy estimation
            .attribution = {}
        };
        
        // Placeholder energy values based on consumer type
        switch (id) {
            case 0: // CPU
                result.energyUWs = 5000000; // 5 joules
                break;
            case 1: // GPU
                result.energyUWs = 3000000; // 3 joules
                break;
            case 2: // Modem
                result.energyUWs = 1000000; // 1 joule
                break;
            case 3: // WiFi
                result.energyUWs = 800000; // 0.8 joules
                break;
        }
        
        results.push_back(result);
    }
    
    *_aidl_return = results;
    return ndk::ScopedAStatus::ok();
}

/**
 * Retrieves information about available power entities
 * Maps to the hardware monitoring functions you implemented
 * 
 * @param[out] _aidl_return Vector of PowerEntity information
 * @return ndk::ScopedAStatus OK on success
 */
ndk::ScopedAStatus Stats::getPowerEntityInfo(std::vector<PowerEntity>* _aidl_return) {
    std::vector<PowerEntity> entities;
    
    SocType soc = detectSocType();
    std::string soc_prefix;
    
    switch (soc) {
        case SocType::SM8350: soc_prefix = "SD888_"; break;
        case SocType::SM8450: soc_prefix = "SD8G1_"; break;
        case SocType::SM8550: soc_prefix = "SD8G2_"; break;
        case SocType::SM8650: soc_prefix = "SD8G3_"; break;
        case SocType::SM8750: soc_prefix = "SD8E_"; break;
        default: soc_prefix = "SD_";
    }
    
    // CPU power entity
    entities.push_back({
        .id = 0,
        .name = soc_prefix + "CPU",
        .states = {
            {.id = 0, .name = "active"},
            {.id = 1, .name = "idle"},
            {.id = 2, .name = "sleep"},
            {.id = 3, .name = "off"}
        }
    });
    
    // GPU power entity
    entities.push_back({
        .id = 1,
        .name = soc_prefix + "GPU",
        .states = {
            {.id = 0, .name = "active"},
            {.id = 1, .name = "idle"},
            {.id = 2, .name = "sleep"}
        }
    });
    
    // Memory power entity
    entities.push_back({
        .id = 2,
        .name = soc_prefix + "MEM",
        .states = {
            {.id = 0, .name = "active"},
            {.id = 1, .name = "self_refresh"},
            {.id = 2, .name = "power_down"}
        }
    });
    
    // Modem power entity
    entities.push_back({
        .id = 3,
        .name = soc_prefix + "MODEM",
        .states = {
            {.id = 0, .name = "active"},
            {.id = 1, .name = "idle"},
            {.id = 2, .name = "sleep"}
        }
    });
    
    *_aidl_return = entities;
    return ndk::ScopedAStatus::ok();
}

/**
 * Retrieves state residency data for specified power entities
 * TODO: Implement real state residency tracking from kernel stats
 * 
 * @param in_powerEntityIds List of power entity IDs to query
 * @param[out] _aidl_return Vector of StateResidencyResult data
 * @return ndk::ScopedAStatus OK on success
 */
ndk::ScopedAStatus Stats::getStateResidency(const std::vector<int32_t>& in_powerEntityIds,
                                           std::vector<StateResidencyResult>* _aidl_return) {
    std::vector<StateResidencyResult> results;
    int64_t current_time_ms = static_cast<int64_t>(std::time(nullptr)) * 1000;
    
    for (const auto& id : in_powerEntityIds) {
        StateResidencyResult result = {.id = id};
        
        // Placeholder state residency data
        switch (id) {
            case 0: // CPU
                result.stateResidencyData = {
                    {
                        .id = 0, // active
                        .totalTimeInStateMs = 60000,
                        .totalStateEntryCount = 120,
                        .lastEntryTimestampMs = current_time_ms - 5000
                    },
                    {
                        .id = 1, // idle
                        .totalTimeInStateMs = 120000,
                        .totalStateEntryCount = 80,
                        .lastEntryTimestampMs = current_time_ms - 2000
                    }
                };
                break;
                
            case 1: // GPU
                result.stateResidencyData = {
                    {
                        .id = 0, // active
                        .totalTimeInStateMs = 30000,
                        .totalStateEntryCount = 45,
                        .lastEntryTimestampMs = current_time_ms - 3000
                    },
                    {
                        .id = 1, // idle
                        .totalTimeInStateMs = 150000,
                        .totalStateEntryCount = 60,
                        .lastEntryTimestampMs = current_time_ms - 1000
                    }
                };
                break;
                
            default:
                // Default placeholder for other entities
                result.stateResidencyData = {
                    {
                        .id = 0,
                        .totalTimeInStateMs = 10000,
                        .totalStateEntryCount = 25,
                        .lastEntryTimestampMs = current_time_ms - 5000
                    }
                };
        }
        
        results.push_back(result);
    }
    
    *_aidl_return = results;
    return ndk::ScopedAStatus::ok();
}

/**
 * Retrieves energy meter information (placeholder - not implemented)
 * 
 * @param[out] _aidl_return Vector of Channel information
 * @return ndk::ScopedAStatus OK with empty list
 */
ndk::ScopedAStatus Stats::getEnergyMeterInfo(std::vector<Channel>* _aidl_return) {
    // Return empty list - energy meter not implemented
    *_aidl_return = {};
    return ndk::ScopedAStatus::ok();
}

/**
 * Reads energy meter data (placeholder - not implemented)
 * 
 * @param in_channelIds List of channel IDs to read
 * @param[out] _aidl_return Vector of EnergyMeasurement data
 * @return ndk::ScopedAStatus OK with empty list
 */
ndk::ScopedAStatus Stats::readEnergyMeter(const std::vector<int32_t>& in_channelIds,
                                         std::vector<EnergyMeasurement>* _aidl_return) {
    // Return empty list - energy meter not implemented
    *_aidl_return = {};
    return ndk::ScopedAStatus::ok();
}

}  // namespace stats
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl