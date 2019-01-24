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
#define MAX_AV_SIZE_X           ( MAX_GRID_SIZE_X - 1 ) 
#define MAX_GRID_SIZE_Y         ( 3 )
#define MAX_AV_SIZE_Y           ( MAX_GRID_SIZE_Y - 1 ) 
#define ACTIVITY_VARIABLES_NUM  ( MAX_AV_SIZE_X * MAX_AV_SIZE_Y )
#define MAX_ADJACENT_ACTIVITY_VARIABLES ( 2 )   /* The max number of regions a sensor can affect per detection. */
#define MAX_SENSOR_COUNT        ( MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE )     
#define NODE_SEPERATION_CM      ( 700 )
#define NODE_OFFSET_SCALE_CM      ( 5 )

/* Ease of access macros, optional usage. */
#define AV(x, y)                ( activity_variables[(x) * MAX_AV_SIZE_X + (y)] )
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
#define ACTIVITY_VARIABLE_MAX                   ( 12.0f )

#define ACTIVITY_VARIABLE_DECAY_FACTOR          ( 0.97f )
#define ACTIVITY_DECAY_PERIOD_MS                ( ONE_SECOND_MS )

#define SENSOR_DETECTION_TIMEOUT_LIMIT          ( ONE_HOUR_MS )

#define SENSOR_HYPERACTIVITY_DETECTION_PERIOD   ( 60 ) // seconds
#define SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE  ( 120 )
#define SENSOR_HYPERACTIVITY_FREQUENCY_THRES    ( 1.0 )

#define COMMON_SENSOR_WEIGHT_FACTOR         ( 1.0f )
#define BASE_SENSOR_WEIGHT_FACTOR_PIR       ( 3.0f )
#define BASE_SENSOR_WEIGHT_FACTOR_LIDAR     ( 3.5f )
#define ROAD_PROXIMITY_FACTOR_0             ( 1.4f )
#define ROAD_PROXIMITY_FACTOR_1             ( 1.2f )
#define ROAD_PROXIMITY_FACTOR_2             ( 1.0f )

#define COMMON_SENSOR_TRICKLE_FACTOR        ( 1.0f )
#define BASE_SENSOR_TRICKLE_FACTOR_PIR      ( 1.003f )
#define BASE_SENSOR_TRICKLE_FACTOR_LIDAR    ( 1.0035f )
#define ROAD_TRICKLE_PROXIMITY_FACTOR_0     ( 1.004f )
#define ROAD_TRICKLE_PROXIMITY_FACTOR_1     ( 1.002f )
#define ROAD_TRICKLE_PROXIMITY_FACTOR_2     ( 1.0f )

/**********************************************************
                          TYPES
**********************************************************/

typedef float activity_variable_t;

typedef struct
{
    activity_variable_t* avs[MAX_ADJACENT_ACTIVITY_VARIABLES];
    uint8_t              av_count;
} activity_variable_set_t;

typedef struct
{
    uint16_t          node_id;
    sensor_type_t     sensor_type;
    sensor_rotation_t sensor_rotation;
} sensor_record_t;

typedef enum
{
    LIDAR_REGION_REGION_0,    /* The lidar is currently detecting something in the region in front of it */
    LIDAR_REGION_REGION_1,    /* The lidar is currently detecting something in the region 1 past it */
    
    LIDAR_REGION_REGION_NONE, /* The lidar is currently 'detecting' something past the edge of the network */
    LIDAR_REGION_COUNT
} lidar_region_t;

typedef struct
{
    lidar_region_t   region;              /* Which region the measured distance falls in. */
    mm_node_position_t effective_position;  /* Where the measurement effectively came from, may not be a real node. */
} lidar_get_region_out_data_t;

typedef struct
{
    uint16_t          node_id;
    sensor_type_t     sensor_type;
    sensor_rotation_t sensor_rotation;
    union
    {
        bool             pir_detected;
        lidar_region_t lidar_region;
    };
    bool is_valid;
} sensor_detection_record_t;

typedef struct
{
    float common_sensor_weight_factor;
    float base_sensor_weight_factor;
    float road_proximity_factor;
} activity_variable_sensor_constants_t;

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
    Check position_config.h to see if the node positions have changed,
    and fetch the new list if needed.
*/
static void fetch_node_positions(void);

/**
    Callback into sensor_transmission.h to receive sensor data from all nodes.
*/
static void sensor_data_evt_handler(sensor_evt_t const * evt);

/**
    Returns true if the sensor in the given event is marked as hyperactive
*/
static bool is_sensor_hyperactive(sensor_evt_t const * evt);

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

/**
 * Apply growth factors to activity variables on sensor detection.
 */ 
static void apply_activity_variable_addition(sensor_evt_t const * evt);

/**
 * Apply growth factors to activity variables on pir sensor detection.
 */ 
static void apply_activity_variable_addition_pir(sensor_evt_t const * evt);

/**
 * Apply growth factors to activity variables on lidar sensor detection.
 */ 
static void apply_activity_variable_addition_lidar(sensor_evt_t const * evt);

/**
 * Apply persistent growth factors to detecting sensors
 */
static void apply_activity_variable_trickle_factor(void);

/**
 * Apply trickle factors to activity variables for records.
 */ 
static void apply_activity_variable_trickle_to_record(sensor_detection_record_t const * record);

/**
 * Apply growth given avs and constants.
 */ 
static void grow_activity_variables(activity_variable_set_t* av_set, activity_variable_sensor_constants_t const * constants);

/**
 * Fetch a detection record for a sensor, create one if needed.
 */ 
static sensor_detection_record_t* get_sensor_record(sensor_evt_t const * evt);

/**
 * Set a new sensor detection record to initial values.
 */ 
static void init_sensor_detection_record(sensor_evt_t const * evt, sensor_detection_record_t* p_record);

/**
 * Calulate which AV region a lidar detection occured in.
 * 
 * [out] which region, a position to consider the detection from, given that it may not be adjacent to the
 *                     sensors position
 */
static void lidar_get_region(lidar_evt_data_t const * evt, lidar_get_region_out_data_t* out_data);

/**
 * Fetch a virtual location for a lidar detection using the detection region and the original sensor position.
 */
static void get_effective_location_for_lidar_detection(mm_node_position_t const * position, sensor_detection_record_t const * record, mm_node_position_t* effective_position);

/**
 * Calculate which activity variable regions are adjacent to a made detection.
 * 
 * [out] the set of activity variables that are adjacent.
 */
static void find_adjacent_activity_variables(mm_node_position_t const * position, sensor_rotation_t sensor_rotation, activity_variable_set_t* av_set);

/**
 * Check a set of AV coordinates for validity, then add to av_set if valid.
 */
static void check_add_av(uint8_t x, uint8_t y, activity_variable_set_t* av_set);

/**********************************************************
                       VARIABLES
**********************************************************/

/**
     Activity variable definition, can be accessed as AV(x, y).

     X is indexed left -> right. (In direction of North-American traffic)
     Y is indexed top -> bottom. (Moving away from the road)

     Variable (x, y) is located at activity_variables[x * MAX_AV_SIZE_X + y]
*/
static activity_variable_t activity_variables[ ACTIVITY_VARIABLES_NUM ];

APP_TIMER_DEF(m_timer_id);
APP_TIMER_DEF(m_daily_timer_id);

static mm_node_position_t node_positions[ MAX_NUMBER_NODES ];

static sensor_record_t mm_active_sensors_past_day[MAX_SENSOR_COUNT];
static uint16_t mm_active_sensors_past_day_count = 0;

static sensor_detection_record_t sensor_detection_records[MAX_SENSOR_COUNT];

static sensor_activity_record_t mm_sensor_activity_record[MAX_SENSOR_COUNT];

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
    memset(&node_positions[0], 0, sizeof(node_positions));
    fetch_node_positions();

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

    //check for sensor error
    record_sensor_activity(evt);


    apply_activity_variable_addition(evt);
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
    fetch_node_positions();

    for ( uint16_t i = 0; i < MAX_NUMBER_NODES; i++)
    {
        if ( !node_positions[i].is_valid )
        {
            continue;
        }
        sensor_rotation_t node_sensor_rotations[MAX_SENSORS_PER_NODE];
        get_sensor_rotations(node_positions[i].node_type, &node_sensor_rotations[0], &node_sensor_rotations[1]);
        bool node_sensors_active[MAX_SENSORS_PER_NODE];
        for ( uint16_t j = 0; j < mm_active_sensors_past_day_count; j++)
        {
            if ( node_positions[i].node_id == mm_active_sensors_past_day[j].node_id )
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
    fetch_node_positions();
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
    Check for change in node positions, and fetch if needed.
*/
static void fetch_node_positions(void)
{
    if( have_node_positions_changed() )
    {
        memcpy(&node_positions[0], get_node_positions(), sizeof( node_positions) );

        /* Update hyperactivity records */
        for ( uint16_t i = 0; i < MAX_SENSOR_COUNT; i++ )
        {
            sensor_activity_record_t * existing_record = &mm_sensor_activity_record[i];
            bool sensor_valid = false;
            for ( uint16_t j = 0; j < MAX_NUMBER_NODES; j++ )
            {
                mm_node_position_t * current_node = &node_positions[j];
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


        clear_unread_node_positions();
    }
}

/**
    Timer handler to process once-per-second updates.
*/
static void timer_event_handler(void * p_context)
{
	apply_activity_variable_drain_factor();
    apply_activity_variable_trickle_factor();

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

/**
 * Apply growth factors to activity variables on sensor detection.
 */ 
static void apply_activity_variable_addition(sensor_evt_t const * evt)
{
    switch(evt->sensor_type)
    {
        case SENSOR_TYPE_PIR:
            apply_activity_variable_addition_pir(evt);
            break;
        case SENSOR_TYPE_LIDAR:
            apply_activity_variable_addition_lidar(evt);
            break;
        default:
            /* Unknown sensor type error. */
            APP_ERROR_CHECK(true);
            break;
    }
}

/**
 * Apply persistent growth factors to detecting sensors
 */
static void apply_activity_variable_trickle_factor(void)
{
    for(uint16_t i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        apply_activity_variable_trickle_to_record(&(sensor_detection_records[i]));
    }
}

/**
 * Apply growth factors to activity variables on pir detection.
 */ 
static void apply_activity_variable_addition_pir(sensor_evt_t const * evt)
{
    pir_evt_data_t const * pir_evt = &evt->pir_data;

    /* Do we know the node that generated the event? */
    fetch_node_positions();
    mm_node_position_t const * position = get_position_for_node(pir_evt->node_id);

    if(NULL == position)
    {
        /* We don't know the node, so we can't use its data. */
        return;
    }

    /* Get the sensor record. (A record will be allocted if one does not exist.) */
    sensor_detection_record_t* record = get_sensor_record(evt);

    /* Does this event represent a new detection? (Easy for PIR sensors) */
    bool is_new_detection = pir_evt->detection && !record->pir_detected;

    /* Apply growth */
    if(is_new_detection)
    {
        /* Which AV's does the detection apply to? */
        activity_variable_set_t av_set;
        find_adjacent_activity_variables(position, pir_evt->sensor_rotation, &av_set);

        /* Generate a sensor constants structure. */
        activity_variable_sensor_constants_t constants;

        memset(&constants, 0, sizeof(constants));
        constants.common_sensor_weight_factor = COMMON_SENSOR_WEIGHT_FACTOR;
        constants.base_sensor_weight_factor   = BASE_SENSOR_WEIGHT_FACTOR_PIR;

        switch(position->grid_position_y)
        {
            case -1:
                constants.road_proximity_factor = ROAD_PROXIMITY_FACTOR_0;
                break;
            case 0:
                constants.road_proximity_factor = ROAD_PROXIMITY_FACTOR_1;
                break;
            case 1:
                constants.road_proximity_factor = ROAD_PROXIMITY_FACTOR_2;
                break;
            default:
                /* Invalid grid position given grid height */
                APP_ERROR_CHECK(true);
                break;
        }

        grow_activity_variables(&av_set, &constants);
    }

    /* Update sensor record so we can correctly interpret the next signal. */
    record->pir_detected = pir_evt->detection;  
}

/**
 * Apply growth factors to activity variables on lidar detection.
 */ 
static void apply_activity_variable_addition_lidar(sensor_evt_t const * evt)
{
    lidar_evt_data_t const * lidar_evt = &evt->lidar_data;

    /* Do we know the node that generated the event? */
    fetch_node_positions();
    mm_node_position_t const * position = get_position_for_node(lidar_evt->node_id);

    if(NULL == position)
    {
        /* We don't know the node, so we can't use its data. */
        return;
    }

    /* Get the sensor record. (A record will be allocted if one does not exist.) */
    sensor_detection_record_t* record = get_sensor_record(evt);

    /* What region does the measured distance fall into? */
    lidar_get_region_out_data_t out_data;
    lidar_get_region(lidar_evt, &out_data);
    lidar_region_t region = out_data.region;
    position = &out_data.effective_position;

    /* Does this event represent a new detection?
       Is it in a detectable region and different than last measurement? */
    bool is_new_detection = ((region == LIDAR_REGION_REGION_0) || (region == LIDAR_REGION_REGION_1)) && (region != record->lidar_region);

    /* Apply growth */
    if(is_new_detection)
    {
        /* Which AV's does the detection apply to? */
        activity_variable_set_t av_set;
        find_adjacent_activity_variables(position, lidar_evt->sensor_rotation, &av_set);

        /* Generate a sensor constants structure. */
        activity_variable_sensor_constants_t constants;

        memset(&constants, 0, sizeof(constants));
        constants.common_sensor_weight_factor = COMMON_SENSOR_WEIGHT_FACTOR;
        constants.base_sensor_weight_factor   = BASE_SENSOR_WEIGHT_FACTOR_LIDAR;

        switch(position->grid_position_y)
        {
            case -1:
                constants.road_proximity_factor = ROAD_PROXIMITY_FACTOR_0;
                break;
            case 0:
                constants.road_proximity_factor = ROAD_PROXIMITY_FACTOR_1;
                break;
            case 1:
                constants.road_proximity_factor = ROAD_PROXIMITY_FACTOR_2;
                break;
            default:
                /* Invalid grid position given grid height */
                APP_ERROR_CHECK(true);
                break;
        }

        grow_activity_variables(&av_set, &constants);
    }

    /* Update sensor record so we can correctly interpret the next signal. */
    record->lidar_region = region;
}

/**
 * Apply trickle factors to activity variables for records.
 */ 
static void apply_activity_variable_trickle_to_record(sensor_detection_record_t const * record)
{
    /* Is the record detecting? */
    switch(record->sensor_type)
    {
        case SENSOR_TYPE_PIR:
            if(!record->pir_detected)
            {
                return;
            }
            break;
        case SENSOR_TYPE_LIDAR:
            if(record->lidar_region == LIDAR_REGION_REGION_NONE)
            {
                return;
            }
            break;
        default:
            /* Unknown sensor type error. */
            APP_ERROR_CHECK(true);
            break;
    }


    /* Do we know the node that generated the event? */
    fetch_node_positions();
    mm_node_position_t const * position = get_position_for_node(record->node_id);

    if(NULL == position)
    {
        /* We don't know the node, so we can't use its data. */
        return;
    }

    /* Apply growth */

    /* Grab the effective position given the detecting region */
    mm_node_position_t effective_position;
    if(SENSOR_TYPE_LIDAR == record->sensor_type)
    {
        get_effective_location_for_lidar_detection(position, record, &effective_position);
        position = &effective_position;
    }
    
    /* Which AV's does the detection apply to? */
    activity_variable_set_t av_set;
    find_adjacent_activity_variables(position, record->sensor_rotation, &av_set);

    /* Generate a sensor constants structure. */
    activity_variable_sensor_constants_t constants;

    memset(&constants, 0, sizeof(constants));
    constants.common_sensor_weight_factor = COMMON_SENSOR_TRICKLE_FACTOR;
    constants.base_sensor_weight_factor   = BASE_SENSOR_TRICKLE_FACTOR_PIR;

    switch(position->grid_position_y)
    {
        case -1:
            constants.road_proximity_factor = ROAD_TRICKLE_PROXIMITY_FACTOR_0;
            break;
        case 0:
            constants.road_proximity_factor = ROAD_TRICKLE_PROXIMITY_FACTOR_1;
            break;
        case 1:
            constants.road_proximity_factor = ROAD_TRICKLE_PROXIMITY_FACTOR_2;
            break;
        default:
            /* Invalid grid position given grid height */
            APP_ERROR_CHECK(true);
            break;
    }

    grow_activity_variables(&av_set, &constants);
}

/**
 * Apply growth constants to a set of activity variables.
 */
static void grow_activity_variables(activity_variable_set_t* av_set, activity_variable_sensor_constants_t const * constants)
{
    for(uint16_t i = 0; i < av_set->av_count; ++i)
    {
        /* Calculate the factor */
        float factor = 1.0f;

        factor *= constants->common_sensor_weight_factor;
        factor *= constants->base_sensor_weight_factor;
        factor *= constants->road_proximity_factor;
        //factor *= constants->sensor_proximity_factor; /* Only close avs show up in av_set */

        /* Apply the factor */
        *(av_set->avs[i]) *= factor;

        /* Check if max exceeded */
        if(*(av_set->avs[i]) > ACTIVITY_VARIABLE_MAX)
        {
            *(av_set->avs[i]) = ACTIVITY_VARIABLE_MAX;
        }
    }
}

/**
 * Calculate which region a lidar detection belongs to
 */ 
static void lidar_get_region(lidar_evt_data_t const * evt, lidar_get_region_out_data_t* out_data)
{
    /* The regions can shift around a lot due to the offset system, so we need 2 values:
            - distance to first node
            - distance to second node
        keep in mind that either of those nodes may not exist, so we will populate a 'default' position for them in that case */
    mm_node_position_t const * position = get_position_for_node(evt->node_id);
    
    /* Should have been checked by caller. */
    APP_ERROR_CHECK(!position);

    int8_t x = position->grid_position_x;
    int8_t y = position->grid_position_y;
    node_rotation_t rotation = evt->sensor_rotation + position->node_rotation;

    int8_t dx = 0;
    int8_t dy = 0;

    switch(rotation)
    {
        case NODE_ROTATION_0:
            /* Next node is in +y direction. */
            dy++;
            break;
        case NODE_ROTATION_90:
            /* Next node is in +x direction. */
            dx++;   
            break;
        case NODE_ROTATION_180:
            /* Next node is in -y direction. */
            dy--;
            break;
        case NODE_ROTATION_270:
            /* Next node is in -x direction. */
            dx--;
            break;
        default:
            /* Current design does not support intermediate angles. */
            APP_ERROR_CHECK(true);
            break;
    }

    /* Fetch positions for 1 forward and 2 forward, replace with local mockups where nodes don't exist. */
    mm_node_position_t const * position_1_forward = get_node_for_position(x + dx, y + dy);   
    mm_node_position_t const * position_2_forward = get_node_for_position(x + 2 * dx, y + 2 * dy);

    mm_node_position_t position_1_forward_backup;
    mm_node_position_t position_2_forward_backup;

    if(position_1_forward == NULL)
    {
        position_1_forward = &position_1_forward_backup;
        memset(&position_1_forward_backup, 0, sizeof(position_1_forward_backup));
        position_1_forward_backup.grid_position_x = x + dx;
        position_1_forward_backup.grid_position_y = y + dy;
        position_1_forward_backup.node_rotation = position->node_rotation;
    }

    if(position_2_forward == NULL)
    {
        position_2_forward = &position_2_forward_backup;
        memset(&position_2_forward_backup, 0, sizeof(position_2_forward_backup));
        position_2_forward_backup.grid_position_x = x + 2 * dx;
        position_2_forward_backup.grid_position_y = y + 2 * dy;
        position_2_forward_backup.node_rotation = position->node_rotation;
    }

    /* Fetch relevant offsets for distance calculation: */
    int8_t node_0_forward_offset = 0;
    int8_t node_1_forward_offset = 0;
    int8_t node_2_forward_offset = 0;

    switch(rotation)
    {
        case NODE_ROTATION_0:
            /* Next node is in +y direction. */
            node_0_forward_offset = position->grid_offset_y;
            node_1_forward_offset = position_1_forward->grid_offset_y;
            node_2_forward_offset = position_2_forward->grid_offset_y;
            break;
        case NODE_ROTATION_90:
            /* Next node is in +x direction. */
            node_0_forward_offset = position->grid_offset_x;
            node_1_forward_offset = position_1_forward->grid_offset_x;
            node_2_forward_offset = position_2_forward->grid_offset_x;
            break;
        case NODE_ROTATION_180:
            /* Next node is in -y direction. */
            node_0_forward_offset = -position->grid_offset_y;
            node_1_forward_offset = -position_1_forward->grid_offset_y;
            node_2_forward_offset = -position_2_forward->grid_offset_y;
            break;
        case NODE_ROTATION_270:
            /* Next node is in -x direction. */
            node_0_forward_offset = -position->grid_offset_x;
            node_1_forward_offset = -position_1_forward->grid_offset_x;
            node_2_forward_offset = -position_2_forward->grid_offset_x;
            break;
        default:
            /* Current design does not support intermediate angles. */
            APP_ERROR_CHECK(true);
            break;
    }

    uint16_t distance_to_1_node_cm = NODE_SEPERATION_CM + (node_1_forward_offset - node_0_forward_offset) * NODE_OFFSET_SCALE_CM;
    uint16_t distance_to_2_node_cm = 2 * NODE_SEPERATION_CM + (node_2_forward_offset - node_0_forward_offset) * NODE_OFFSET_SCALE_CM;

    /* Now we know distances to both regions... is the distance within it? */
    uint16_t distance_measured = evt->distance_measured;

    if(distance_measured < distance_to_1_node_cm)
    {
        out_data->region = LIDAR_REGION_REGION_0;
        memcpy(&(out_data->effective_position), position, sizeof(out_data->effective_position));
    }
    else if(distance_measured < distance_to_2_node_cm)
    {
        out_data->region = LIDAR_REGION_REGION_1;
        memcpy(&(out_data->effective_position), position_1_forward, sizeof(out_data->effective_position));

        /* Set the node to have the same rotation so that the normalized sensor direction is the same as from the node the sensor is physically on. */
        out_data->effective_position.node_rotation = position->node_rotation;
    }
    else
    {
        out_data->region = LIDAR_REGION_REGION_NONE;
        memset(&(out_data->effective_position), 0, sizeof(out_data->effective_position));
    }
}

/**
 * Fetch a virtual location for a lidar detection using the detection region and the original sensor position.
 */
static void get_effective_location_for_lidar_detection(mm_node_position_t const * position, sensor_detection_record_t const * record, mm_node_position_t* effective_position)
{
    /* Should not be fetching an effective position if it wasn't a detection. */
    APP_ERROR_CHECK(record->lidar_region == LIDAR_REGION_REGION_NONE);

    if(record->lidar_region == LIDAR_REGION_REGION_0)
    {
        /* If first region, it came from the original position. */
        memcpy(effective_position, position, sizeof(mm_node_position_t));
        return;
    }

    /* Now we need to mock out a position in the direction of the sensor: */
    int8_t x = position->grid_position_x;
    int8_t y = position->grid_position_y;
    node_rotation_t rotation = record->sensor_rotation + position->node_rotation;

    int8_t dx = 0;
    int8_t dy = 0;

    switch(rotation)
    {
        case NODE_ROTATION_0:
            /* Next node is in +y direction. */
            dy++;
            break;
        case NODE_ROTATION_90:
            /* Next node is in +x direction. */
            dx++;   
            break;
        case NODE_ROTATION_180:
            /* Next node is in -y direction. */
            dy--;
            break;
        case NODE_ROTATION_270:
            /* Next node is in -x direction. */
            dx--;
            break;
        default:
            /* Current design does not support intermediate angles. */
            APP_ERROR_CHECK(true);
            break;
    }

    memset(effective_position, 0, sizeof(mm_node_position_t));
    effective_position->grid_position_x = x + dx;
    effective_position->grid_offset_y = y + dy;
    effective_position->node_rotation = position->node_rotation; 
}

/**
 * Fetch a record for sensor detection, create one if needed.
 */
static sensor_detection_record_t* get_sensor_record(sensor_evt_t const * evt)
{
    sensor_detection_record_t * p_empty = NULL;
    sensor_detection_record_t const * p_list_end = &sensor_detection_records[MAX_SENSOR_COUNT];
 
    uint16_t node_id;
    sensor_rotation_t rotation;

    switch(evt->sensor_type)
    {
        case SENSOR_TYPE_PIR:
            node_id = evt->pir_data.node_id;
            rotation = evt->pir_data.sensor_rotation;
            break;
        case SENSOR_TYPE_LIDAR:
            node_id = evt->lidar_data.node_id;
            rotation = evt->lidar_data.sensor_rotation;
            break;
        default:
            /* Unknown sensor type error. */
            APP_ERROR_CHECK(true);
            return NULL;
    }


    /* Search for existing record */
    for(sensor_detection_record_t* p_list =  &(sensor_detection_records[0]); p_list <= p_list_end; ++p_list)
    {
        if(!p_list->is_valid)
        {
            /* Grab an empty slot while we're here */
            if(NULL == p_empty)
            {
                p_empty = p_list;
            }

            continue;
        }

        if(p_list->node_id != node_id)
        {
            continue;
        }

        if(p_list->sensor_rotation != rotation)
        {
            continue;
        }

        /* Existing record */
        return p_list;
    }

    /* No existing record, should have grabbed a slot while looking */
    APP_ERROR_CHECK(!p_empty);

    /* Create and return a new record. */
    init_sensor_detection_record(evt, p_empty);
    return p_empty;
}

/**
 * Initialize a new sensor detection record.
 */
static void init_sensor_detection_record(sensor_evt_t const * evt, sensor_detection_record_t* p_record)
{
    p_record->sensor_type = evt->sensor_type;
    p_record->is_valid = true;

    switch(evt->sensor_type)
    {
        case SENSOR_TYPE_PIR:
            p_record->node_id = evt->pir_data.node_id;
            p_record->sensor_rotation = evt->pir_data.sensor_rotation;
            p_record->pir_detected = false; /* Pir's assumed off to start. */
            break;
        case SENSOR_TYPE_LIDAR:
            p_record->node_id = evt->lidar_data.node_id;
            p_record->sensor_rotation = evt->lidar_data.sensor_rotation;
            p_record->lidar_region = LIDAR_REGION_REGION_NONE; /* Lidar assumed not detecting to start */
            break;
        default:
            /* Unknown sensor type error. */
            APP_ERROR_CHECK(true);
            break;
    }
}

/**
 * Compute which AV regions are adjacent to a sensor detection.
 */
static void find_adjacent_activity_variables(mm_node_position_t const * position, sensor_rotation_t sensor_rotation, activity_variable_set_t* av_set)
{
    /* Find sensor rotation relative to node rotation 0 */
    node_rotation_t rotation = position->node_rotation + sensor_rotation;
    rotation %= NODE_ROTATION_COUNT;
    int8_t x = position->grid_position_x;
    int8_t y = position->grid_position_y;

    /* Convert node position (-floor(MAX_GRID_SIZE/2.0) to MAX_GRID_SIZE/2) into
       'av' coordinates (0 to MAX_GRID_SIZE)) */
    uint8_t x_array = (uint8_t)(x + (MAX_GRID_SIZE_X / 2)); /* Offset centre based coordinates to left based coordinates. */
    uint8_t y_array = (uint8_t)(y + (MAX_GRID_SIZE_Y / 2)); /* Offset centre based coordinates to top based coordinates. */
    y_array = MAX_GRID_SIZE_Y - y_array -1;                 /* Positon coordinates and AV coordinates use inverted Y direction. */

    /* On any case x or y may underflow, if that happens they will be too big and won't be saved. */
    switch(rotation)
    {
        case NODE_ROTATION_0:
            /* Faces up, so can only affect row - 1, and column, column - 1 */
            y_array--;
            check_add_av(x_array, y_array, av_set);
            check_add_av(x_array - 1, y_array, av_set);
            break;
        case NODE_ROTATION_90:
            /* Faces right, so can only affect row, row - 1, and column */   
            check_add_av(x_array, y_array, av_set);
            check_add_av(x_array, y_array - 1, av_set);
            break;
        case NODE_ROTATION_180:
            /* Faces down, so can only affect row, and column, column - 1 */ 
            check_add_av(x_array, y_array, av_set);
            check_add_av(x_array - 1, y_array, av_set);
            break;
        case NODE_ROTATION_270:
            /* Faces left, so can only affect row, row - 1, column - 1 */
            x_array--;
            check_add_av(x_array, y_array, av_set);
            check_add_av(x_array, y_array - 1, av_set);
            break;
        default:
            /* Current design does not support intermediate angles. */
            APP_ERROR_CHECK(true);
            break;
    }
}

/**
 * Checks an av coordinate for validity, then adds to av_set if valid.
 */
static void check_add_av(uint8_t x, uint8_t y, activity_variable_set_t* av_set)
{
    /* Check bounds on input */
    if(x >= MAX_AV_SIZE_X)
    {
        return;
    }

    if(y >= MAX_AV_SIZE_Y)
    {
        return;
    }

    /* Save to av_set */
    APP_ERROR_CHECK(av_set->av_count >= MAX_ADJACENT_ACTIVITY_VARIABLES);
    av_set->avs[av_set->av_count] = &(AV(x, y));
    av_set->av_count++;
}