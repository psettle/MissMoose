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
#include <stdlib.h>

#include "app_error.h"

#include "mm_led_strip_states.h"
#include "mm_activity_variables.h"
#include "mm_led_control.h"
#include "mm_position_config.h"
#include "mm_av_transmission.h"

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
	led_signalling_state_t * states;
}  output_set_t;

typedef struct
{
    output_set_t no_detection;
    output_set_t possible_detection;
    output_set_t detection;
} output_table_t;

typedef struct
{
    led_signalling_state_t      current_av_state;
    led_signalling_state_t      current_output_state;
    bool                        timeout_active;
    uint16_t                    second_counter;
} led_signalling_state_record_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Updates the current_av_states based on the raw AV values
*/
static void update_current_av_states(void);

/**
    Updates the current_output_states by considering any state changes
    and timeouts.
*/
static void update_current_output_states(void);

/**
    Sends an signalling state update to an LED node.
*/
static void set_led_output_state(int8_t x, int8_t y, led_signalling_state_t state);

/**
    Updates the LED signalling states based on an output set.
*/
static void escalate_set(led_signalling_state_record_t * p_record, output_set_t const * escalate_to);

/**
    Updates an individual LED signalling state, escalating compounding CONCERN states.
*/
static void escalate_output(led_signalling_state_t* p_state, led_signalling_state_t escalate_to);

/**
    Gets an output table for an AV based on it's location.
*/
static output_table_t const * get_output_table_for_av(uint16_t x, uint16_t y);

/**
    Gets the output set for an AV based on it's location and current value.
*/
static output_set_t const * get_output_set_for_av(mm_activity_variable_t const * av, output_table_t const * output_table);

/**
    Determines if a current_output_state has timed out.
*/
static bool has_current_output_state_timed_out(led_signalling_state_record_t const * p_record);

/**
    Resets all current_av_states.
*/
static void clear_all_current_av_states(void);

/**
	Creates an output_table_t struct. num_states indicates how many led_signalling_state_t are
	in states.
*/
/**
	Creates an output_table_t struct. num_states indicates how many led_signalling_state_t are
	in X_Y_states.
*/
static output_table_t create_output_table
(
	led_signalling_state_t const * no_detection_states,
	led_signalling_state_t const * possible_detection_states,
	led_signalling_state_t const * detection_states,
	uint16_t num_states
);

/**********************************************************
                       VARIABLES
**********************************************************/

/**
    Records for managing minimum signalling duration timeouts.
    Assumes that there are MAX_GRID_SIZE_X nodes with LEDs.
*/
//static led_signalling_state_record_t led_signalling_state_records [MAX_GRID_SIZE_X];
static led_signalling_state_record_t * led_signalling_state_records;

static mm_sensor_algorithm_config_t const * sensor_algorithm_config;

static output_table_t top_left_output;
static output_table_t top_right_output;
static output_table_t bottom_left_output;
static output_table_t bottom_right_output;
static output_table_t default_output;

/**********************************************************
                       DECLARATIONS
**********************************************************/

void mm_led_strip_states_init(mm_sensor_algorithm_config_t const * config)
{
	sensor_algorithm_config = config;

	led_signalling_state_records = malloc((sensor_algorithm_config->max_grid_size_x) * sizeof(led_signalling_state_record_t));

	/* Creates all the output tables for use in the algorithm for 3x3 grids. */
	top_left_output = create_output_table
	(
		(const led_signalling_state_t[]) {IDLE, IDLE, IDLE},
		(const led_signalling_state_t[]) {ALARM, CONCERN, IDLE},
		(const led_signalling_state_t[]) {ALARM, ALARM, CONCERN},
		sensor_algorithm_config->max_grid_size_x
	);

	top_right_output = create_output_table
	(
		(const led_signalling_state_t[]) {IDLE, IDLE, IDLE},
		(const led_signalling_state_t[]) {ALARM, ALARM, CONCERN},
		(const led_signalling_state_t[]) {ALARM, ALARM, ALARM},
		sensor_algorithm_config->max_grid_size_x
	);	
	
	bottom_left_output = create_output_table
	(
		(const led_signalling_state_t[]) {IDLE, IDLE, IDLE},
		(const led_signalling_state_t[]) {CONCERN, IDLE, IDLE},
		(const led_signalling_state_t[]) {ALARM, CONCERN, IDLE},
		sensor_algorithm_config->max_grid_size_x
	);	
	
	bottom_right_output = create_output_table
	(
		(const led_signalling_state_t[]) {IDLE, IDLE, IDLE},
		(const led_signalling_state_t[]) {ALARM, CONCERN, IDLE},
		(const led_signalling_state_t[]) {ALARM, ALARM, CONCERN},
		sensor_algorithm_config->max_grid_size_x
	);	
	
	default_output = create_output_table
	(
		(const led_signalling_state_t[]) {IDLE, IDLE, IDLE},
		(const led_signalling_state_t[]) {IDLE, IDLE, IDLE},
		(const led_signalling_state_t[]) {IDLE, IDLE, IDLE},
		sensor_algorithm_config->max_grid_size_x
	);

	/* Initialize LED signalling states */
    memset( &led_signalling_state_records[0], 0, sizeof(led_signalling_state_records) );
}

/**
    Updates current LED signalling states and increments
    timeout counter.
*/
void mm_led_signalling_states_on_second_elapsed(void)
{
    update_current_av_states();
    update_current_output_states();
}

/**
 *  When node positions are updated refresh all of the output nodes in case their state is wrong.
 */
void mm_led_signalling_states_on_position_update(void)
{
    for(int8_t i = 0; i < sensor_algorithm_config->max_grid_size_x; ++i)
    {
        set_led_output_state(i - 1, 1, led_signalling_state_records[i].current_output_state);
    }
}

/**
    Updates the current_av_states based on the raw AV values
*/
static void update_current_av_states(void)
{
    /* Clear previous states */
    clear_all_current_av_states();

    for (uint8_t x = 0; x < (mm_get_max_av_size_x()); x++)
    {
        for (uint8_t y = 0; y < (mm_get_max_av_size_y()); y++)
        {
            output_table_t const * output_table = get_output_table_for_av(x, y);
            output_set_t const * output_set = get_output_set_for_av(&AV(x, y), output_table);
            escalate_set(led_signalling_state_records, output_set);
        }
    }

    mm_av_transmission_send_all_avs();
}

/**
    Updates the current_output_states by considering any state changes
    and timeouts.
*/
static void update_current_output_states(void)
{
    for (int8_t i = 0; i < sensor_algorithm_config->max_grid_size_x; i++)
    {   
        if (led_signalling_state_records[i].current_av_state > led_signalling_state_records[i].current_output_state)
        {
            /* If the current_av_state is greater than the current_output_state,
             * start the timeout and update the output state. */
            led_signalling_state_records[i].second_counter = 0;
            led_signalling_state_records[i].timeout_active = true;
            led_signalling_state_records[i].current_output_state = led_signalling_state_records[i].current_av_state;

            /* Hardcoded right now. Assumes that the roadside nodes have LEDs. */
            set_led_output_state(i - 1, 1, led_signalling_state_records[i].current_output_state);
        }
        else if (led_signalling_state_records[i].current_av_state == led_signalling_state_records[i].current_output_state)
        {
            /* If there was no change in state, just update the timeout counter
             * if the timeout is active. */
            if (led_signalling_state_records[i].timeout_active)
            {
                led_signalling_state_records[i].second_counter++;
            }
        }
        else
        {
            /* If the current_av_state is less than the current_output_state,
             * check to see if the current_output_state has timed out.. */
            if (has_current_output_state_timed_out(&(led_signalling_state_records[i])))
            {
                /* ...if it has, turn off timeout and update current_output_state. */
                led_signalling_state_records[i].second_counter = 0;
                led_signalling_state_records[i].timeout_active = false;
                led_signalling_state_records[i].current_output_state = led_signalling_state_records[i].current_av_state;

                /* Hardcoded right now. Assumes that the roadside nodes have LEDs. */
                set_led_output_state(i - 1, 1, led_signalling_state_records[i].current_output_state);
            }
            else
            {
                /* ...otherwise, just update the timeout counter. */
                led_signalling_state_records[i].second_counter++;
            }
        }
    }
}

/**
    Sends an signalling state update to an LED node.
*/
static void set_led_output_state(int8_t x, int8_t y, led_signalling_state_t state)
{
    /* Get the position of the LED node. */
    mm_node_position_t const * node_position = get_node_for_position( x, y);

    /* Check to make sure that this node actually exists before sending anything!*/
    if (node_position != NULL)
    {
        switch ( state )
        {
            case CONCERN:
                mm_led_control_update_node_leds
                (
                    node_position->node_id,
                    LED_FUNCTION_LEDS_BLINKING,
                    LED_COLOURS_YELLOW
                );
                break;
            case ALARM:
                mm_led_control_update_node_leds
                (
                    node_position->node_id,
                    LED_FUNCTION_LEDS_BLINKING,
                    LED_COLOURS_RED
                );
                break;
            case IDLE:
            default:
                mm_led_control_update_node_leds
                (
                    node_position->node_id,
                    LED_FUNCTION_LEDS_OFF,
                    LED_COLOURS_RED
                );
                break;
        }
    }
}

/**
    Updates the LED signalling states based on an output set.
*/
static void escalate_set(led_signalling_state_record_t * p_record, output_set_t const * escalate_to)
{
    for(uint16_t i = 0; i < (uint16_t) mm_get_max_av_size_x; ++i)
    {
        escalate_output(&(p_record[i].current_av_state), escalate_to->states[i]);
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
    Gets an output table for an AV based on it's location.
*/
static output_table_t const * get_output_table_for_av(uint16_t x, uint16_t y)
{
    if (x == 0 && y == 0)
    {
        return &top_left_output;
    }
    else if ( x == 1 && y == 0)
    {
        return &top_right_output;
    }
    else if ( x == 0 && y == 1)
    {
        return &bottom_left_output;
    }
    else if ( x == 1 && y == 1)
    {
        return &bottom_right_output;
    }
    else
    {
        /* We don't support anything other than 3x3 configurations right now. */

        APP_ERROR_CHECK(true);
        return &default_output;
    }
}

/**
    Gets the output set for an AV based on it's location and current value.
*/
static output_set_t const * get_output_set_for_av(mm_activity_variable_t const * av, output_table_t const * output_table)
{
    activity_variable_state_t av_state = mm_get_status_for_av(av);

    switch (av_state)
    {
    case ACTIVITY_VARIABLE_STATE_IDLE:
        return &output_table->no_detection;
        break;
    case ACTIVITY_VARIABLE_STATE_POSSIBLE_DETECTION:
        return &output_table->possible_detection;
        break;
    case ACTIVITY_VARIABLE_STATE_DETECTION:
        return &output_table->detection;
        break;
    default:
        /* Invalid AV state. */
        APP_ERROR_CHECK(true);
        return &output_table->no_detection;
    }
}

/**
    Determines if a current_output_state has timed out.
*/
static bool has_current_output_state_timed_out(led_signalling_state_record_t const * p_record)
{
    /* If the timeout isn't running, something is wrong! */
    APP_ERROR_CHECK(!p_record->timeout_active);

    return (
            (p_record->current_output_state == CONCERN && p_record->second_counter >= sensor_algorithm_config->minimum_concern_signal_duration_s) ||
            (p_record->current_output_state == ALARM && p_record->second_counter >= sensor_algorithm_config->minimum_alarm_signal_duration_s)
           );
}

/**
    Resets all current_av_states.
*/
static void clear_all_current_av_states(void)
{
    for (uint16_t i = 0; i < sensor_algorithm_config->max_grid_size_x; i++)
    {
        led_signalling_state_records[i].current_av_state = IDLE;
    }
}

/**
	Creates an output_table_t struct. num_states indicates how many led_signalling_state_t are
	in X_Y_states.
*/
static output_table_t create_output_table
(
	led_signalling_state_t const * no_detection_states, 
	led_signalling_state_t const * possible_detection_states,
	led_signalling_state_t const * detection_states,
	uint16_t num_states
)
{
	output_table_t output_table =
	{
		{
			malloc(num_states * sizeof(led_signalling_state_t))
		},
		{
			malloc(num_states * sizeof(led_signalling_state_t))
		},
		{
			malloc(num_states * sizeof(led_signalling_state_t))
		}
	};

	memcpy(output_table.detection.states, no_detection_states, sizeof(no_detection_states));
	memcpy(output_table.possible_detection.states, possible_detection_states, sizeof(possible_detection_states));
	memcpy(output_table.detection.states, detection_states, sizeof(detection_states));

	return output_table;
}