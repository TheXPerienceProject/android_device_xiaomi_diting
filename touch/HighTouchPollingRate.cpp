/*
 * Copyright (C) 2022 The LineageOS Project
 * Copyright (C) 2011-2023-2025 The XPerience Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.xperience.touch@1.0-service.diting"

#include "HighTouchPollingRate.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <string>  // Include for std::string
#include <thread>
#include <chrono>
#include <sstream> // Para std::stringstream
#include <dirent.h>
#include <vector>
#include <iostream>

// Constants for ioctl
#define SET_CUR_VALUE 0
#define TOUCH_GAME_MODE 0
#define TOUCH_MAGIC 't'
#define TOUCH_IOC_SETMODE _IO(TOUCH_MAGIC, SET_CUR_VALUE)
#define TOUCH_DEV_PATH "/dev/xiaomi-touch"
#define TOUCH_ID 0

namespace vendor {
namespace xperience {
namespace touch {
namespace V1_0 {
namespace implementation {

const std::string kHighTouchPollingPath = "TOUCH_DEV_PATH";

// Class constructor
HighTouchPollingRate::HighTouchPollingRate() {
    monitoringThread = std::thread(&HighTouchPollingRate::monitorPollingRate, this);   
}

// Implementation of isEnabled()
Return<bool> HighTouchPollingRate::isEnabled() {

   // Read the value directly from sysfs (or device node)
    // std::string path = this->FindSysfsPath("touch_thp_cmd"); // remove later
    std::string path = "/sys/devices/virtual/touch/touch_dev/touch_thp_cmd";
   if (path.empty()) {
       LOG(ERROR) << "Failed to find touch_thp_cmd in sysfs";
       return false; // Return false if path not found
   }

   std::string value_str;
   if (!android::base::ReadFileToString(path, &value_str)) { // Read value from file
       LOG(ERROR) << "Failed to read touch_thp_cmd from sysfs";
       return false; // Return false if file cannot be read
   }

   try {
       int enabled = std::stoi(value_str); // Try to convert value to integer
       return enabled == 1;// Return true if value is 1, false if 0
   } catch (const std::invalid_argument& e) {// Catch conversion errors
       LOG(ERROR) << "Invalid touch device state: " << e.what();
       return false;// Return false if value is invalid
   } catch (const std::out_of_range& e) {// Catch range errors
       LOG(ERROR) << "Touch device state out of range: " << e.what();
       return false; // Return false if value is out of range
   }

}

Return<bool> HighTouchPollingRate::setEnabled(bool enabled) {
    int fd = open(TOUCH_DEV_PATH, O_RDWR);// Open touch device
    if (fd < 0) {
        LOG(ERROR) << "Failed to open touch device: " << strerror(errno);
        return false; // Return false if device cannot be opened
    }

    int arg[3] = {TOUCH_ID, TOUCH_GAME_MODE, enabled ? 1 : 0};// Create argument for ioctl
    if (ioctl(fd, TOUCH_IOC_SETMODE, &arg) < 0) { // Call ioctl
        LOG(ERROR) << "ioctl failed: " << strerror(errno);
        close(fd);  // Close file descriptor in case of error
        return false; // Return false if ioctl fails
    }

    close(fd);// Close file descriptor

    // Optionally update sysfs as well (if needed)
    // std::string path = this->FindSysfsPath("touch_thp_cmd"); // Eliminar esta línea
    std::string path = "/sys/devices/virtual/touch/touch_dev/touch_thp_cmd";

    if (!path.empty()) {
        std::ofstream file(path);
        if (file.is_open()) {
            file << (enabled ? "1" : "0");
        } else {
            LOG(ERROR) << "Failed to open sysfs path for writing";
        }
    }

    return enabled; // Return the value that was tried to be set
}

    /*
     * This function searches for a specific sysfs attribute within the device's directory.
     *
     * It starts by opening the base sysfs directory ("/sys/devices").
     * Then, it iterates through each entry within this directory.
     * For each entry, it checks if it's a directory (using ent->d_type == DT_DIR).
     * If it's a directory, it opens that directory and iterates through its entries.
     * Inside the device directory, it checks if any entry's name matches the provided
     * attribute_name. If a match is found, it constructs the full path to the
     * attribute (e.g., "/sys/devices/.../xiaomi-touch/.../attribute_name") and returns it.
     *
     * If the attribute is not found within the device's sysfs directory, the function
     * returns an empty string.
     *
     * Note: This function assumes that the device's directory name ("xiaomi-touch") is
     * unique enough to identify the correct device.  If there's a possibility of 
     * multiple devices with the same name, you might need to add more sophisticated
     * logic to identify the correct one.
     */
/*std::string HighTouchPollingRate::FindSysfsPath(const std::string& attribute_name) {
    std::string base_path = "/sys/devices";// Base path to devices
    std::string device_name = "xiaomi-touch"; // Device name (adjust if needed)

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(base_path.c_str())) != NULL) { // Open base directory
        while ((ent = readdir(dir)) != NULL) { // Iterate over directory entries
            std::string dev_path = base_path + "/" + ent->d_name;// Build path to device
            if (ent->d_type == DT_DIR) { // If entry is a directory
                DIR *dev_dir;
                struct dirent *attr_ent;
                if ((dev_dir = opendir(dev_path.c_str())) != NULL) {// Open device directory
                    while ((attr_ent = readdir(dev_dir)) != NULL) {// Iterate over directory entries
                        if (std::string(attr_ent->d_name) == attribute_name) {// If entry matches the searched attribute
                            return dev_path + "/" + attribute_name;// Return the full path to the attribute
                        }
                    }
                    closedir(dev_dir); // Close device directory
                }
            }
        }
        closedir(dir); // Close base directory
    } else {
        perror("could not open directory");// Print error message if directory cannot be opened
    }
    return "";// Return an empty string if attribute is not found
}*/

    /* Monitors touch events and automatically disables
     * the high polling rate after inactivity.
    */
void HighTouchPollingRate::monitorPollingRate() {
    auto lastTouchTime = std::chrono::steady_clock::now();
    const auto inactivityThreshold = std::chrono::minutes(10);
    auto screenOffTime = std::chrono::time_point<std::chrono::steady_clock>::min();

    std::vector<bool> screenStateHistory(3, true); // Historial de 3 estados inicializado a encendido

    while (true) {
        bool screenOn = isScreenOn();
        std::vector<std::string> eventData = getTouchEvents();
        bool touchDetected = false;

        for (const std::string& line : eventData) {
            std::stringstream ss(line);
            int type, code, value;
            if (ss >> std::hex >> type >> std::hex >> code >> std::hex >> value) {
                if (type == 0x0000 && code == 0x0000) {
                    touchDetected = true;
                    break;
                }
            }
        }

        if (touchDetected) {
            lastTouchTime = std::chrono::steady_clock::now();
        } else {
            auto currentTime = std::chrono::steady_clock::now();
            if (isEnabled() && (currentTime - lastTouchTime) > inactivityThreshold) {
                LOG(INFO) << "No touch activity for 10 minutes, deactivating high polling rate.";
                setEnabled(false);
            }
        }

        /*
            check if the screen is on or not and if it is off, 
            check if it has not been idle for more than 10 minutes
            to deactivate or activate the polling rate.
        */
        if (!screenOn) {
            if (screenOffTime == std::chrono::time_point<std::chrono::steady_clock>::min()) {
                screenOffTime = std::chrono::steady_clock::now();
            }
        } else {
            if (screenOffTime != std::chrono::time_point<std::chrono::steady_clock>::min()) {
                auto timeSinceScreenOff = std::chrono::steady_clock::now() - screenOffTime;
                if (timeSinceScreenOff < inactivityThreshold && isEnabled()) {
                    LOG(INFO) << "Screen turned on before inactivity timeout, reenabling high polling rate.";
                    setEnabled(true);
                }
                screenOffTime = std::chrono::time_point<std::chrono::steady_clock>::min();
            }
        }

        // Actualizar el historial de estados
        screenStateHistory.erase(screenStateHistory.begin());
        screenStateHistory.push_back(screenOn);

        // Detectar apagado-encendido rápido
        if (!screenStateHistory[1] && screenStateHistory[2]) {
            LOG(INFO) << "Quick screen off-on detected, reenabling high polling rate.";
            setEnabled(true);
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));//original was 1s try with 3s
    }
}

    /*
    * Get touch events from /dev/input/event7 using popen and getevent
    */
std::vector<std::string> HighTouchPollingRate::getTouchEvents() {
    std::vector<std::string> events;
    FILE* pipe = popen("getevent -lt /dev/input/event7", "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            events.emplace_back(buffer);
        }
        pclose(pipe);
    }
    return events;
}

    /*
    // Implement logic to detect screen state
    // Use sysfs or PowerManager as needed
    // Return true if the screen is on, false if it's off
    // */
bool HighTouchPollingRate::isScreenOn() {

    std::string path = "/sys/class/backlight/panel0-backlight/bl_power";
    std::string value_str;
    if (android::base::ReadFileToString(path, &value_str)) {
        try {
            int value = std::stoi(value_str);
            return value == 0; // 0 indicates display on and 4 screen off on diting
        } catch (...) {
            LOG(ERROR) << "Error al leer el estado de la pantalla desde sysfs";
        }
    }
    return true; // Devolver true por defecto si no se puede leer el estado de la pantalla
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace xperience
}  // namespace vendor
