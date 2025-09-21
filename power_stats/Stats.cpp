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
#include <map>
#include <ctime>
#include <android-base/file.h>
#include <android-base/strings.h>
#include <android-base/properties.h>
#include <log/log.h>

using android::base::ReadFileToString;
using android::base::Trim;
using android::base::GetProperty;
using android::base::StartsWith;

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace stats {

// ==================== Constants ====================

constexpr const char* kProcStat = "/proc/stat";
constexpr const char* kProcMeminfo = "/proc/meminfo";
constexpr const char* kGpuFreqPath = "/sys/class/kgsl/kgsl-3d0/gpuclk";
constexpr const char* kGpuBusyPath = "/sys/class/kgsl/kgsl-3d0/gpu_busy_percentage";
constexpr const char* kBatteryCapacity = "/sys/class/power_supply/battery/capacity";
constexpr const char* kBatteryCurrentNow = "/sys/class/power_supply/battery/current_now";
constexpr const char* kCpuFreqBase = "/sys/devices/system/cpu/cpu";

// Possible CPU idle time paths
const std::vector<std::string> kCpuIdleTimePaths = {
    "/sys/devices/system/cpu/cpu%d/cpuidle/state%d/time",
    "/sys/devices/system/cpu/cpu%d/cpuidle/state%d/residency",
    "/sys/devices/system/cpu/cpuidle/state%d/time"
};

// ==================== Utility Functions ====================

/**
 * @brief Read the first readable file from a list of paths
 */
std::string readFirstAvailable(const std::vector<std::string>& paths) {
    for (const auto& path : paths) {
        std::string content;
        if (ReadFileToString(path, &content)) {
            return Trim(content);
        }
    }
    return "";
}

/**
 * @brief Reads int64_t from sysfs file, returns -1 if failed
 */
static int64_t readInt64File(const std::string& path) {
    std::string data;
    if (ReadFileToString(path, &data)) {
        try {
            return std::stoll(Trim(data));
        } catch (...) { return -1; }
    }
    return -1;
}

/**
 * @brief Reads CPU idle time for a specific core and state
 */
int64_t readCpuIdleTime(int cpu_id, int state_id) {
    for (const auto& pattern : kCpuIdleTimePaths) {
        char path[256];
        snprintf(path, sizeof(path), pattern.c_str(), cpu_id, state_id);
        std::string time_str;
        if (ReadFileToString(path, &time_str)) {
            try { return std::stoll(Trim(time_str)); }
            catch (const std::exception& e) {
                ALOGW("Failed to parse CPU idle time from %s: %s", path, e.what());
            }
        }
    }
    return 0;
}

/**
 * @brief Reads CPU frequency for a specific core
 */
int64_t readCpuFrequency(int cpu_id) {
    std::string freq_path = kCpuFreqBase + std::to_string(cpu_id) + "/cpufreq/scaling_cur_freq";
    std::string freq_str;
    if (ReadFileToString(freq_path, &freq_str)) {
        return std::stoll(Trim(freq_str)) * 1000; // KHz -> Hz
    }
    return 0;
}

/**
 * @brief Reads GPU busy percentage
 */
int32_t readGpuUsage() {
    std::string busy_str;
    if (ReadFileToString(kGpuBusyPath, &busy_str)) return std::stoi(Trim(busy_str));
    return 0;
}

/**
 * @brief Reads GPU frequency
 */
int64_t readGpuFrequency() {
    std::string freq_str;
    if (ReadFileToString(kGpuFreqPath, &freq_str)) return std::stoll(Trim(freq_str));
    return 0;
}

/**
 * @brief Reads battery voltage in µV
 */
int64_t readBatteryVoltage() {
    int64_t voltage = readInt64File("/sys/class/power_supply/battery/voltage_now");
    if (voltage <= 0) voltage = 3700000; // fallback 3.7V
    return voltage;
}

/**
 * @brief Reads battery current in µA
 */
int64_t readBatteryCurrent() {
    int64_t current = readInt64File("/sys/class/power_supply/battery/current_now");
    if (current <= 0) current = 1000; // fallback 1mA
    return current;
}

/**
 * @brief Reads battery power in µW
 */
int64_t readBatteryPower() {
    int64_t power = readInt64File("/sys/class/power_supply/battery/power_now");
    if (power > 0) return power;

    int64_t voltage = readBatteryVoltage();
    int64_t current = readBatteryCurrent();
    return (voltage * current) / 1000000LL; // µW
}

/**
 * @brief Reads RPMh residency stats (Lahaina only)
 */
static std::map<std::string, int64_t> readRpmhResidency() {
    std::map<std::string,int64_t> result;
    const char* path = "/sys/power/rpmh_stats/master_stats";
    std::string stats;
    if (!ReadFileToString(path, &stats)) return result;

    std::istringstream iss(stats);
    std::string line;
    while (std::getline(iss, line)) {
        auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        std::string key = Trim(line.substr(0, pos));
        std::string val_str = Trim(line.substr(pos+1));
        try {
            int64_t val = (val_str.find("0x") == 0) ? std::stoll(val_str,nullptr,16) : std::stoll(val_str);
            result[key] = val;
        } catch (...) { continue; }
    }
    return result;
}

/**
 * @brief Detects SoC type
 */
SocType detectSocType() {
    std::string platform = GetProperty("ro.board.platform", "");
    std::string soc_model = GetProperty("ro.soc.model", "");
    if (soc_model.find("SM8350") != std::string::npos) return SocType::SM8350;
    if (soc_model.find("SM8450") != std::string::npos) return SocType::SM8450;
    if (soc_model.find("SM8550") != std::string::npos) return SocType::SM8550;
    if (soc_model.find("SM8650") != std::string::npos) return SocType::SM8650;
    if (soc_model.find("SM8750") != std::string::npos) return SocType::SM8750;
    if (soc_model.find("SM7325") != std::string::npos) return SocType::SM7325;

    if (platform.find("lahaina") != std::string::npos) return SocType::SM8350;
    if (platform.find("taro") != std::string::npos) return SocType::SM8450;
    if (platform.find("kalama") != std::string::npos) return SocType::SM8550;
    if (platform.find("pineapple") != std::string::npos) return SocType::SM8650;
    if (platform.find("sun") != std::string::npos) return SocType::SM8750;
    if (platform.find("yupik") != std::string::npos) return SocType::SM7325;

    ALOGW("Unknown SoC platform: %s, model: %s", platform.c_str(), soc_model.c_str());
    return SocType::UNKNOWN;
}

// ==================== Energy Calculation Functions ====================

/**
 * @brief Reads CPU energy in µW·ms weighted per cluster
 */
int64_t readCpuEnergy() {
    int64_t total_energy = 0;
    int64_t voltage = readBatteryVoltage();
    int64_t current = readBatteryCurrent();
    if (voltage <=0 || current <=0) { voltage=3700000; current=1000; }

    int num_cpus = sysconf(_SC_NPROCESSORS_CONF);
    std::map<int,int64_t> cluster_weight = {
        {0,1}, {1,1}, {2,1}, {3,1},       // Efficiency cores
        {4,2}, {5,2}, {6,2}, {7,2}        // Prime cores (weighted 2x)
    };

    for (int cpu = 0; cpu < num_cpus; cpu++) {
        int64_t freq_hz = readCpuFrequency(cpu);
        if (freq_hz <= 0) freq_hz = 1000000;

        int64_t idle_ms = 0;
        for (int state=0; state<8; state++) idle_ms += readCpuIdleTime(cpu,state);

        int64_t active_ms = 1000 - idle_ms;
        if (active_ms < 0) active_ms = 1000;

        int64_t weight = cluster_weight.count(cpu) ? cluster_weight[cpu] : 1;
        total_energy += (voltage * current / 1000000LL) * active_ms * weight;
    }
    return total_energy;
}

/**
 * @brief Reads GPU energy in µW·ms weighted by busy percentage and frequency
 */
int64_t readGpuEnergy() {
    int64_t voltage = readBatteryVoltage();
    int64_t current = readBatteryCurrent();
    if (voltage <=0 || current <=0) { voltage=3700000; current=1000; }

    int32_t busy = readGpuUsage();
    if (busy <= 0) busy = 10;

    int64_t freq_hz = readGpuFrequency();
    if (freq_hz <=0) freq_hz = 300000000; // 300MHz fallback

    // Weighted energy: base * busy fraction * frequency factor
    double freq_factor = static_cast<double>(freq_hz) / 1000000000.0; // normalize to GHz
    return static_cast<int64_t>((voltage * current / 1000000LL) * busy * freq_factor);
}

/**
 * @brief Reads communication energy (modem/WiFi) weighted by proportion
 */
int64_t readCommEnergy(double proportion) {
    int64_t power = readBatteryPower();
    if (power <= 0) power = 5000000;
    return static_cast<int64_t>(power * proportion);
}

/**
 * @brief Reads RPMh energy per state (Lahaina only)
 */
std::map<std::string,int64_t> readRpmhEnergy() {
    std::map<std::string,int64_t> rpmh_energy;
    SocType soc = detectSocType();
    if (soc != SocType::SM8350) return rpmh_energy;

    auto stats = readRpmhResidency();
    int64_t voltage = readBatteryVoltage();
    int64_t current = readBatteryCurrent();
    if (voltage <=0 || current<=0) { voltage=3700000; current=1000; }

    for (auto& [state, ms] : stats) {
        rpmh_energy[state] = (voltage * current / 1000000LL) * ms;
    }
    return rpmh_energy;
}

// ==================== IPowerStats Interface ====================

ndk::ScopedAStatus Stats::getEnergyConsumed(
        const std::vector<int32_t>& in_energyConsumerIds,
        std::vector<EnergyConsumerResult>* _aidl_return) {

    std::vector<EnergyConsumerResult> results;
    int64_t timestamp_ms = static_cast<int64_t>(std::time(nullptr)) * 1000;

    int64_t cpu_energy = readCpuEnergy();
    int64_t gpu_energy = readGpuEnergy();
    int64_t modem_energy = readCommEnergy(0.2);
    int64_t wifi_energy = readCommEnergy(0.1);

    for (auto id : in_energyConsumerIds) {
        EnergyConsumerResult result;
        result.id = id;
        result.timestampMs = timestamp_ms;

        switch(id) {
            case 0: result.energyUWs = cpu_energy; break;
            case 1: result.energyUWs = gpu_energy; break;
            case 2: result.energyUWs = modem_energy; break;
            case 3: result.energyUWs = wifi_energy; break;
            default: result.energyUWs = 1000000;
        }
        results.push_back(result);
    }

    *_aidl_return = results;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Stats::getStateResidency(
        const std::vector<int32_t>& in_powerEntityIds,
        std::vector<StateResidencyResult>* _aidl_return) {

    std::vector<StateResidencyResult> results;
    int64_t timestamp_ms = static_cast<int64_t>(std::time(nullptr)) * 1000;

    auto rpmh_stats = readRpmhEnergy();

    for (auto id : in_powerEntityIds) {
        StateResidencyResult result;
        result.id = id;

        if (!rpmh_stats.empty()) {
            for (auto& [state, energy] : rpmh_stats) {
                result.stateResidencyData.push_back({
                    .id = static_cast<int32_t>(result.stateResidencyData.size()),
                    .totalTimeInStateMs = energy,
                    .totalStateEntryCount = 0,
                    .lastEntryTimestampMs = timestamp_ms - 1000
                });
            }
        } else {
            result.stateResidencyData = {
                {.id=0,.totalTimeInStateMs=10000,.totalStateEntryCount=10,.lastEntryTimestampMs=timestamp_ms-2000},
                {.id=1,.totalTimeInStateMs=20000,.totalStateEntryCount=5,.lastEntryTimestampMs=timestamp_ms-5000}
            };
        }

        results.push_back(result);
    }

    *_aidl_return = results;
    return ndk::ScopedAStatus::ok();
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
        case SocType::SM7325: soc_name = "SD778G+"; break;
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
 * Implementation of getEnergyConsumed()
 * Uses battery power readings when available, otherwise placeholders.
 */
ndk::ScopedAStatus Stats::getEnergyConsumed(
        const std::vector<int32_t>& in_energyConsumerIds,
        std::vector<EnergyConsumerResult>* _aidl_return) {

    std::vector<EnergyConsumerResult> results;
    int64_t current_time_ms = static_cast<int64_t>(std::time(nullptr)) * 1000;

    int64_t batteryPower = readBatteryPower(); // µW

    for (const auto& id : in_energyConsumerIds) {
        EnergyConsumerResult result = {
            .id = id,
            .timestampMs = current_time_ms,
            .energyUWs = 0,
            .attribution = {}
        };

        // Simple distribution logic
        switch (id) {
            case 0: result.energyUWs = batteryPower * 0.5; break; // CPU
            case 1: result.energyUWs = batteryPower * 0.2; break; // GPU
            case 2: result.energyUWs = batteryPower * 0.2; break; // Modem
            case 3: result.energyUWs = batteryPower * 0.1; break; // WiFi
            default: result.energyUWs = batteryPower * 0.05; break; // Other
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
        case SocType::SM7325: soc_prefix = "SD778G_"; break;
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
 * Implementation of getStateResidency()
 * Uses RPMh stats if available, otherwise dummy fallback.
 */
ndk::ScopedAStatus Stats::getStateResidency(
        const std::vector<int32_t>& in_powerEntityIds,
        std::vector<StateResidencyResult>* _aidl_return) {

    std::vector<StateResidencyResult> results;
    int64_t current_time_ms = static_cast<int64_t>(std::time(nullptr)) * 1000;

    auto rpmh_stats = readRpmhResidency();

    for (const auto& id : in_powerEntityIds) {
        StateResidencyResult result = {.id = id};

        if (!rpmh_stats.empty()) {
            // Build residency data from RPMh
            for (const auto& [state, val] : rpmh_stats) {
                result.stateResidencyData.push_back({
                    .id = static_cast<int32_t>(result.stateResidencyData.size()),
                    .totalTimeInStateMs = val,
                    .totalStateEntryCount = 0,
                    .lastEntryTimestampMs = current_time_ms - 1000
                });
            }
        } else {
            // Fallback dummy values
            result.stateResidencyData = {
                {.id = 0, .totalTimeInStateMs = 10000, .totalStateEntryCount = 10, .lastEntryTimestampMs = current_time_ms - 2000},
                {.id = 1, .totalTimeInStateMs = 20000, .totalStateEntryCount = 5,  .lastEntryTimestampMs = current_time_ms - 5000}
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
