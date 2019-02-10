/**
file: mm_sensor_error_check.c
brief:
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"

#include "mm_sensor_error_check.h"
#include "mm_sensor_algorithm_config.h"
#include "mm_position_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          TYPES
**********************************************************/

typedef struct
{
    uint16_t          node_id;
    sensor_type_t     sensor_type;
    sensor_rotation_t sensor_rotation;
} sensor_record_t;

typedef struct
{
    sensor_record_t   sensor;
    uint32_t          activity_timestamps[SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE];
    uint16_t          activity_records_count;
    bool              sensor_hyperactive;
} sensor_activity_record_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Create a sensor_record_t from the given sensor_evt_t
*/
static void create_sensor_record(sensor_evt_t const * evt, sensor_record_t * rv);

/**********************************************************
                       VARIABLES
**********************************************************/

static sensor_record_t active_sensors_past_day[ MAX_SENSOR_COUNT ];
static uint16_t active_sensors_past_day_count = 0;
static sensor_activity_record_t sensor_activity_record[ MAX_SENSOR_COUNT ];

/**********************************************************
                       DECLARATIONS
**********************************************************/

void mm_sensor_error_init(void)
{
    memset(&active_sensors_past_day[0], 0, sizeof(active_sensors_past_day));
    active_sensors_past_day_count = 0;
    memset(&sensor_activity_record[0], 0, sizeof(sensor_activity_record));
}

/**
    Record that a sensor has been active (has had a detection event).
*/
void mm_sensor_error_record_sensor_activity(sensor_evt_t const * evt, uint32_t minute_count)
{
    bool first_activity_of_day = true;
    sensor_record_t activity_record;
    create_sensor_record(evt, &activity_record);
    for ( uint16_t i = 0; i < active_sensors_past_day_count; i++ )
    {
        if( activity_record.sensor_type == active_sensors_past_day[i].sensor_type && 
            activity_record.node_id == active_sensors_past_day[i].node_id && 
            activity_record.sensor_rotation == active_sensors_past_day[i].sensor_rotation )
        {
            first_activity_of_day = false;
            break;
        }
    }
    if ( first_activity_of_day )
    {
        memcpy(&active_sensors_past_day[active_sensors_past_day_count], &activity_record, sizeof( activity_record ));
        active_sensors_past_day_count++;
        APP_ERROR_CHECK_BOOL(active_sensors_past_day_count <= MAX_SENSOR_COUNT);
    }
    // check to see if this sensor has an existing activity record and if it does add to it
    for ( uint16_t i = 0; i < MAX_SENSOR_COUNT; i++ )
    {
        sensor_activity_record_t * existing_record = &sensor_activity_record[i];
        if ( activity_record.node_id == existing_record->sensor.node_id &&
             activity_record.sensor_type == existing_record->sensor.sensor_type &&
             activity_record.sensor_rotation == existing_record->sensor.sensor_rotation )
        {
            if ( existing_record->activity_records_count == SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE )
            {
                existing_record->activity_records_count = 0;
            }
            existing_record->activity_timestamps[existing_record->activity_records_count] = minute_count;
            existing_record->activity_records_count++;
            return;
        }
    }
    // create a new sensor activity record for the current sensor
    for ( uint16_t i = 0; i < MAX_SENSOR_COUNT; i++ )
    {
        sensor_activity_record_t * new_record = &sensor_activity_record[i];
        // nodeid of 0 is an invalid entry and therefore free to overwrite
        if ( new_record->sensor.node_id == 0 )
        {
            memcpy( &new_record->sensor, &activity_record, sizeof( activity_record ) );
            new_record->sensor_hyperactive = false;
            memset(&new_record->activity_timestamps[0], 0, SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE * sizeof(uint16_t));
            new_record->activity_timestamps[0] = minute_count;
            new_record->activity_records_count = 1;
        }
    }     
}

/**
    Checks to ensure all sensors have made at least 1 detection in the last 24 hours
*/
void mm_sensor_error_check_for_sensor_inactivity(void)
{
    const mm_node_position_t * node_positions = get_node_positions();
    for ( uint16_t i = 0; i < MAX_NUMBER_NODES; i++)
    {
        if ( !node_positions[i].is_valid )
        {
            continue;
        }
        sensor_rotation_t node_sensor_rotations[MAX_SENSORS_PER_NODE];
        get_sensor_rotations(node_positions[i].node_type, MAX_SENSORS_PER_NODE, node_sensor_rotations);
        bool node_sensors_active[MAX_SENSORS_PER_NODE];
        for ( uint16_t j = 0; j < active_sensors_past_day_count; j++)
        {
            if ( node_positions[i].node_id == active_sensors_past_day[j].node_id )
            {
                if ( active_sensors_past_day[j].sensor_rotation == node_sensor_rotations[0] )
                {
                    node_sensors_active[0] = true;
                }
                if ( active_sensors_past_day[j].sensor_rotation == node_sensor_rotations[1] )
                {
                    node_sensors_active[1] = true;
                }
            }
        }
        for ( uint16_t j = 0; j < MAX_SENSORS_PER_NODE; j++)
        {
            if ( !node_sensors_active[j] )
            {
                // flag sensor node_sensor_rotations[k] of node_positions[i].node_id as inactive
            }
        }
    }
    memset(&active_sensors_past_day[0], 0, sizeof( active_sensors_past_day ) );
    active_sensors_past_day_count = 0;
}

/**
    Checks for any sensors which are hyperactive. Flags hyperactive sensors so their events will be ignored.
*/
void mm_sensor_error_check_for_sensor_hyperactivity(void)
{
    for ( uint16_t i = 0; i < MAX_SENSOR_COUNT; i++ )
    {
        sensor_activity_record_t * existing_record = &sensor_activity_record[i];
        // nodeid of 0 is an invalid entry
        if ( existing_record->sensor.node_id != 0 )
        {
            uint16_t newest_timestamp = 0;
            uint16_t oldest_timestamp = UINT16_MAX;
            for ( uint16_t j = 0; j < SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE; j++ )
            {
                uint16_t current_timestamp = existing_record->activity_timestamps[j];
                // timestamp of 0 is not valid
                if ( current_timestamp == 0 )
                {
                    continue;
                }
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
    Returns true if the sensor in the given event is marked as hyperactive
*/
bool mm_sensor_error_is_sensor_hyperactive(sensor_evt_t const * evt)
{
    sensor_record_t activity_record;
    create_sensor_record(evt, &activity_record);
    for ( uint16_t i = 0; i < MAX_SENSOR_COUNT; i++)
    {
        sensor_activity_record_t * existing_record = &sensor_activity_record[i];
        if ( activity_record.node_id == existing_record->sensor.node_id &&
             activity_record.sensor_rotation == existing_record->sensor.sensor_rotation)
        {
            return existing_record->sensor_hyperactive;
        }
    }
    return false;
}

/**
    Called before clearing node position changed flag.
*/
void mm_sensor_error_on_node_positions_update(void)
{
    if(have_node_positions_changed())
    {
        /* make sure to only keep sensor records for sensors that still exist */
        const mm_node_position_t * node_positions = get_node_positions();
        for ( uint16_t i = 0; i < MAX_SENSOR_COUNT; i++ )
        {
            sensor_activity_record_t * existing_record = &sensor_activity_record[i];
            bool sensor_valid = false;
            for ( uint16_t j = 0; j < MAX_NUMBER_NODES; j++ )
            {
                const mm_node_position_t * current_node = &node_positions[j];
                sensor_rotation_t node_sensor_rotations[MAX_SENSORS_PER_NODE];
                get_sensor_rotations(current_node->node_type, MAX_SENSORS_PER_NODE, node_sensor_rotations);
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
}

/**
    Create a sensor_record_t from the given sensor_evt_t
*/
static void create_sensor_record(sensor_evt_t const * evt, sensor_record_t * rv)
{
    switch(evt->sensor_type)
    {
        case SENSOR_TYPE_PIR:
            rv->node_id = evt->pir_data.node_id;
            rv->sensor_type = evt->sensor_type;
            rv->sensor_rotation = evt->pir_data.sensor_rotation;
            break;
        case SENSOR_TYPE_LIDAR:
            rv->node_id = evt->lidar_data.node_id;
            rv->sensor_type = evt->sensor_type;
            rv->sensor_rotation = evt->lidar_data.sensor_rotation;
            break;
        default:
            /* App error, unknown sensor data type. */
            APP_ERROR_CHECK(true);
            rv->node_id = 0;
            rv->sensor_type = SENSOR_TYPE_UNKNOWN;
            rv->sensor_rotation = SENSOR_ROTATION_0;
            break;
    }
}
