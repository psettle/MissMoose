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
#include "mm_activity_variable_drain.h"
#include "mm_led_strip_states.h"
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
    Timer handler to process second tick.
*/
static void one_second_timer_handler(void * p_context);

/**
    Check if node positions have changed, and apply an update to all
    users if so.

    Then clear the flag.
 */
static void update_node_positions(void);

/**
 * Tick events, second, minute, etc.
 *
 * Invoked from main context.
 */
static void on_second_elapsed(void* p_unused, uint16_t size_0);
static void on_minute_elapsed(void);

/**
    Gets a timestamp accurate to 1 minute. Rolls over to 0 ever 24 hours.
*/
static uint32_t get_minute_timestamp(void);

/**********************************************************
                       VARIABLES
**********************************************************/

APP_TIMER_DEF(m_second_timer_id);

static uint32_t second_counter = 0;
static uint32_t minute_counter = 0;
static uint32_t hour_counter = 0;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Initialize algorithm intermediates, register with external interfaces.
*/
void mm_sensor_algorithm_init(mm_sensor_algorithm_config_t const * config)
{
    second_counter = 0;
    minute_counter = 0;
    hour_counter = 0;

    /* Initialize dynamic algorithm constants. */
    mm_sensor_algorithm_config_init(config);

    /* Initialize algorithm components. */
    mm_sensor_error_init();
    mm_activity_variables_init();
    mm_activity_variable_growth_init();
    mm_led_strip_states_init();

    /* Register for sensor data with sensor_transmission.h */
    mm_sensor_transmission_register_sensor_data(sensor_data_evt_handler);

    /* Initialize 1 second timer. */
    uint32_t err_code;
    err_code = app_timer_create(&m_second_timer_id, APP_TIMER_MODE_REPEATED, one_second_timer_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_second_timer_id, ONE_SECOND_TICKS, NULL);
    APP_ERROR_CHECK(err_code);
}

#ifdef MM_ALLOW_SIMULATED_TIME
/**
 * Simulate a second passing, only use for simulating time, not in production.
 */
void mm_sensor_algorithm_on_second_elapsed(void)
{
    on_second_elapsed(NULL, 0);
}
#endif

/**
    Callback into sensor_transmission.h to receive sensor data from all nodes.
*/
static void sensor_data_evt_handler(sensor_evt_t const * evt)
{
    /* Make sure everyone has valid node positions before processing the event. */
    update_node_positions();

    /* Now sensor data can be processed with respect to the algorithm. */
    mm_sensor_error_record_sensor_activity(evt, get_minute_timestamp());

    mm_activity_variable_growth_on_sensor_detection(evt);
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
    /* Make sure everyone has valid node positions before processing the event. */
    update_node_positions();

    second_counter++;
    second_counter %= SECONDS_PER_MINUTE;

    if(second_counter == 0)
    {
        on_minute_elapsed();
    }

    mm_activity_variable_growth_on_second_elapsed();
    mm_apply_activity_variable_drain_factor();
    mm_led_signalling_states_on_second_elapsed();

    /* Space left to add other once-per-second updates if
     * necessary in the future. */
}

/**
    Called every minute from main context.
*/
static void on_minute_elapsed(void)
{
    minute_counter++;
    minute_counter %= MINUTES_PER_HOUR;

    if (minute_counter == 0)
    {
        hour_counter++;
    }

    mm_sensor_error_on_minute_elapsed(get_minute_timestamp());
}

/**
    Gets a timestamp accurate to 1 minute. Rolls over to 0 ever 24 hours.
*/
static uint32_t get_minute_timestamp(void)
{
    return (hour_counter * MINUTES_PER_HOUR) + minute_counter;
}

/**
    Check if node positions have changed, and apply an update to all
    users if so.

    Then clear the flag.
 */
static void update_node_positions(void)
{
    if(have_node_positions_changed())
    {
        /* Tell all users we are about to clear the flag: */
        mm_sensor_error_on_node_positions_update(get_minute_timestamp());
        mm_led_signalling_states_on_position_update();
        /* Clear the flag: */
        clear_unread_node_positions();
    }
}
