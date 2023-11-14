/*
 * JasonFreeLab
 *
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize factory restore 
 * 
 * @note For some devices which don't have a physical restore key or button, such as light, we use
 *       a quick reboot method to record the reboot times, if users quickly reboot the device within
 *       a specific time, configured by FACTORY_QUICK_REBOOT_TIMEOUT, and reboot several times,
 *       configured by FACTORY_QUICK_REBOOT_MAX_TIMES, the device will do a factory restore, clear
 *       stored ssid and password.
 * 
 * @return
 *     - ESP_OK : OK
 *     - others : fail
 */
esp_err_t factory_restore_init(void);

#ifdef __cplusplus
}
#endif