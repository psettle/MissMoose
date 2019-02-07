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

/**
    Creates the 30 second timer (responsible for de-escalating
    the "concern" state), but does not start it, and
    initializes led_signalling_states.
*/
void mm_led_strip_states_init(void);

/**
    Updates the signalling state of the LEDs based on the
    current value of the activity variables.

    Hardcoded for 3x3 grids for now.
*/
void mm_update_led_signalling_states(void);

/**
    Sends blaze messages to each LED node to update the colors based
    on the current LED signalling states.

    Hardcoded for 3x3 grids for now.
*/
void mm_update_node_led_colors(void);

/**
    Gets the value of has_concern_state to determine
    if the timer should be started. True if
    a concern state LED state has been detected.
*/
bool mm_should_start_thirty_second_timer(void);

/**
    Starts thirty second timer which de-escalates a concern
    state after thirty seconds. Clears has_concern_state
    back to false.
*/
void mm_start_thirty_second_timer(void);

#endif /* MM_LED_STRIP_STATES_H */
