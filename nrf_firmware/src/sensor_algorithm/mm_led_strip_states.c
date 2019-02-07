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

/**********************************************************
                        MACROS
**********************************************************/

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

typedef struct
{
    led_signalling_state_t states[MAX_GRID_SIZE_X];
}  output_set_t;

typedef struct
{
    output_set_t no_detection;
    output_set_t possible_detection;
    output_set_t detection;
} output_table_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Determines if the activity variable located at (x, y)
    is road side or not.
*/
static bool is_road_side_av( uint16_t x, uint16_t y );

/**
    Updates the LED signalling states based on an output set.
*/
static void escalate_set(led_signalling_state_t* p_state, output_set_t const * escalate_to);

/**
    Updates an individual LED signalling state, escalating compounding CONCERN states.
*/
static void escalate_output(led_signalling_state_t* p_state, led_signalling_state_t escalate_to);

/**
    Determines if the AV is below the detection threshold.
*/
static bool is_av_below_detection_threshold(bool is_road_side, mm_activity_variable_t av);

/**
    Gets an output table for an AV based on it's location.
*/
static output_table_t get_output_table_for_av(uint16_t x, uint16_t y);

/**
    Gets the output set for an AV based on it's location and current value.
*/
static output_set_t get_output_set_for_av(bool is_road_side, mm_activity_variable_t av, output_table_t const * output_table);

/**********************************************************
                       VARIABLES
**********************************************************/

/* Assumes that there are MAX_GRID_SIZE_X nodes with LEDs. */
static led_signalling_state_t led_signalling_states [MAX_GRID_SIZE_X];

static output_table_t const top_left_output =
{
    {
        { IDLE, IDLE, IDLE }
    },
    {
        { ALARM, CONCERN, IDLE }
    },
    {
        { ALARM, ALARM, CONCERN }
    }
};

static output_table_t const top_right_output =
{
    {
        { IDLE, IDLE, IDLE }
    },
    {
        { ALARM, ALARM, CONCERN }
    },
    {
        { ALARM, ALARM, ALARM }
    }
};

static output_table_t const bottom_left_output =
{
    {
        { IDLE, IDLE, IDLE }
    },
    {
        { CONCERN, IDLE, IDLE }
    },
    {
        { ALARM, CONCERN, IDLE }
    }
};

static output_table_t const bottom_right_output =
{
    {
        { IDLE, IDLE, IDLE }
    },
    {
        { ALARM, CONCERN, IDLE }
    },
    {
        { ALARM, ALARM, CONCERN }
    }
};

static output_table_t const default_output =
{
    {
        { IDLE, IDLE, IDLE }
    },
    {
        { IDLE, IDLE, IDLE }
    },
    {
        { IDLE, IDLE, IDLE }
    }
};

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Creates the 30 second timer responsible for de-escalating
    the "concern" state, but does not start it.
*/
void mm_led_strip_states_init(void)
{
    /* Initialize LED signalling states */
    memset( &led_signalling_states[0], 0, sizeof(led_signalling_states) );
}

/**
    Updates the signalling state of the LEDs based on the
    current value of the activity variables.

    Hardcoded for only 3x3 grids right now.
*/
void mm_update_led_signalling_states(void)
{
    for (uint16_t x = 0; x < (MAX_AV_SIZE_X); x++)
    {
        for (uint16_t y = 0; y < (MAX_AV_SIZE_Y); y++)
        {
            output_table_t output_table = get_output_table_for_av(x, y);
            output_set_t output_set = get_output_set_for_av(is_road_side_av(x, y), AV(x, y), &output_table);
            escalate_set(led_signalling_states, &output_set);
        }
    }
}

/**
    Sends blaze messages to each LED node to update the colors based
    on the current LED signalling states.

    Hardcoded for 3x3 grids for now.
*/
void mm_update_node_led_colors(void)
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
    Determines if the activity variable located at (x, y)
    is road side or not.

    Right now it only uses y to decide this but maybe it
    would want to use both coordinates in the future?
*/
static bool is_road_side_av( uint16_t x, uint16_t y )
{
	return y == 0;
}

/**
    Updates the LED signalling states based on an output set.
*/
static void escalate_set(led_signalling_state_t* p_state, output_set_t const * escalate_to)
{
    for(uint8_t i = 0; i < MAX_AV_SIZE_X; ++i)
    {
        escalate_output(&p_state[i], escalate_to->states[i]);
    }
}

/**
    Updates an individual LED signalling state, escalating compounding CONCERN states.
*/
static void escalate_output(led_signalling_state_t* p_state, led_signalling_state_t escalate_to)
{
    if(*p_state == CONCERN && escalate_to == CONCERN)
    {
        *p_state = ALARM;
    }
    else if( escalate_to > *p_state)
    {
        *p_state = escalate_to;
    }
}

/**
    Determines if the AV is below the detection threshold.
*/
static bool is_av_below_detection_threshold(bool is_road_side, mm_activity_variable_t av)
{
    return (
            (is_road_side && ( av < POSSIBLE_DETECTION_THRESHOLD_RS))) ||
            (!is_road_side && ( av < POSSIBLE_DETECTION_THRESHOLD_NRS)
           );
}

/**
    Gets an output table for an AV based on it's location.
*/
static output_table_t get_output_table_for_av(uint16_t x, uint16_t y)
{
    if (x == 0 && y == 0)
    {
        return top_left_output;
    }
    else if ( x == 1 && y == 0)
    {
        return top_right_output;
    }
    else if ( x == 0 && y == 1)
    {
        return bottom_left_output;
    }
    else if ( x == 1 && y == 1)
    {
        return bottom_right_output;
    }
    else
    {
        return default_output;
    }
}

/**
    Gets the output set for an AV based on it's location and current value.
*/
static output_set_t get_output_set_for_av(bool is_road_side, mm_activity_variable_t av, output_table_t const * output_table)
{
    if (is_av_below_detection_threshold(is_road_side, av))
    {
        return output_table->no_detection;
    }

    if (is_road_side)
    {
        if (av > DETECTION_THRESHOLD_RS)
        {
            return output_table->detection;
        }
        else
        {
            return output_table->possible_detection;
        }
    }

    if (av > DETECTION_THRESHOLD_NRS)
    {
        return output_table->detection;
    }
    else
    {
        return output_table->possible_detection;
    }
}

