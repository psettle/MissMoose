/*
 * mm_led_strip_states.h
 *
 *  Created on: Jan 31, 2019
 *      Author: nataliewong
 */

#ifndef SRC_SENSOR_ALGORITHM_MM_LED_STRIP_STATES_H_
#define SRC_SENSOR_ALGORITHM_MM_LED_STRIP_STATES_H_

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

/* Road-side (RS), non-road-side (NRS) */
#define POSSIBLE_DETECTION_THRESHOLD_RS     ( 3.0f )
#define POSSIBLE_DETECTION_THRESHOLD_NRS    ( 4.0f )

#define DETECTION_THRESHOLD_RS              ( 6.0f )
#define DETECTION_THRESHOLD_NRS             ( 7.0f )

/**********************************************************
                        TYPES
**********************************************************/

/* LED signalling states */
typedef enum
{
    IDLE,
	CONCERN,
	ALARM
} led_signalling_state_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Creates the 30 second timer (responsible for de-escalating
    the "concern" state), but does not start it, and
    initializes led_signalling_states.
*/
void led_strip_states_init(void);

/**
    Updates the signalling state of the LEDs based on the
    current value of the activity variables.

    Hardcoded for 3x3 grids for now.
*/
void update_led_signalling_states(void);

/**
    Sends blaze messages to each LED node to update the colors based
    on the current LED signalling states.

    Hardcoded for 3x3 grids for now.
*/
void update_node_led_colors(void);

/**
    Gets the value of has_concern_state to determine
    if the timer should be started. True if
    a concern state LED state has been detected.
*/
bool should_start_thirty_second_timer(void);

/**
    Starts thirty second timer which de-escalates a concern
    state after thirty seconds. Clears has_concern_state
    back to false.
*/
void start_thirty_second_timer(void);

#endif /* SRC_SENSOR_ALGORITHM_MM_LED_STRIP_STATES_H_ */
