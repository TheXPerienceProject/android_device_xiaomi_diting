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
#include <set>
#include <ctime>
#include <charconv>
#include <regex>
#include <future>
#include <numeric>
#include <atomic>
#include <android-base/file.h>
#include <android-base/strings.h>
#include <android-base/properties.h>
#include <log/log.h>

using android::base::ReadFileToString;
using android::base::Trim;
using android::base::GetProperty;

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace stats {

// ==================== Constants ====================

constexpr const char* kProcStat = "/proc/stat";
constexpr const char* kGpuFreqPath = "/sys/class/kgsl/kgsl-3d0/gpuclk";
constexpr const char* kGpuBusyPath = "/sys/class/kgsl/kgsl-3d0/gpu_busy_percentage";
constexpr const char* kBatteryVoltagePath = "/sys/class/power_supply/battery/voltage_now";
constexpr const char* kBatteryCurrentPath = "/sys/class/power_supply/battery/current_now";
constexpr const char* kCpuFreqBase = "/sys/devices/system/cpu/cpu";

// Possible CPU idle time paths
const std::vector<std::string> kCpuIdleTimePaths = {
    "/sys/devices/system/cpu/cpu%d/cpuidle/state%d/time",
    "/sys/devices/system/cpu/cpu%d/cpuidle/state%d/residency",
    "/sys/devices/system/cpu/cpu%d/cpuidle/state%d/time"
};

// ==================== Cached / Lazy Variables ====================

static SocType gCachedSoc = SocType::UNKNOWN;
static int64_t gCachedVoltage = -1;
static int64_t gCachedCurrent = -1;

struct ClusterInfo {
    int weight;               ///< Weight of the cluster based on max frequency
    std::vector<int> cpus;    ///< CPUs belonging to this cluster
};
static std::vector<ClusterInfo> gClusterInfo;
static bool gClusterInitialized = false;

// ==================== Utility Functions ====================

/**
 * @brief Reads int64_t from sysfs file, returns fallback if failed
 * @param path Path to read
 * @param fallback Value to return on failure
 * @return Parsed int64_t or fallback
 */
static int64_t readInt64File(const std::string& path, int64_t fallback = -1) {
    std::string data;
    if (!ReadFileToString(path, &data)) return fallback;
    int64_t val = 0;
    auto [ptr, ec] = std::from_chars(data.data(), data.data() + data.size(), val);
    if (ec != std::errc()) return fallback;
    return val;
}

/**
 * @brief Detects SoC type, cached after first detection
 * @return Detected SocType
 */
static SocType detectSocType() {
    if (gCachedSoc != SocType::UNKNOWN) return gCachedSoc;

    std::string platform = GetProperty("ro.board.platform", "");
    std::string soc_model = GetProperty("ro.soc.model", "");

    ALOGD("Platform detection: platform='%s', soc_model='%s'", platform.c_str(), soc_model.c_str());
    if (soc_model.find("SM8350") != std::string::npos) gCachedSoc = SocType::SM8350;
    else if (soc_model.find("SM8450") != std::string::npos) gCachedSoc = SocType::SM8450;
    else if (soc_model.find("SM8550") != std::string::npos) gCachedSoc = SocType::SM8550;
    else if (soc_model.find("SM8650") != std::string::npos) gCachedSoc = SocType::SM8650;
    else if (soc_model.find("SM8750") != std::string::npos) gCachedSoc = SocType::SM8750;
    else if (soc_model.find("SM7325") != std::string::npos) gCachedSoc = SocType::SM7325;
    else gCachedSoc = SocType::UNKNOWN;

    return gCachedSoc;
}

/**
 * @brief Lazy cached voltage in µV
 * @return Voltage in microvolts
 */
static int64_t getCachedVoltage() {
    if (gCachedVoltage < 0) {
        gCachedVoltage = readInt64File(kBatteryVoltagePath, 3700000);
    }
    return gCachedVoltage;
}

/**
 * @brief Lazy cached current in µA
 * @return Current in microamps
 */
static int64_t getCachedCurrent() {
    if (gCachedCurrent < 0) {
        gCachedCurrent = readInt64File(kBatteryCurrentPath, 1000);
    }
    return gCachedCurrent;
}

// ==================== CPU Cluster Initialization ====================
/**
 * @brief Detects CPU clusters and assigns weights based on max frequency.
 *        Uses parallel tasks for faster initialization on multi-core systems.
 *        Caches results for subsequent calls.
 */
static void initializeClustersParallel() {
    if (gClusterInitialized) return;

    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus <= 0) num_cpus = 8;

    std::vector<int> cpu_to_cluster(num_cpus);
    std::set<int> cluster_ids;

    // Read cluster ID for each CPU in parallel
    std::vector<std::future<void>> futures;
    for (int cpu = 0; cpu < num_cpus; cpu++) {
        futures.push_back(std::async(std::launch::async, [cpu, &cpu_to_cluster]() {
            int cluster_id = static_cast<int>(
                readInt64File(kCpuFreqBase + std::to_string(cpu) + "/topology/physical_package_id", 0)
            );
            cpu_to_cluster[cpu] = cluster_id;
        }));
    }

    // Wait for all cluster ID reads to complete
    for (auto& f : futures) f.get();

    cluster_ids.insert(cpu_to_cluster.begin(), cpu_to_cluster.end());
    gClusterInfo.clear();

    // Assign cluster weights and CPU lists
    for (int cluster_id : cluster_ids) {
        ClusterInfo info;
        info.weight = 1;  // default weight
        for (int cpu = 0; cpu < num_cpus; cpu++) {
            if (cpu_to_cluster[cpu] == cluster_id) {
                info.cpus.push_back(cpu);
            }
        }

        // Determine cluster weight in parallel
        futures.clear();
        std::atomic<int> max_weight{1};
        for (int cpu : info.cpus) {
            futures.push_back(std::async(std::launch::async, [cpu, &max_weight]() {
                int64_t max_freq = readInt64File(
                    kCpuFreqBase + std::to_string(cpu) + "/cpufreq/cpuinfo_max_freq", 0
                );
                if (max_freq > 2500000) max_weight.store(2, std::memory_order_relaxed);
            }));
        }
        for (auto& f : futures) f.get();
        info.weight = max_weight.load();

        gClusterInfo.push_back(info);
    }

    gClusterInitialized = true;
}

// ==================== CPU Energy ====================

/**
 * @brief Calculate CPU energy in µW·ms using cpuidle residency times
 * @return Total CPU energy in µW·ms
 */
int64_t readCpuEnergyParallel() {
    initializeClustersParallel();

    int64_t voltage = getCachedVoltage();
    int64_t current = getCachedCurrent();
    int64_t power = (voltage * current) / 1000000LL;

    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus <= 0) num_cpus = 8;

    std::atomic<int64_t> total_energy{0};

    for (const auto& cluster : gClusterInfo) {
        for (int cpu : cluster.cpus) {
            int state_id = 0;
            while (true) {
                std::ostringstream oss;
                oss << "/sys/devices/system/cpu/cpu" << cpu 
                    << "/cpuidle/state" << state_id << "/time";
                std::string path = oss.str();

                int64_t time_ms = readInt64File(path, -1);
                if (time_ms < 0) break;

                // Weighted energy per cluster
                total_energy.fetch_add(time_ms * power * cluster.weight, std::memory_order_relaxed);
                state_id++;
            }
        }
    }

    return total_energy.load();
}

// ==================== GPU Energy ====================

/**
 * @brief Reads GPU energy using runtime_active_time if available
 *        Falls back to previous busy% * freq calculation
 * @return Total GPU energy in µW·ms
 */
int64_t readGpuEnergy() {
    int64_t voltage = getCachedVoltage();
    int64_t current = getCachedCurrent();
    int64_t power = (voltage * current) / 1000000LL;

    // Try reading runtime_active_time (ns) for GPU
    int64_t runtime_ns = readInt64File("/sys/class/kgsl/kgsl-3d0/power/runtime_active_time", -1);
    if (runtime_ns > 0) {
        // Convert ns → ms
        int64_t runtime_ms = runtime_ns / 1000000LL;
        return power * runtime_ms;
    }

    // Fallback: busy% * freq factor
    int32_t busy_pct = readInt64File(kGpuBusyPath,0);
    busy_pct = std::clamp(busy_pct, 0, 100);
    double busy_frac = busy_pct / 100.0;

    int64_t freq = readInt64File(kGpuFreqPath, 300000000);
    int64_t max_freq = readInt64File("/sys/class/kgsl/kgsl-3d0/max_gpuclk", 600000000);
    double freq_factor = std::min(1.0, static_cast<double>(freq)/max_freq);

    return static_cast<int64_t>(power * busy_frac * freq_factor);
}

// ==================== Communication Energy ====================

/**
 * @brief Reads communication energy (modem/WiFi) intelligently per SoC
 *        Uses real battery power readings if available, otherwise estimates.
 * @param type "modem" or "wifi"
 * @return Weighted energy in µW·ms
 */
int64_t readCommEnergy(const std::string& type) {
    int64_t power = -1;

    // Try kernel-exposed energy files
    power = readInt64File("/sys/class/power_supply/battery/power_now", -1);
    if (power < 0) power = readInt64File("/sys/class/power_supply/battery/power_avg", -1);

    if (power < 0) {
        // fallback to (V*I)/1e6
        int64_t voltage = getCachedVoltage();
        int64_t current = getCachedCurrent();
        power = (voltage * current) / 1000000LL;
    }

    // Default proportion
    double proportion = 0.1;

    SocType soc = detectSocType();
    if (type == "modem") {
        switch (soc) {
            case SocType::SM8350: proportion = 0.25; break;
            case SocType::SM8450: proportion = 0.2;  break;
            case SocType::SM8550: proportion = 0.18; break;
            case SocType::SM8650: proportion = 0.15; break;
            case SocType::SM8750: proportion = 0.15; break;
            case SocType::SM7325: proportion = 0.22; break;
            default: proportion = 0.2;
        }
    } else if (type == "wifi") {
        switch (soc) {
            case SocType::SM8350: proportion = 0.1; break;
            case SocType::SM8450: proportion = 0.08; break;
            case SocType::SM8550: proportion = 0.07; break;
            case SocType::SM8650: proportion = 0.06; break;
            case SocType::SM8750: proportion = 0.06; break;
            case SocType::SM7325: proportion = 0.09; break;
            default: proportion = 0.08;
        }
    }

    return static_cast<int64_t>(power * proportion);
}

// ==================== IPowerStats Interface ====================

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
 * Uses battery power readings and dedicated energy calculation functions.
 */
ndk::ScopedAStatus Stats::getEnergyConsumed(
        const std::vector<int32_t>& in_energyConsumerIds,
        std::vector<EnergyConsumerResult>* _aidl_return) {

    std::vector<EnergyConsumerResult> results;
    int64_t timestamp_ms = static_cast<int64_t>(std::time(nullptr)) * 1000;

    // Launch parallel tasks for each energy consumer
    auto cpu_future   = std::async(std::launch::async, [](){ return readCpuEnergyParallel(); });
    auto gpu_future   = std::async(std::launch::async, [](){ return readGpuEnergy(); });
    auto modem_future = std::async(std::launch::async, [](){ return readCommEnergy("modem"); });
    auto wifi_future  = std::async(std::launch::async, [](){ return readCommEnergy("wifi"); });

    // Retrieve results from futures
    int64_t cpu_energy   = cpu_future.get();
    int64_t gpu_energy   = gpu_future.get();
    int64_t modem_energy = modem_future.get();
    int64_t wifi_energy  = wifi_future.get();

    // Map input consumer IDs to their respective energy
    for (const auto& id : in_energyConsumerIds) {
        EnergyConsumerResult result = {
            .id = id,
            .timestampMs = timestamp_ms,
            .energyUWs = 0,
            .attribution = {}
        };

        switch(id) {
            case 0: result.energyUWs = cpu_energy; break;    // CPU cluster
            case 1: result.energyUWs = gpu_energy; break;    // GPU
            case 2: result.energyUWs = modem_energy; break;  // Modem
            case 3: result.energyUWs = wifi_energy; break;   // WiFi
            default: result.energyUWs = 1000000;             // Fallback
        }
        results.push_back(result);
    }

    *_aidl_return = results;
    return ndk::ScopedAStatus::ok();
}

/**
 * Retrieves information about available power entities
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
    
    // Memory / Comm entity
    entities.push_back({
        .id = 2,
        .name = soc_prefix + "COMM",
        .states = {
            {.id = 0, .name = "active"},
            {.id = 1, .name = "idle"}
        }
    });

    *_aidl_return = entities;
    return ndk::ScopedAStatus::ok();
}

/**
 * @brief Implementation of getStateResidency()
 * 
 * Provides residency information for each power entity.
 * 
 * RPMh stats (specific to Lahaina) are no longer used.
 * For all SoCs, fallback dummy values are returned to ensure
 * compatibility across Snapdragon 8350 → 8750 and future SoCs.
 *
 * @param in_powerEntityIds Vector of power entity IDs to query
 * @param[out] _aidl_return Vector of StateResidencyResult
 * @return ndk::ScopedAStatus OK on success
 */
ndk::ScopedAStatus Stats::getStateResidency(
        const std::vector<int32_t>& in_powerEntityIds,
        std::vector<StateResidencyResult>* _aidl_return) {

    std::vector<StateResidencyResult> results;
    int64_t current_time_ms = static_cast<int64_t>(std::time(nullptr)) * 1000;

    for (const auto& id : in_powerEntityIds) {
        StateResidencyResult result;
        result.id = id;

        std::vector<StateResidency> states;

        int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
        if (num_cpus <= 0) num_cpus = 8;

        for (int cpu = 0; cpu < num_cpus; cpu++) {
            int state_id = 0;
            while (true) {
                std::ostringstream oss;
                oss << "/sys/devices/system/cpu/cpu" << cpu
                    << "/cpuidle/state" << state_id << "/time";
                std::string path = oss.str();

                int64_t time_ms = readInt64File(path, -1);
                if (time_ms < 0) break;

                StateResidency s;
                s.id = state_id;
                s.totalTimeInStateMs = time_ms;
                s.totalStateEntryCount = 0;  // no se expone por defecto
                s.lastEntryTimestampMs = current_time_ms - 1000;
                states.push_back(s);

                state_id++;
            }
        }

        if (states.empty()) {
            // fallback mínimo si kernel no expone cpuidle
            StateResidency s;
            s.id = 0;
            s.totalTimeInStateMs = 1;
            s.totalStateEntryCount = 1;
            s.lastEntryTimestampMs = current_time_ms;
            states.push_back(s);
        }

        result.stateResidencyData = states;
        results.push_back(result);
    }

    *_aidl_return = results;
    return ndk::ScopedAStatus::ok();
}

/**
 * Retrieves energy meter information (placeholder - not implemented)
 * * @param[out] _aidl_return Vector of Channel information
 * @return ndk::ScopedAStatus OK with empty list
 */
ndk::ScopedAStatus Stats::getEnergyMeterInfo(std::vector<Channel>* _aidl_return) {
    // Return empty list - energy meter not implemented
    *_aidl_return = {};
    return ndk::ScopedAStatus::ok();
}

/**
 * Reads energy meter data (placeholder - not implemented)
 * * @param in_channelIds List of channel IDs to read
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