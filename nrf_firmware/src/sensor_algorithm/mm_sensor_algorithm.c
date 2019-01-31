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
#include "mm_activity_variables.h"
#include "mm_activity_variable_drain.h"
#include "mm_sensor_algorithm_config.h"
#include "mm_led_strip_states.h"


/**********************************************************
                        CONSTANTS
**********************************************************/

#define ONE_SECOND_MS           ( 1000 )
#define ONE_SECOND_TIMER_TICKS APP_TIMER_TICKS(ACTIVITY_DECAY_PERIOD_MS)

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
    Timer handler to process once-per-second updates.
*/
static void one_second_timer_event_handler(void * p_context);

/**********************************************************
                       VARIABLES
**********************************************************/

APP_TIMER_DEF(m_one_second_timer_id);

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

    /* Register for sensor data with sensor_transmission.h */
    mm_sensor_transmission_register_sensor_data(sensor_data_evt_handler);

    /* Initialize 1 second timer. */
    uint32_t err_code;
    err_code = app_timer_create(&m_one_second_timer_id, APP_TIMER_MODE_REPEATED, one_second_timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_one_second_timer_id, ONE_SECOND_TIMER_TICKS, NULL);
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
    Timer handler to process once-per-second updates.
*/
static void one_second_timer_event_handler(void * p_context)
{
	apply_activity_variable_drain_factor();
	update_led_signalling_states();
	update_node_led_colors();

	if (should_start_thirty_second_timer())
	{
		start_thirty_second_timer();
	}
	/* Space left to add other once-per-second updates if
	 * necessary in the future. */
}
