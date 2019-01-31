/*
 * mm_led_strip_states.c
 *
 *  Created on: Jan 31, 2019
 *      Author: nataliewong
 */

/**********************************************************
                        INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"

#include "mm_led_strip_states.h"
#include "mm_sensor_algorithm_config.h"
#include "mm_activity_variables.h"
//#include "mm_led_control.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define THIRTY_SECONDS_MS                   ( 30 * 1000 )
#define THIRTY_SECOND_TIMER_TICKS APP_TIMER_TICKS(THIRTY_SECONDS_MS)

/**********************************************************
                        MACROS
**********************************************************/

/**********************************************************
                        TYPES
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Single shot timer handler to process once-per-thirty-second updates.

    Exclusively used for de-escalating the concern AV state right now.
*/
static void thirty_second_timer_event_handler(void * p_context);

/**
    Determines if the activity variable located at (x, y)
    is road side or not.
*/
static bool is_road_side_av( uint16_t x, uint16_t y );

/**********************************************************
                       VARIABLES
**********************************************************/

/* Assumes that there are MAX_GRID_SIZE_X nodes with LEDs. */
static led_signalling_state_t led_signalling_states [MAX_GRID_SIZE_X];

static bool has_concern_state = false;

APP_TIMER_DEF(m_thirty_second_timer_id);

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Creates the 30 second timer responsible for de-escalating
    the "concern" state, but does not start it.
*/
void led_strip_states_init(void)
{
    /* Initialize LED signalling states */
    memset( &led_signalling_states[0], 0, sizeof(led_signalling_states) );

	/* Create thirty second timer, but don't start it yet. */
	uint32_t err_code = app_timer_create(&m_thirty_second_timer_id, APP_TIMER_MODE_SINGLE_SHOT, thirty_second_timer_event_handler);
	APP_ERROR_CHECK(err_code);
}

/**
    Updates the signalling state of the LEDs based on the
    current value of the activity variables.

    Hardcoded for only 3x3 grids right now.
*/
void update_led_signalling_states(void)
{
	for ( uint16_t x = 0; x < ( MAX_GRID_SIZE_X - 1 ); x++ )
	{
		for ( uint16_t y = 0; y < ( MAX_GRID_SIZE_Y - 1 ); y++ )
		{
			/* If the AV value is less than the threshold value, ignore and continue. */
			if ( ( is_road_side_av(x, y) && ( AV(x, y) < POSSIBLE_DETECTION_THRESHOLD_RS ) ) ||
					( !is_road_side_av(x, y) && ( AV(x, y) < POSSIBLE_DETECTION_THRESHOLD_RS ) ) )
			{
				continue;
			}

			if ( is_road_side_av(x, y) )
			{
				if ( ( AV(x, y) >= DETECTION_THRESHOLD_RS ) )
				{
					/* Adding the enums here guarantees that
					 * multiple "concern" states will escalate to
					 * an "alarm" state.*/
					if (x == 0) // Top left
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[1] + ALARM );
						led_signalling_states[2] = ( (led_signalling_states[2] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[2] + CONCERN );

						has_concern_state = true;
					}
					else if ( x == 1) // Top right
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[1] + ALARM );
						led_signalling_states[2] = ( (led_signalling_states[2] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[2] + ALARM );
					}
				}
				else
				{
					/* Adding the enums here guarantees that
					 * multiple "concern" states will escalate to
					 * an "alarm" state.*/
					if (x == 0 && y == 0) // Top left
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[1] + CONCERN );

						has_concern_state = true;
					}
					else if ( x == 1 && y == 0 ) // Top right
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[1] + ALARM );
						led_signalling_states[2] = ( (led_signalling_states[2] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[2] + CONCERN );

						has_concern_state = true;
					}
				}
			}
			else
			{
				if ( ( AV(x, y) >= DETECTION_THRESHOLD_NRS ) )
				{
					/* Adding the enums here guarantees that
					 * multiple "concern" states will escalate to
					 * an "alarm" state.*/
					if (x == 0) // Bottom left
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[1] + CONCERN );

						has_concern_state = true;
					}
					else if ( x == 1) // Bottom right
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[1] + ALARM );
						led_signalling_states[2] = ( (led_signalling_states[2] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[2] + CONCERN );

						has_concern_state = true;
					}
				}
				else
				{
					/* Adding the enums here guarantees that
					 * multiple "concern" states will escalate to
					 * an "alarm" state.*/
					if (x == 0) // Bottom left
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[0] + CONCERN );

						has_concern_state = true;
					}
					else if ( x == 1) // Bottom right
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[1] + CONCERN );

						has_concern_state = true;
					}
				}
			}
		}
	}
}

/**
    Sends blaze messages to each LED node to update the colors based
    on the current LED signalling states.

    Hardcoded for 3x3 grids for now.
*/
void update_node_led_colors(void)
{
	for ( uint16_t i = 0; i < MAX_GRID_SIZE_X; i++ )
	{
		/* Assumes LED nodes are always at the "top" of the grid. */
		//mm_node_position_t const * node_position = get_node_at_position( i, 0);

		switch ( led_signalling_states[i] )
		{
			case CONCERN:
				//mm_led_control_update_node_leds( node_position->node_id, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW );
				break;
			case ALARM:
				//mm_led_control_update_node_leds( node_position->node_id, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED );
				break;
			case IDLE:
			default:
				//mm_led_control_update_node_leds( node_position->node_id, LED_FUNCTION_LEDS_OFF, LED_COLOURS_RED );
				break;
		}
	}
}

/**
    Gets the value of has_concern_state to determine
    if the timer should be started. True if
    a concern state LED state has been detected.
*/
bool should_start_thirty_second_timer(void)
{
	return has_concern_state;
}

/**
    Starts thirty second timer which de-escalates a concern
    state after thirty seconds. Clears has_concern_state
    back to false.
*/
void start_thirty_second_timer(void)
{
	uint32_t err_code;
	err_code = app_timer_start(m_thirty_second_timer_id, THIRTY_SECOND_TIMER_TICKS, NULL);
	APP_ERROR_CHECK(err_code);

	has_concern_state = false;
}

/**
    Determines if the activity variable located at (x, y)
    is road side or not.
*/
static bool is_road_side_av( uint16_t x, uint16_t y )
{
	return y == 0;
}

/**
    Single shot timer handler to process once-per-thirty-second updates.

    Exclusively used for de-escalating the concern AV state right now.
*/
static void thirty_second_timer_event_handler(void * p_context)
{
	for ( uint16_t i = 0; i < MAX_GRID_SIZE_X; i++ )
	{
		if ( led_signalling_states[i] == CONCERN )
		{
			led_signalling_states[i] = IDLE;
		}
	}
}
