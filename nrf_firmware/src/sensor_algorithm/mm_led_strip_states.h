/*
 * mm_led_strip_states.h
 *
 *  Created on: Jan 31, 2019
 *      Author: nataliewong
 */

#ifndef MM_LED_STRIP_STATES_H
#define MM_LED_STRIP_STATES_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                        MACROS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

void mm_led_strip_states_init(void);

/**
    Updates current LED signalling states and increments
    timeout counter.
*/
void mm_led_signalling_states_on_second_elapsed(uint32_t seconds);

/**
 * Updates LED signalling states for all output nodes in case their positions have changed.
 */
void mm_led_signalling_states_on_position_update(void);

#endif /* MM_LED_STRIP_STATES_H */
