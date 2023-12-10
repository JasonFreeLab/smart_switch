/*
 * JasonFreeLab
 *
 */
#include <sdkconfig.h>

#include "driver/gpio.h"

#include "esp_log.h"

#define GPIO_OUTPUT_IO       CONFIG_SWITCH_GPIO
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO)

static const char *TAG = "switch";

/**
 * @brief initialize the switch
 */
void switch_init(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO5
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gpio_set_level(GPIO_OUTPUT_IO, false);
}

/**
 * @brief deinitialize the switch
 */
void switch_deinit(void)
{
}

/**
 * @brief turn on/off the highlevel switch
 */
void switch_set_on_off(bool value)
{
    ESP_LOGI(TAG, "switch_set_on : %s", value == true ? "true" : "false");

    if (value) { //正逻辑
        gpio_set_level(GPIO_OUTPUT_IO, true);
    } else {
        gpio_set_level(GPIO_OUTPUT_IO, false);
    }
}
