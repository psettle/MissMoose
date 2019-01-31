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
#include "app_scheduler.h"

#include "mm_monitoring_dispatch.h"
#include "mm_sensor_transmission.h"
#include "mm_sensor_algorithm.h"
#include "mm_activity_variables.h"
#include "mm_activity_variable_growth.h"
#include "mm_position_config.h"
#include "mm_sensor_error_check.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/* Timer macros */
#define ONE_SECOND_MS       ( 1000 )
#define ONE_SECOND_TICKS    APP_TIMER_TICKS(ONE_SECOND_MS)
#define SECONDS_PER_MINUTE  ( 60 )
#define MINUTES_PER_HOUR    ( 60  )
#define HOURS_PER_DAY       ( 24  )


/**********************************************************
                          TYPES
**********************************************************/

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
    Timer handler to process second tick.
*/
static void one_second_timer_handler(void * p_context);

/**
 * Tick events, second, minute, etc.
 * 
 * Invoked from main context.
 */
static void on_second_elapsed(void* p_unused, uint16_t size_0);
static void on_minute_elapsed(void);

/**
    Called every hour from main context.
*/
static void on_hour_elapsed(void);

/**
    Called every day from main context.
*/
static void on_day_elapsed(void);

/**********************************************************
                       VARIABLES
**********************************************************/

APP_TIMER_DEF(m_second_timer_id);

static uint8_t second_counter = 0;
static uint8_t minute_counter = 0;
static uint8_t hour_counter = 0;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Initialize algorithm intermediates, register with external interfaces.
*/
void mm_sensor_algorithm_init(void)
{
    /* Initialize algorithm components. */
    mm_activity_variables_init();
    mm_activity_variable_growth_init();

    /* Register for sensor data with sensor_transmission.h */
    mm_sensor_transmission_register_sensor_data(sensor_data_evt_handler);

    /* Initialize 1 second timer. */
    uint32_t err_code;
    err_code = app_timer_create(&m_second_timer_id, APP_TIMER_MODE_REPEATED, one_second_timer_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_second_timer_id, ONE_SECOND_TICKS, NULL);
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
    mm_record_sensor_activity(evt, minute_counter);
    // if the sensor that triggered the current event is hyperactive do not process the event further
    if ( mm_is_sensor_hyperactive(evt) )
    {
        return;
    }
    mm_activity_variable_growth_on_sensor_detection(evt);
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
    Timer handler to process second tick.
*/
static void one_second_timer_handler(void * p_context)
{
    /* Kick timer event to main. */
    uint32_t err_code = app_sched_event_put(NULL, 0, on_second_elapsed);
    APP_ERROR_CHECK(err_code);
}

/**
    Called every second from main context.
*/
static void on_second_elapsed(void* p_unused, uint16_t size_0)
{
    if(second_counter == 0)
    {
        on_minute_elapsed();
    }

    second_counter++;
    second_counter %= SECONDS_PER_MINUTE;

    if(have_node_positions_changed())
    {
        /* Tell all users we are about to clear the flag: */
        mm_activity_variable_growth_on_node_positions_update();

        /* Clear the flag: */
        clear_unread_node_positions();
    }

    mm_activity_variable_growth_on_second_elapsed();
}

/**
    Called every minute from main context.
*/
static void on_minute_elapsed(void)
{
    if(minute_counter % MINUTES_PER_HOUR == 0)
    {
        on_hour_elapsed();
    }
    minute_counter++;

    mm_check_for_sensor_hyperactivity();
}

/**
    Called every hour from main context.
*/
static void on_hour_elapsed(void)
{
    if(hour_counter == 0)
    {
        on_day_elapsed();
    }

    hour_counter++;
    hour_counter %= HOURS_PER_DAY;
}

/**
    Called every day from main context.
*/
static void on_day_elapsed(void)
{
    mm_check_for_sensor_inactivity();
}
