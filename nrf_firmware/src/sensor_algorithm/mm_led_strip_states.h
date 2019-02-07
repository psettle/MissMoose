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

#include "app_timer.h"

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
    Determines whether or not the LED signalling states have changed.
*/
bool mm_have_led_signalling_states_changed(void);

/**
    Sends blaze messages to updated LED nodes and implements
    de-escalation.
*/
void mm_on_led_signalling_states_change(void);

/**
    Updates current LED signalling states and increments
    timeout counter.
*/
void mm_led_signalling_states_on_second_elapsed(void);

#endif /* MM_LED_STRIP_STATES_H */
