/*
 * JasonFreeLab
 *
 */
#include <string.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "conn_mgr.h"
#include "esp_sleep.h"
#include "iot_export.h"

#define FACTORY_QUICK_REBOOT_TIMEOUT        (CONFIG_FACTORY_QUICK_REBOOT_TIMEOUT * 1000)
#define FACTORY_QUICK_REBOOT_MAX_TIMES      CONFIG_FACTORY_QUICK_REBOOT_MAX_TIMES
#define FACTORY_QUICK_REBOOT_TIMES          "q_rt"

#define AWSS_KV_RST                         "awss.rst"

static const char *TAG = "factory_rst";

//切换智能配网模式
static void factory_set_sc_mode(void)
{
    uint8_t mode_kv = 0;
    conn_sc_mode_t mode_set = CONN_SC_ZERO_MODE;
    int len_kv = sizeof(uint8_t);

    int ret = HAL_Kv_Get(SC_MODE, &mode_kv, &len_kv);
    if (ret == ESP_OK) {
        if (mode_kv == CONN_SOFTAP_MODE) {
            mode_set = CONN_SC_ZERO_MODE;
        } else {
            mode_set = CONN_SOFTAP_MODE;
        }
    }

    conn_mgr_set_sc_mode(mode_set);
}

//重启定时器计数执行操作
static esp_err_t factory_restore_handle(void)
{
    esp_err_t ret = ESP_OK;
    int quick_reboot_times = 0;

    /**< If the device restarts within the instruction time, the event_mdoe value will be incremented by one */
    int length = sizeof(int);
    ret = HAL_Kv_Get(FACTORY_QUICK_REBOOT_TIMES, &quick_reboot_times, &length);

    quick_reboot_times++;

    ret = HAL_Kv_Set(FACTORY_QUICK_REBOOT_TIMES, &quick_reboot_times, sizeof(int), 0);

    if (quick_reboot_times >= FACTORY_QUICK_REBOOT_MAX_TIMES) {
        char rst = 0x01;

        /*  since we cannot report reset status to cloud in this stage, just set the reset flag.
            when connects to cloud, awss will do the reset report. */
        ret = HAL_Kv_Set(AWSS_KV_RST, &rst, sizeof(rst), 0);  
        ret = HAL_Kv_Del(FACTORY_QUICK_REBOOT_TIMES);

        ESP_LOGW(TAG, "factory restore");
        conn_mgr_reset_wifi_config();
        factory_set_sc_mode();
    } else {
        ESP_LOGI(TAG, "quick reboot times %d, don't need to restore", quick_reboot_times);
    }

    return ret;
}

//重启定时器超时清除
static void factory_restore_timer_handler(void *timer)
{
    if (!xTimerStop(timer, 0)) {
        ESP_LOGE(TAG, "xTimerStop timer %p", timer);
    }

    if (!xTimerDelete(timer, 0)) {
        ESP_LOGE(TAG, "xTimerDelete timer %p", timer);
    }

    /* erase reboot times record */
    HAL_Kv_Del(FACTORY_QUICK_REBOOT_TIMES);

    ESP_LOGI(TAG, "Quick reboot timeout, clear reboot times");
}

//创建重启定时器
esp_err_t factory_restore_init(void)
{
#ifndef CONFIG_IDF_TARGET_ESP8266
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
        HAL_Kv_Del(FACTORY_QUICK_REBOOT_TIMES);
        return ESP_OK;
    }
#endif
    /* 名称 周期 是否自动重载 ID 回调函数 */
    TimerHandle_t timer = xTimerCreate("factory_clear", FACTORY_QUICK_REBOOT_TIMEOUT / portTICK_PERIOD_MS,
                                       false, NULL, factory_restore_timer_handler);

    xTimerStart(timer, portMAX_DELAY);

    return factory_restore_handle();
}
