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
#define SENSOR_DETECTION_TIMEOUT_LIMIT_TICKS(SENSOR_DETECTION_TIMEOUT_LIMIT)

/**********************************************************
                    ALGORITHM TUNING
**********************************************************/

#define ACTIVITY_VARIABLE_MIN               ( 1.0f )
#define ACTIVITY_VARIABLE_MAX               #error not implemented

#define ACTIVITY_VARIABLE_DECAY_FACTOR      ( 0.97f )
#define ACTIVITY_DECAY_PERIOD_MS            ( ONE_SECOND_MS )

#define SENSOR_DETECTION_TIMEOUT_LIMIT      ( ONE_HOUR_MS )

/**********************************************************
                          TYPES
**********************************************************/

typedef float activity_variable_t;

typedef struct
{
    uint16_t          node_id;
    sensor_type_t     sensor_type;
    sensor_rotation_t sensor_rotation;
} sensor_activity_record_t;


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
    Record that a sensor has been active (has had a detection event).
*/
static void record_sensor_activity(sensor_evt_t const * evt);

/**
    Checks to ensure all sensors have made at least 1 detection in the last 24 hours
*/
static void check_for_sensor_inactivity(void);

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
static sensor_activity_record_t mm_active_nodes_past_day[ MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE ];
static uint16_t mm_active_nodes_past_day_count = 0;
static uint16_t hour_counter = 0;

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
    Record that a sensor has been active (has had a detection event).
*/
static void record_sensor_activity(sensor_evt_t const * evt)
{
    for ( uint16_t i = 0; i < mm_active_nodes_past_day_count; i++ )
    {
        switch(mm_active_nodes_past_day[i].sensor_type)
        {
            case SENSOR_TYPE_PIR:
                if( evt->sensor_type == SENSOR_TYPE_PIR && 
                    evt->pir_data.node_id == mm_active_nodes_past_day[i].node_id && 
                    evt->pir_data.sensor_rotation == mm_active_nodes_past_day[i].sensor_rotation )
                {
                    return;
                }
                break;
            case SENSOR_TYPE_LIDAR:
                if( evt->sensor_type == SENSOR_TYPE_LIDAR &&
                    evt->lidar_data.node_id == mm_active_nodes_past_day[i].node_id &&
                    evt->lidar_data.sensor_rotation == mm_active_nodes_past_day[i].sensor_rotation )
                {
                    return;
                }
                break;
            default:
                /* App error, unknown sensor data type. */
                APP_ERROR_CHECK(true);
                break;
        }
    }
    sensor_activity_record_t activity_record;
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
    memcpy(&mm_active_nodes_past_day[mm_active_nodes_past_day_count], &activity_record, sizeof( activity_record ));
    mm_active_nodes_past_day_count++;
    APP_ERROR_CHECK_BOOL(mm_active_nodes_past_day_count > sizeof( mm_active_nodes_past_day ));
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
        for ( uint16_t j = 0; j < mm_active_nodes_past_day_count; j++)
        {
            if ( node_positions_copy[i].node_id == mm_active_nodes_past_day[j].node_id )
                    {
                        if ( mm_active_nodes_past_day[j].sensor_rotation == node_sensor_rotations[0] )
                        {
                            node_sensors_active[0] = true;
                        }
                        if ( mm_active_nodes_past_day[j].sensor_rotation == node_sensor_rotations[1] )
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
    memset(&mm_active_nodes_past_day[0], 0, sizeof( mm_active_nodes_past_day ) );
    mm_active_nodes_past_day_count = 0;
}

/**
    Timer handler to process once-per-second updates.
*/
static void timer_event_handler(void * p_context)
{
	apply_activity_variable_drain_factor();

	/* Space left to add other once-per-second updates if
	 * necessary in the future. */
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
