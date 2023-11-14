/*
 * JasonFreeLab
 *
 */
#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>

/**
 * @brief initialize the led lowlevel module
 *
 * @param none
 *
 * @return none
 */
void led_init(void);

/**
 * @brief deinitialize the led's lowlevel module
 *
 * @param none
 *
 * @return none
 */
void led_deinit(void);

/**
 * @brief turn on/off the lowlevel led
 *
 * @param value The "On" value
 *
 */
void led_set_on_off(bool value);

/**
 * @brief set led low/high level
 *
 * @param value The "On" value
 *
 */
void led_set_level(bool value);

#endif /* _led_H_ */
