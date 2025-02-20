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
}

// Implementation of isEnabled()
Return<bool> HighTouchPollingRate::isEnabled() {

   // Read the value directly from sysfs (or device node)
   std::string path = this->FindSysfsPath("touch_thp_cmd");  // Get file path
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
    std::string path = this->FindSysfsPath("touch_thp_cmd"); 
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
std::string HighTouchPollingRate::FindSysfsPath(const std::string& attribute_name) {
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
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace xperience
}  // namespace vendor
