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
#include "mm_position_config.h"
#include "mm_sensor_transmission.h"
#include "mm_sensor_algorithm.h"


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
#define ONE_MINUTE_MS           ( ONE_SECOND_MS * 60 )
#define ONE_HOUR_MS             ( ONE_MINUTE_MS * 60 )
#define ONE_DAY_MS              ( ONE_HOUR_MS * 24 )
#define TIMER_TICKS APP_TIMER_TICKS(ACTIVITY_DECAY_PERIOD_MS)
#define SENSOR_DETECTION_TIMEOUT_LIMIT_TICKS APP_TIMER_TICKS(SENSOR_DETECTION_TIMEOUT_LIMIT)

/**********************************************************
                    ALGORITHM TUNING
**********************************************************/

#define ACTIVITY_VARIABLE_MIN                   ( 1.0f )
#define ACTIVITY_VARIABLE_MAX                   #error not implemented

#define ACTIVITY_VARIABLE_DECAY_FACTOR          ( 0.97f )
#define ACTIVITY_DECAY_PERIOD_MS                ( ONE_SECOND_MS )

#define SENSOR_DETECTION_TIMEOUT_LIMIT          ( ONE_HOUR_MS )

#define SENSOR_HYPERACTIVITY_DETECTION_PERIOD   ( 60 ) // seconds
#define SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE  ( 120 )
#define SENSOR_HYPERACTIVITY_FREQUENCY_THRES    ( 1.0 )

/**********************************************************
                          TYPES
**********************************************************/

typedef float activity_variable_t;

typedef struct
{
    uint16_t          node_id;
    sensor_type_t     sensor_type;
    sensor_rotation_t sensor_rotation;
} sensor_record_t;

typedef struct
{
    sensor_record_t   sensor;
    uint16_t          activity_timestamps[SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE];
    uint16_t          activity_records_count;
    bool              sensor_hyperactive;
} sensor_activity_record_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Callback into sensor_transmission.h to receive sensor data from all nodes.
*/
static void sensor_data_evt_handler(sensor_evt_t const * evt);

/**
    Returns true if the sensor in the given event is marked as hyperactive
*/
static bool is_sensor_hyperactive(sensor_evt_t const * evt)

/**
    Send sensor events to the monitoring dispatch manager.
*/
static void send_monitoring_dispatch(sensor_evt_t const * evt);

/**
    Applies the activity variable drain factor to all activity variables.
*/
static void apply_activity_variable_drain_factor(void);

/**
    Record that a sensor has been active (has had a detection event).
*/
static void record_sensor_activity(sensor_evt_t const * evt);

/**
    Checks to ensure all sensors have made at least 1 detection in the last 24 hours
*/
static void check_for_sensor_inactivity(void);

/**
    Checks for any sensors which are hyperactive. Flags hyperactive sensors so their events will be ignored.
*/
static void check_for_sensor_hyperactivity(void);

/**
    Timer handler to process once-per-second updates.
*/
static void timer_event_handler(void * p_context);

/**
    Timer handler to process once-per-day updates.
*/
static void daily_timer_event_handler(void * p_context);

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
APP_TIMER_DEF(m_daily_timer_id);

static mm_node_position_t node_positions_copy[ MAX_NUMBER_NODES ];
static sensor_record_t mm_active_sensors_past_day[ MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE ];
static uint16_t mm_active_sensors_past_day_count = 0;
static sensor_activity_record_t mm_sensor_activity_record[ MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE ];
static uint16_t hour_counter = 0;
static uint16_t minute_counter = 0;
static uint16_t second_counter = 0;

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

    /* Get a copy of the current node positions for use later */
    memcpy(&node_positions_copy[0], get_node_positions(), sizeof( node_positions_copy) );

    /* Initialize 1 second timer. */
    uint32_t err_code;
    err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_REPEATED, timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
    APP_ERROR_CHECK(err_code);

    /* Initialize daily (24 hour) timer. */
    err_code = app_timer_create(&m_daily_timer_id, APP_TIMER_MODE_REPEATED, daily_timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_daily_timer_id, SENSOR_DETECTION_TIMEOUT_LIMIT_TICKS, NULL);
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
    record_sensor_activity(evt);
}

/**
    Returns true if the sensor in the given event is marked as hyperactive
*/
static bool is_sensor_hyperactive(sensor_evt_t const * evt)
{
    for ( uint16_t i = 0; i < MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE; i++)
    {
        sensor_activity_record_t * existing_record = &mm_sensor_activity_record[i];
        switch(evt->sensor_type)
        {
            case SENSOR_TYPE_PIR:
                if ( evt->pir_data.node_id == existing_record->sensor.node_id &&
                     evt->pir_data.sensor_rotation == existing_record->sensor.sensor_rotation)
                {
                    return existing_record->sensor_hyperactive;
                }
                break;
            case SENSOR_TYPE_LIDAR:
                if ( evt->lidar_data.node_id == existing_record->sensor.node_id &&
                     evt->lidar_data.sensor_rotation == existing_record->sensor.sensor_rotation)
                {
                    return existing_record->sensor_hyperactive;
                }
                break;
            default:
                /* App error, unknown sensor data type. */
                APP_ERROR_CHECK(true);
                break;
        }
    }
    return false;
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
            if ( !is_sensor_hyperactive(evt) )
            {
                mm_monitoring_dispatch_send_pir_data
                    (
                    evt->pir_data.node_id,
                    evt->pir_data.sensor_rotation,
                    evt->pir_data.detection
                    );
            }
            break;
        case SENSOR_TYPE_LIDAR:
            if ( !is_sensor_hyperactive(evt) )
            {
                mm_monitoring_dispatch_send_lidar_data
                    (
                    evt->lidar_data.node_id,
                    evt->lidar_data.sensor_rotation,
                    evt->lidar_data.distance_measured
                    );
            }
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
    Record that a sensor has been active (has had a detection event).
*/
static void record_sensor_activity(sensor_evt_t const * evt)
{
    bool first_activity_of_day = true;
    for ( uint16_t i = 0; i < mm_active_sensors_past_day_count; i++ )
    {
        switch(mm_active_sensors_past_day[i].sensor_type)
        {
            case SENSOR_TYPE_PIR:
                if( evt->sensor_type == SENSOR_TYPE_PIR && 
                    evt->pir_data.node_id == mm_active_sensors_past_day[i].node_id && 
                    evt->pir_data.sensor_rotation == mm_active_sensors_past_day[i].sensor_rotation )
                {
                    first_activity_of_day = false;
                }
                break;
            case SENSOR_TYPE_LIDAR:
                if( evt->sensor_type == SENSOR_TYPE_LIDAR &&
                    evt->lidar_data.node_id == mm_active_sensors_past_day[i].node_id &&
                    evt->lidar_data.sensor_rotation == mm_active_sensors_past_day[i].sensor_rotation )
                {
                    first_activity_of_day = false;
                }
                break;
            default:
                /* App error, unknown sensor data type. */
                APP_ERROR_CHECK(true);
                break;
        }
    }
    sensor_record_t activity_record;
    switch(evt->sensor_type)
    {
        case SENSOR_TYPE_PIR:
            activity_record.node_id = evt->pir_data.node_id;
            activity_record.sensor_type = evt->sensor_type;
            activity_record.sensor_rotation = evt->pir_data.sensor_rotation;
            break;
        case SENSOR_TYPE_LIDAR:
            activity_record.node_id = evt->lidar_data.node_id;
            activity_record.sensor_type = evt->sensor_type;
            activity_record.sensor_rotation = evt->lidar_data.sensor_rotation;
            break;
            break;
        default:
            /* App error, unknown sensor data type. */
            APP_ERROR_CHECK(true);
            break;
    }
    if ( first_activity_of_day )
    {
        memcpy(&mm_active_sensors_past_day[mm_active_sensors_past_day_count], &activity_record, sizeof( activity_record ));
        mm_active_sensors_past_day_count++;
        APP_ERROR_CHECK_BOOL(mm_active_sensors_past_day_count > MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE);
    }
    for ( uint16_t i = 0; i < MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE; i++ )
    {
        sensor_activity_record_t * existing_record = &mm_sensor_activity_record[i];
        if ( activity_record.node_id == existing_record->sensor.node_id &&
             activity_record.sensor_type == existing_record->sensor.sensor_type &&
             activity_record.sensor_rotation == existing_record->sensor.sensor_rotation )
        {
            if ( existing_record->activity_records_count == SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE )
            {
                existing_record->activity_records_count = 0;
            }
            existing_record->activity_timestamps[existing_record->activity_records_count] = minute_counter;
            existing_record->activity_records_count++;
            return;
        }
    }
    for ( uint16_t i = 0; i < MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE; i++ )
    {
        sensor_activity_record_t * existing_record = &mm_sensor_activity_record[i];
        // nodeid of 0 is an invalid entry and therefore free to overwrite
        if ( existing_record->sensor.node_id == 0 )
        {
            existing_record->sensor = activity_record;
            existing_record->sensor_hyperactive = false;
            existing_record->activity_timestamps[0] = minute_counter;
            existing_record->activity_records_count = 1;
        }
    }     
}

/**
    Checks to ensure all sensors have made at least 1 detection in the last 24 hours
*/
static void check_for_sensor_inactivity(void)
{
    if ( have_node_positions_changed() )
    {
        memset(&node_positions_copy[0], 0, sizeof( node_positions_copy ) );
        memcpy(&node_positions_copy[0], get_node_positions(), sizeof( node_positions_copy) );
    }
    for ( uint16_t i = 0; i < MAX_NUMBER_NODES; i++)
    {
        if ( !node_positions_copy[i].is_valid )
        {
            continue;
        }
        sensor_rotation_t node_sensor_rotations[MAX_SENSORS_PER_NODE];
        get_sensor_rotations(node_positions_copy[i].node_type, &node_sensor_rotations[0], &node_sensor_rotations[1]);
        bool node_sensors_active[MAX_SENSORS_PER_NODE];
        for ( uint16_t j = 0; j < mm_active_sensors_past_day_count; j++)
        {
            if ( node_positions_copy[i].node_id == mm_active_sensors_past_day[j].node_id )
                    {
                        if ( mm_active_sensors_past_day[j].sensor_rotation == node_sensor_rotations[0] )
                        {
                            node_sensors_active[0] = true;
                        }
                        if ( mm_active_sensors_past_day[j].sensor_rotation == node_sensor_rotations[1] )
                        {
                            node_sensors_active[1] = true;
                        }
                    }
        }
        for ( uint16_t j = 0; j < MAX_SENSORS_PER_NODE; j++)
        {
            if ( !node_sensors_active[j] )
            {
                // flag sensor node_sensor_rotations[k] of node_positions_copy[i].node_id as inactive
            }
        }
    }
    memset(&mm_active_sensors_past_day[0], 0, sizeof( mm_active_sensors_past_day ) );
    mm_active_sensors_past_day_count = 0;
}

/**
    Checks for any sensors which are hyperactive. Flags hyperactive sensors so their events will be ignored.
*/
static void check_for_sensor_hyperactivity(void)
{
    if ( have_node_positions_changed() )
    {
        memset(&node_positions_copy[0], 0, sizeof( node_positions_copy ) );
        memcpy(&node_positions_copy[0], get_node_positions(), sizeof( node_positions_copy) );

        for ( uint16_t i = 0; i < MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE; i++ )
        {
            sensor_activity_record_t * existing_record = &mm_sensor_activity_record[i];
            bool sensor_valid = false;
            for ( uint16_t j = 0; j < MAX_NUMBER_NODES; j++ )
            {
                mm_node_position_t * current_node = &node_positions_copy[j];
                sensor_rotation_t node_sensor_rotations[MAX_SENSORS_PER_NODE];
                get_sensor_rotations(current_node->node_type, &node_sensor_rotations[0], &node_sensor_rotations[1]);
                if ( existing_record->sensor.node_id == current_node->node_id &&
                     (existing_record->sensor.sensor_rotation == node_sensor_rotations[0] || 
                      existing_record->sensor.sensor_rotation == node_sensor_rotations[1]) )
                {
                    sensor_valid = true;
                    break;
                }
            }
            if ( !sensor_valid )
            {
                existing_record->sensor.node_id = 0; // set sensor invalid
            }
        } 
    }
    for ( uint16_t i = 0; i < MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE; i++ )
    {
        sensor_activity_record_t * existing_record = &mm_sensor_activity_record[i];
        // nodeid of 0 is an invalid entry
        if ( existing_record->sensor.node_id != 0 )
        {
            uint16_t newest_timestamp = 0;
            uint16_t oldest_timestamp = 65535; // maximum uint16_t
            for ( uint16_t j = 0; j < SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE; j++ )
            {
                uint16_t current_timestamp = existing_record->activity_timestamps[j];
                if ( current_timestamp > newest_timestamp )
                {
                    newest_timestamp = current_timestamp;
                }
                if ( current_timestamp < oldest_timestamp )
                {
                    oldest_timestamp = current_timestamp;
                }
            }
            float detection_frequency = ((float)(newest_timestamp - oldest_timestamp)) / ((float)SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE);
            existing_record->sensor_hyperactive = detection_frequency > SENSOR_HYPERACTIVITY_FREQUENCY_THRES;
        }
    }
}

/**
    Timer handler to process once-per-second updates.
*/
static void timer_event_handler(void * p_context)
{
	apply_activity_variable_drain_factor();

	/* Space left to add other once-per-second updates if
	 * necessary in the future. */

    // once per minute events
    second_counter++;
    if ( second_counter == SENSOR_HYPERACTIVITY_DETECTION_PERIOD )
    {
        second_counter = 0;
        minute_counter++;

        check_for_sensor_hyperactivity();

        /* Space left to add other once-per-minute updates if
        * necessary in the future. */
    }
}

/**
    Timer handler to process once-per-day updates.
*/
static void daily_timer_event_handler(void * p_context)
{
    hour_counter++;
    //  this timer triggers every hour but we only want to execute behaviour every 24 hours
    if ( hour_counter == 24 )
    {
        hour_counter = 0;

        check_for_sensor_inactivity();

        /* Space left to add other once-per-day updates if
	     * necessary in the future. */
    }
}
