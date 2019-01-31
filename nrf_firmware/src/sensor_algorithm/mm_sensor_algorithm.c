/**
file: mm_sensor_algorithm.c
brief:
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"
#include "app_timer.h"

#include "mm_monitoring_dispatch.h"
#include "mm_sensor_transmission.h"
#include "mm_sensor_algorithm.h"
#include "mm_position_config.h"
//#include "mm_led_control.h"
#include "mm_activity_variables.h"
#include "mm_activity_variable_drain.h"
#include "mm_sensor_algorithm_config.h"


/**********************************************************
                        CONSTANTS
**********************************************************/

#define ONE_SECOND_MS           ( 1000 )
#define THIRTY_SECONDS_MS       ( 30 * ONE_SECOND_MS )
#define ONE_SECOND_TIMER_TICKS APP_TIMER_TICKS(ACTIVITY_DECAY_PERIOD_MS)
#define THIRTY_SECOND_TIMER_TICKS APP_TIMER_TICKS(THIRTY_SECONDS_MS)

/**********************************************************
                    ALGORITHM TUNING
**********************************************************/

/* Road-side (RS), non-road-side (NRS) */
#define POSSIBLE_DETECTION_THRESHOLD_RS     ( 3.0f )
#define POSSIBLE_DETECTION_THRESHOLD_NRS    ( 4.0f )

#define DETECTION_THRESHOLD_RS              ( 6.0f )
#define DETECTION_THRESHOLD_NRS             ( 7.0f )

/**********************************************************
                          TYPES
**********************************************************/

typedef float activity_variable_t;

/* LED signalling states */
typedef enum
{
    IDLE,
	CONCERN,
	ALARM
} led_signalling_state_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Callback into sensor_transmission.h to receive sensor data from all nodes.
*/
static void sensor_data_evt_handler(sensor_evt_t const * evt);

/**
    Send sensor events to the monitoring dispatch manager.
*/
static void send_monitoring_dispatch(sensor_evt_t const * evt);

/**
    Updates the signalling state of the LEDs based on the
    current value of the activity variables.

    Hardcoded for 3x3 grids for now.
*/
static void update_led_signalling_states(void);

/**
    Sends blaze messages to each LED node to update the colors based
    on the current LED signalling states.

    Hardcoded for 3x3 grids for now.
*/
static void update_node_led_colors(void);

/**
    Timer handler to process once-per-second updates.
*/
static void one_second_timer_event_handler(void * p_context);

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

static bool start_thirty_second_timer = false;

APP_TIMER_DEF(m_one_second_timer_id);
APP_TIMER_DEF(m_thirty_second_timer_id);

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Initialize algorithm intermediates, register with external interfaces.
*/
void mm_sensor_algorithm_init(void)
{
    /* Initialize activity variables. */
    mm_activity_variables_init();

    /* Initialize LED signalling states */
    memset( &led_signalling_states[0], 0, sizeof(led_signalling_states) );

    /* Register for sensor data with sensor_transmission.h */
    mm_sensor_transmission_register_sensor_data(sensor_data_evt_handler);

    /* Initialize 1 second timer. */
    uint32_t err_code;
    err_code = app_timer_create(&m_one_second_timer_id, APP_TIMER_MODE_REPEATED, one_second_timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_one_second_timer_id, ONE_SECOND_TIMER_TICKS, NULL);
    APP_ERROR_CHECK(err_code);

    /* Create thirty second timer, but don't start it yet. */
	err_code = app_timer_create(&m_thirty_second_timer_id, APP_TIMER_MODE_SINGLE_SHOT, thirty_second_timer_event_handler);
	APP_ERROR_CHECK(err_code);
}

/**
    Callback into sensor_transmission.h to receive sensor data from all nodes.
*/
static void sensor_data_evt_handler(sensor_evt_t const * evt)
{
    /* Always send sensor events to the monitoring dispatch. */
    send_monitoring_dispatch(evt);

    /* Now sensor data can be processed with respect to the algorithm. */
}

/**
    Send sensor events to the monitoring dispatch manager.
*/
static void send_monitoring_dispatch(sensor_evt_t const * evt)
{
    /* Switch on sensor type and call the appropriate monitoring dispatch function. */
    switch(evt->sensor_type)
    {
        case SENSOR_TYPE_PIR:
            mm_monitoring_dispatch_send_pir_data
                (
                evt->pir_data.node_id,
                evt->pir_data.sensor_rotation,
                evt->pir_data.detection
                );
            break;
        case SENSOR_TYPE_LIDAR:
            mm_monitoring_dispatch_send_lidar_data
                (
                evt->lidar_data.node_id,
                evt->lidar_data.sensor_rotation,
                evt->lidar_data.distance_measured
                );
            break;
        default:
            /* App error, unknown sensor data type. */
            APP_ERROR_CHECK(true);
            break;
    }
}

/**
    Updates the signalling state of the LEDs based on the
    current value of the activity variables.

    Hardcoded for only 3x3 grids right now.
*/
static void update_led_signalling_states(void)
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

						start_thirty_second_timer = true;
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

						start_thirty_second_timer = true;
					}
					else if ( x == 1 && y == 0 ) // Top right
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[1] + ALARM );
						led_signalling_states[2] = ( (led_signalling_states[2] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[2] + CONCERN );

						start_thirty_second_timer = true;
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

						start_thirty_second_timer = true;
					}
					else if ( x == 1) // Bottom right
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[1] + ALARM );
						led_signalling_states[2] = ( (led_signalling_states[2] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[2] + CONCERN );

						start_thirty_second_timer = true;
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

						start_thirty_second_timer = true;
					}
					else if ( x == 1) // Bottom right
					{
						/* Enforce enum maximum */
						led_signalling_states[0] = ( (led_signalling_states[0] + ALARM) > ALARM ) ? ALARM : ( led_signalling_states[0] + ALARM );
						led_signalling_states[1] = ( (led_signalling_states[1] + CONCERN) > ALARM ) ? ALARM : ( led_signalling_states[1] + CONCERN );

						start_thirty_second_timer = true;
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
static void update_node_led_colors(void)
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

/**
    Timer handler to process once-per-second updates.
*/
static void one_second_timer_event_handler(void * p_context)
{
	apply_activity_variable_drain_factor();
	update_led_signalling_states();
	update_node_led_colors();

	if (start_thirty_second_timer)
	{
		uint32_t err_code;
		err_code = app_timer_start(m_thirty_second_timer_id, THIRTY_SECOND_TIMER_TICKS, NULL);
		APP_ERROR_CHECK(err_code);

		start_thirty_second_timer = false;
	}
	/* Space left to add other once-per-second updates if
	 * necessary in the future. */
}

/**
    Determines if the activity variable located at (x, y)
    is road side or not.
*/
static bool is_road_side_av( uint16_t x, uint16_t y )
{
	return y == 0;
}
