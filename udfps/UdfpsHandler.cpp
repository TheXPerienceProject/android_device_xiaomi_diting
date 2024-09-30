/*
 * Copyright (C) 2022 The LineageOS Project
 * Copyright (C) 2024 The XPerience Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "UdfpsHandler.diting"

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/unique_fd.h>

#include <poll.h>
#include <sys/ioctl.h>
#include <fstream>
#include <thread>

#include "UdfpsHandler.h"
#include "xiaomi_touch.h"

#define COMMAND_NIT 10
#define PARAM_NIT_FOD 1
#define PARAM_NIT_FOD6 6
#define PARAM_NIT_NONE 0

#define COMMAND_FOD_PRESS_STATUS 1
#define PARAM_FOD_PRESSED 1
#define PARAM_FOD_RELEASED 0

#define COMMAND_FOD_PRESS_X 2
#define COMMAND_FOD_PRESS_Y 3

#define FOD_STATUS_OFF 0
#define FOD_STATUS_ON 1

#define TOUCH_DEV_PATH "/dev/xiaomi-touch"
#define TOUCH_ID 0
#define TOUCH_MAGIC 'T'
#define TOUCH_IOC_SET_CUR_VALUE _IO(TOUCH_MAGIC, SET_CUR_VALUE)
#define TOUCH_IOC_GET_CUR_VALUE _IO(TOUCH_MAGIC, GET_CUR_VALUE)

#define DISP_FEATURE_PATH "/dev/mi_display/disp_feature"

#define MI_DISP_EVENT_FOD 2
#define MI_DISP_EVENT_BRIGHTNESS_CLONE 5

#define MI_DISP_IOCTL_REGISTER_EVENT _IOW('D', 0x07, struct disp_event_req)
#define MI_DISP_IOCTL_SET_LOCAL_HBM _IOW('D', 0x0E, struct disp_local_hbm_req)

#define FOD_PRESS_STATUS_PATH "/sys/class/touch/touch_dev/fod_press_status"

/* 
 * Mi display params
 */
struct disp_base {
	__u32 flag;
	__u32 disp_id;
};

struct disp_event_req {
    struct disp_base base;
    __u32 type;
};

struct disp_local_hbm_req {
	struct disp_base base;
	__u32 local_hbm_value;
};

namespace {

template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

static bool readBool(int fd) {
    char c;
    int rc;

    rc = lseek(fd, 0, SEEK_SET);
    if (rc) {
        LOG(ERROR) << "failed to seek fd, err: " << rc;
        return false;
    }

    rc = read(fd, &c, sizeof(char));
    if (rc != 1) {
        LOG(ERROR) << "failed to read bool from fd, err: " << rc;
        return false;
    }

    return c != '0';
}

static ssize_t readBuffer(int fd, char *buffer, int size) {
    ssize_t rc;
    rc = read(fd, buffer, size);
    if (rc == -1) {
        LOG(ERROR) << "failed to read buffer from fd, err: " << rc;
        return -1;
    }

    return rc;
}

}  // anonymous namespace

class XiaomiSm8450UdfpsHander : public UdfpsHandler {
  public:
    void init(fingerprint_device_t* device) {
        mDevice = device;
        touch_fd_ = android::base::unique_fd(open(TOUCH_DEV_PATH, O_RDWR));
        //enable fod
        setFodStatus(FOD_STATUS_ON);

        disp_fd_ = android::base::unique_fd(open(DISP_FEATURE_PATH, O_RDWR));
        capEnable = false;
        fpPressed = false;

        fodX = 0;
        fodY = 0;

        std::thread([this]() {
            int fd = open(DISP_FEATURE_PATH, O_RDWR | O_CLOEXEC);
            if (fd < 0) {
                LOG(ERROR) << "failed to open fd, err: " << fd;
                return;
            }

            // Fod
            registerDisplayEvent(fd, 0, MI_DISP_EVENT_FOD);
            registerDisplayEvent(fd, 1, MI_DISP_EVENT_FOD);

            // clone the brightness as per ximi does
            registerDisplayEvent(fd, 0, MI_DISP_EVENT_BRIGHTNESS_CLONE);
            registerDisplayEvent(fd, 1, MI_DISP_EVENT_BRIGHTNESS_CLONE);

            struct pollfd dispEventPoll = {
                    .fd = fd,
                    .events = POLLIN | POLLRDNORM,
                    .revents = 0,
            };

            while (true) {
                int rc = poll(&dispEventPoll, 1, -1);
                if (rc < 0) {
                    LOG(ERROR) << "failed to poll fd, err: " << rc;
                    continue;
                }

                char buffer[0x400];
                memset(buffer, 0, sizeof(buffer));
                ssize_t bufferSize = readBuffer(fd, buffer, sizeof(buffer));
                if (bufferSize < 0) {
                    LOG(ERROR) << "read Display event failed, err: " << rc;
                    continue;
                }

                handleDisplayEvent(buffer, bufferSize);
            }
        }).detach();

        std::thread([this]() {
            int fd = open(FOD_PRESS_STATUS_PATH, O_RDONLY);
            if (fd < 0) {
                LOG(ERROR) << "failed to open fd, err: " << fd;
                return;
            }

            struct pollfd fodPressStatusPoll = {
                    .fd = fd,
                    .events = POLLERR | POLLPRI,
                    .revents = 0,
            };

            while (true) {
                int rc = poll(&fodPressStatusPoll, 1, -1);
                if (rc < 0) {
                    LOG(ERROR) << "failed to poll fd, err: " << rc;
                    continue;
                }

                bool pressed = readBool(fd);
                LOG(DEBUG) << __func__ << " extCmd: COMMAND_FOD_PRESS_STATUS " << pressed;

                extCmd(COMMAND_FOD_PRESS_STATUS,
                                pressed ? PARAM_FOD_PRESSED : PARAM_FOD_RELEASED);
            }
        }).detach();
    }

    void extCmd(int32_t cmd, int32_t param) {
        mDevice->extCmd(mDevice, COMMAND_FOD_PRESS_X, fodX);
        mDevice->extCmd(mDevice, COMMAND_FOD_PRESS_Y, fodY);
        mDevice->extCmd(mDevice, cmd, param);
    }

    void onFingerDown(uint32_t /*x*/, uint32_t /*y*/, float /*minor*/, float /*major*/) {
        // fodX = x;
        // fodY = y;
        LOG(INFO) << __func__;
        setFingerDown();
    }

    void onFingerUp() {
        LOG(INFO) << __func__;
        setFingerUp();
    }

    void onAcquired(int32_t result, int32_t vendorCode) {
        LOG(INFO) << __func__ << " result: " << result << " vendorCode: " << vendorCode;
        if (result == FINGERPRINT_ACQUIRED_GOOD && fpPressed) {
            setFingerUp();
        }
    }

    void onEnrollResult(uint32_t fingerId, uint32_t groupId, uint32_t remaining) {
        LOG(INFO) << __func__ << " fingerId: " << fingerId << " remaining: " << remaining;
        if (remaining == 0 && fpPressed) {
            setFingerUp();
        }
    }

    void cancel() {
        LOG(INFO) << __func__;
        setFingerUp();
    }

    void preEnroll() {
        LOG(DEBUG) << __func__;
    }

    void enroll() {
        LOG(DEBUG) << __func__;
    }

    void postEnroll() {
        LOG(DEBUG) << __func__;
    }

  private:
    fingerprint_device_t* mDevice;
    android::base::unique_fd touch_fd_;
    android::base::unique_fd disp_fd_;
    int brightnessValue;
    bool capEnable;
    bool fpPressed;
    uint32_t fodX;
    uint32_t fodY;

    void registerDisplayEvent(int fd, int id, int type) {
        disp_event_req req;
        req.base.flag = 0;
        req.base.disp_id = id;
        req.type = type;
        ioctl(fd, MI_DISP_IOCTL_REGISTER_EVENT, &req);
    }

    void setDisplayLocalHBM(int id, int value) {
        disp_local_hbm_req req;
        req.base.flag = 0;
        req.base.disp_id = id;
        req.local_hbm_value = value;
        ioctl(disp_fd_.get(), MI_DISP_IOCTL_SET_LOCAL_HBM, &req);
    }

    void setFodStatus(int value) {
        int buf[MAX_BUF_SIZE] = {TOUCH_ID, Touch_Fod_Enable, value};
        ioctl(touch_fd_.get(), TOUCH_IOC_SET_CUR_VALUE, &buf);
    }

    void setFingerUp() {
        fpPressed = false;
        setDisplayLocalHBM(0, 0);
        int buf[MAX_BUF_SIZE] = {TOUCH_ID, THP_FOD_DOWNUP_CTL, 0};
        ioctl(touch_fd_.get(), TOUCH_IOC_SET_CUR_VALUE, &buf);
    }

    void setFingerDown() {
        fpPressed = true;
        setDisplayLocalHBM(0, 2);
        int buf[MAX_BUF_SIZE] = {TOUCH_ID, THP_FOD_DOWNUP_CTL, 1};
        ioctl(touch_fd_.get(), TOUCH_IOC_SET_CUR_VALUE, &buf);
    }

    void handleDisplayEvent(const char* buffer, size_t bufferSize) {
        if (bufferSize < 0xC) {
            LOG(ERROR) << "Invalid event size " << bufferSize << " expect 0xC";
            return;
        } else if (bufferSize > 0xF) {
            int pos = 0;
            int handledSize = 0;
            int dataSize = bufferSize - 15;
            do {
                const char *p = &buffer[pos];
                uint32_t type = *(uint32_t*)&buffer[pos + 4];

                if (type == MI_DISP_EVENT_BRIGHTNESS_CLONE) {
                    // Handle brightness clone event
                    if ((*((uint32_t*)p + 2) & 0xFFFFFFFE) == 12) {
                        LOG(ERROR) << "Invalid Backlight value";
                    } else {
                        brightnessValue = *((int *)p + 3);
                    }
                } else if (type == MI_DISP_EVENT_FOD) {
                    // Handle FOD event
                    uint32_t value = *((uint32_t *)p + 3);
                    handleFodEvent(value);
                }
                handledSize += *((uint32_t*)p + 2);
                pos = handledSize;
            } while (dataSize > handledSize);
        }
    }

    void handleFodEvent(uint32_t value) {
        if ((~value & 3) != 0) {
            if ((value & 8) != 0) {
                if ((value & 4) != 0) {
                    LOG(DEBUG) << "fod low brightness capture start";
                    // Adjusting sharpness for low light conditions
                    extCmd(COMMAND_NIT, PARAM_NIT_FOD6); // Keep this call for future devices
                } else {
                    // Adjusting sharpness for low light conditions
                    LOG(DEBUG) << "fod high brightness capture start";
                    extCmd(COMMAND_NIT, PARAM_NIT_FOD);
                }
                capEnable = true;
            } else {
                if (capEnable) {
                    LOG(DEBUG) << "fod capture stop";
                    capEnable = false;
                    extCmd(COMMAND_NIT, PARAM_NIT_NONE);
                }
            }
        } else {
            // Handle other FOD events (if needed)
        }
    }
};

static UdfpsHandler* create() {
    return new XiaomiSm8450UdfpsHander();
}

static void destroy(UdfpsHandler* handler) {
    delete handler;
}

extern "C" UdfpsHandlerFactory UDFPS_HANDLER_FACTORY = {
        .create = create,
        .destroy = destroy,
};
