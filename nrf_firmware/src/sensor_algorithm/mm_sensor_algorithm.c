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


/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_GRID_SIZE_X         ( 3 )
#define MAX_GRID_SIZE_Y         ( 3 )
#define ACTIVITY_VARIABLES_NUM  ( (MAX_GRID_SIZE_X - 1) * (MAX_GRID_SIZE_Y - 1) )

/* Ease of access macros, optional usage. */
#define AV(x, y)                ( activity_variables[(x) * (MAX_GRID_SIZE_X - 1) + (y)] )
#define AV_TOP_LEFT             ( AV(0,0) )
#define AV_TOP_RIGHT            ( AV(1,0) )
#define AV_BOTTOM_LEFT          ( AV(0,1) )
#define AV_BOTTOM_RIGHT         ( AV(1,1) )

/* Timer macros */
#define ONE_SECOND_MS           ( 1000 )
#define TIMER_TICKS APP_TIMER_TICKS(ACTIVITY_DECAY_PERIOD_MS)

/**********************************************************
                    ALGORITHM TUNING
**********************************************************/

#define ACTIVITY_VARIABLE_MIN               ( 1.0f )
#define ACTIVITY_VARIABLE_MAX               #error not implemented

#define ACTIVITY_VARIABLE_DECAY_FACTOR      ( 0.97f )
#define ACTIVITY_DECAY_PERIOD_MS            ( ONE_SECOND_MS )

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
    Applies the activity variable drain factor to all activity variables.
*/
static void apply_activity_variable_drain_factor(void);

/**
    Updates the signalling state of the LEDs based on the
    current value of the activity variables.
*/
static void update_led_signalling_states(void);

/**
    Timer handler to process once-per-second updates.
*/
static void timer_event_handler(void * p_context);

/**********************************************************
                       VARIABLES
**********************************************************/

/**
     Activity variable definition, can be accessed as AV(x, y).

     X is indexed left -> right. (In direction of North-American traffic)
     Y is indexed top -> bottom. (Moving away from the road)

     Variable (x, y) is located at activity_variables[x * (MAX_GRID_SIZE_X - 1) + y]
*/
static activity_variable_t activity_variables[ ACTIVITY_VARIABLES_NUM ];

APP_TIMER_DEF(m_timer_id);

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Initialize algorithm intermediates, register with external interfaces.
*/
void mm_sensor_algorithm_init(void)
{
    /* Initialize activity variables. */
    memset(&(activity_variables[0]), 0, sizeof(activity_variables));
    AV_TOP_LEFT = ACTIVITY_VARIABLE_MIN;
    AV_TOP_RIGHT = ACTIVITY_VARIABLE_MIN;
    AV_BOTTOM_LEFT = ACTIVITY_VARIABLE_MIN;
    AV_BOTTOM_RIGHT = ACTIVITY_VARIABLE_MIN;

    /* Register for sensor data with sensor_transmission.h */
    mm_sensor_transmission_register_sensor_data(sensor_data_evt_handler);

    /* Initialize 1 second timer. */
    uint32_t err_code;
    err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_REPEATED, timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
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
    Applies the activity variable drain factor to all activity variables.
*/
static void apply_activity_variable_drain_factor(void)
{
	for ( uint16_t i = 0; i < ACTIVITY_VARIABLES_NUM; i++ )
	{
		if ( activity_variables[i] >  ACTIVITY_VARIABLE_MIN )
		{
			activity_variables[i] *= ACTIVITY_VARIABLE_DECAY_FACTOR;

			/* Enforce ACTIVITY_VARIABLE_MIN */
			if ( activity_variables[i] < ACTIVITY_VARIABLE_MIN )
			{
				activity_variables[i] = ACTIVITY_VARIABLE_MIN;
			}
		}
	}
}

/**
    Updates the signalling state of the LEDs based on the
    current value of the activity variables.

    Hardcoded for only 3x3 grids right now.
*/
static void update_led_signalling_states(void)
{
	/* Assumes that there are MAX_GRID_SIZE_X nodes
	 * with LEDs. */
	led_signalling_state_t states [MAX_GRID_SIZE_X];
	memset( &states[0], 0, sizeof(states) );

	/* TODO: loop though AVs and set signalling states,
	 * then send blaze message for LED updates. */
}

/**
    Timer handler to process once-per-second updates.
*/
static void timer_event_handler(void * p_context)
{
	apply_activity_variable_drain_factor();
	update_led_signalling_states();

	/* Space left to add other once-per-second updates if
	 * necessary in the future. */
}
