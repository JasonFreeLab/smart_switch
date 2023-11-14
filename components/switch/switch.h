/*
 * JasonFreeLab
 *
 */
#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <stdbool.h>

/**
 * @brief initialize the switch lowlevel module
 *
 * @param none
 *
 * @return none
 */
void switch_init(void);

/**
 * @brief deinitialize the switch's lowlevel module
 *
 * @param none
 *
 * @return none
 */
void switch_deinit(void);

/**
 * @brief turn on/off the lowlevel switch
 *
 * @param value The "On" value
 *
 */
void switch_set_on_off(bool value);

#endif /* _SWITCH_H_ */
