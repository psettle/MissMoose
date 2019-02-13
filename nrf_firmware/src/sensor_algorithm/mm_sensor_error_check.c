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
    sensor_rotation_t sensor_rotation;
} sensor_def_t;

typedef struct
{
    sensor_def_t      sensor;
    uint32_t          t_last_detection;
    bool              is_inactive;
    bool              is_valid;
} sensor_inactivity_record_t;

typedef struct
{
    sensor_def_t      sensor;
    uint32_t          activity_timestamps[SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE];
    uint16_t          activity_timestamp_index;
    bool              sensor_hyperactive;
    bool              is_valid;
} sensor_hyperactivity_record_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Create a sensor_def_t from the given sensor_evt_t
*/
static void create_sensor_record(sensor_evt_t const * evt, sensor_def_t * sensor);

/**
    Process a sensor event for possible inactivity.
*/
static void on_sensor_evt_inactive_check(sensor_evt_t const * evt, uint32_t minute_count);

/**
    Create inactive sensor records for the provided position where they do not exist.
*/
static void create_inactive_sensor_records(mm_node_position_t const * position, uint32_t minute_count);

/**
    Create inactive sensor record for the provided sensor where it does not exist.
 */
static void create_inactive_sensor_record(sensor_def_t const * sensor, uint32_t minute_count);

/**
    Process a sensor event for possible hyperactivity.
 */
static void on_sensor_evt_hyperactive_check(sensor_evt_t const * evt, uint32_t minute_count);

/**
    Fetches or creates a sensor hyperactivity record.
 */
static sensor_hyperactivity_record_t* get_sensor_hyperactivity_record(sensor_evt_t const * evt);

/**
    Check for and flag inactive sensors.
 */
static void evaluate_sensor_inactivity(uint32_t minute_count);

/**
    Check for and flag hyperactive sensors.
 */
static void evaluate_sensor_hyperactivity(void);

/**
    Check for and flag a specific hyperactive sensor.
 */
static void evaluate_sensor_hyperactivity_record(sensor_hyperactivity_record_t * record);

/**********************************************************
                       VARIABLES
**********************************************************/

static sensor_inactivity_record_t       sensor_inactivity_records[ MAX_SENSOR_COUNT ];
static sensor_hyperactivity_record_t    sensor_hyperactivity_records[MAX_SENSOR_COUNT];

/**********************************************************
                       DECLARATIONS
**********************************************************/

void mm_sensor_error_init(void)
{
    memset(&sensor_inactivity_records[0], 0, sizeof(sensor_inactivity_records));
    memset(&sensor_hyperactivity_records[0], 0, sizeof(sensor_hyperactivity_records));
}

/**
    Record that a sensor has been active (has had a detection event).
*/
void mm_sensor_error_record_sensor_activity(sensor_evt_t const * evt, uint32_t minute_count)
{
    on_sensor_evt_inactive_check(evt, minute_count);
    on_sensor_evt_hyperactive_check(evt, minute_count);
}

void mm_sensor_error_on_minute_elapsed(uint32_t minute_count)
{
    evaluate_sensor_inactivity(minute_count);
    evaluate_sensor_hyperactivity();
}

/**
    Returns true if the sensor in the given event is marked as hyperactive.
*/
bool mm_sensor_error_is_sensor_hyperactive(sensor_evt_t const * evt)
{
    sensor_hyperactivity_record_t const * record = get_sensor_hyperactivity_record(evt);

    /* Records should always exist. */
    APP_ERROR_CHECK(record == NULL);

    return record->sensor_hyperactive;
}

/**
    Called before clearing node position changed flag.
*/
void mm_sensor_error_on_node_positions_update(uint32_t minute_count)
{
    /* The node positions have changed, which means
       there are potentially new sensors to track inactivity
       for. Go through each node position and fetch an inactivity record for it. */

    for (uint16_t i = 0; i < MAX_NUMBER_NODES; i++)
    {
        mm_node_position_t const * position = &(get_node_positions()[i]);

        /* Emplace sensor records for each one. */
        create_inactive_sensor_records(position, minute_count);
    }
}

/**
    Create a sensor_record_t from the given sensor_evt_t
*/
static void create_sensor_record(sensor_evt_t const * evt, sensor_def_t * sensor)
{
    memset(sensor, 0, sizeof(sensor_def_t));

    switch(evt->sensor_type)
    {
        case SENSOR_TYPE_PIR:
            sensor->node_id = evt->pir_data.node_id;
            sensor->sensor_rotation = evt->pir_data.sensor_rotation;
            break;
        case SENSOR_TYPE_LIDAR:
            sensor->node_id = evt->lidar_data.node_id;
            sensor->sensor_rotation = evt->lidar_data.sensor_rotation;
            break;
        default:
            /* App error, unknown sensor data type. */
            APP_ERROR_CHECK(true);
            break;
    }
}

/**
    Process a sensor event for possible inactivity.
*/
static void on_sensor_evt_inactive_check(sensor_evt_t const * evt, uint32_t minute_count)
{
    /* Fetch a template to search for a sensor with. */
    sensor_def_t sensor;
    create_sensor_record(evt, &sensor);

    /* Find the inactive record for that sensor, mark the minutes since last detection. */
    for (uint16_t i = 0; i < MAX_SENSOR_COUNT; i++)
    {
        sensor_inactivity_record_t* record = &sensor_inactivity_records[i];

        if (sensor.node_id == record->sensor.node_id &&
            sensor.sensor_rotation == record->sensor.sensor_rotation &&
            record->is_valid)
        {
            record->t_last_detection = minute_count;
            record->is_inactive = false;
            return;
        }
    }

    /* If we got here, there was no record for that sensor, but there should be a record for all sensors */
    APP_ERROR_CHECK(true);
}

/**
    Create inactive sensor records for the provided position where they do not exist.
*/
static void create_inactive_sensor_records(mm_node_position_t const * position, uint32_t minute_count)
{
    /* Which sensors does this node have? */
    sensor_rotation_t node_sensor_rotations[MAX_SENSORS_PER_NODE];
    get_sensor_rotations(position->node_type, MAX_SENSORS_PER_NODE, node_sensor_rotations);

    /* Create a record for each one. */
    for (uint16_t i = 0; i < MAX_SENSORS_PER_NODE; ++i)
    {
        sensor_def_t sensor;
        memset(&sensor, 0, sizeof(sensor));
        sensor.node_id = position->node_id;
        sensor.sensor_rotation = node_sensor_rotations[i];

        create_inactive_sensor_record(&sensor, minute_count);
    }
}

/**
    Create inactive sensor record for the provided sensor where it does not exist.
 */
static void create_inactive_sensor_record(sensor_def_t const * sensor, uint32_t minute_count)
{
    /* First check if the record exists already. */
    sensor_inactivity_record_t* empty_record = NULL;
    for (uint16_t i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        sensor_inactivity_record_t const * record = &sensor_inactivity_records[i];

        if (sensor->node_id == record->sensor.node_id &&
            sensor->sensor_rotation == record->sensor.sensor_rotation &&
            record->is_valid)
        {
            /* The record already exists. */
            return;
        }

        /* Grab an empty record if one does not exist. */
        if (!record->is_valid &&
            empty_record == NULL)
        {
            empty_record = &sensor_inactivity_records[i];
            break;
        }
    }

    memset(empty_record, 0, sizeof(sensor_inactivity_record_t));
    memcpy(&empty_record->sensor, sensor, sizeof(sensor_def_t));
    empty_record->is_valid = true;
    empty_record->is_inactive = false;
    empty_record->t_last_detection = minute_count; /* Save the current timestamp so it is not immedietly hyperactive. */
}

/**
    Process a sensor event for possible hyperactivity.
 */
static void on_sensor_evt_hyperactive_check(sensor_evt_t const * evt, uint32_t minute_count)
{
    /* Fetch the record to write to. */
    sensor_hyperactivity_record_t* record = get_sensor_hyperactivity_record(evt);

    /* No existing record, something is wrong. */
    APP_ERROR_CHECK(record == NULL);

    /* Emplace the detection timestamp. */
    record->activity_timestamps[record->activity_timestamp_index] = minute_count;

    /* Increment queue index */
    record->activity_timestamp_index++;
    record->activity_timestamp_index %= SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE;
}

/**
    Fetches or creates a sensor hyperactivity record.
 */
static sensor_hyperactivity_record_t* get_sensor_hyperactivity_record(sensor_evt_t const * evt)
{
    /* Fetch a template to search for a sensor with. */
    sensor_def_t sensor;
    create_sensor_record(evt, &sensor);

    /* check to see if this sensor has an existing hyperactivity record and if it does add to it */
    for (uint16_t i = 0; i < MAX_SENSOR_COUNT; i++)
    {
        sensor_hyperactivity_record_t* existing_record = &sensor_hyperactivity_records[i];

        /* Does this record match the event? */
        if (sensor.node_id != existing_record->sensor.node_id ||
            sensor.sensor_rotation != existing_record->sensor.sensor_rotation ||
            !existing_record->is_valid)
        {
            /* No it doesn't */
            continue;
        }

        return existing_record;
    }

    for (uint16_t i = 0; i < MAX_SENSOR_COUNT; i++)
    {
        sensor_hyperactivity_record_t* new_record = &sensor_hyperactivity_records[i];

        /* Skip valid records. */
        if (new_record->is_valid)
        {
            continue;
        }

        /* Initialize the new record. */
        memset(new_record, 0, sizeof(sensor_hyperactivity_record_t));
        memcpy(&new_record->sensor, &sensor, sizeof(sensor_def_t));
        new_record->sensor_hyperactive = false;
        new_record->is_valid = true;

        return new_record;
    }

    /* Couldn't find or allocate record. */
    APP_ERROR_CHECK(true);
    return NULL;
}

/**
    Check for and flag inactive sensors.
 */
static void evaluate_sensor_inactivity(uint32_t minute_count)
{
    /* Check for and flag any sensors where the last detection is too old. */
    for (uint16_t i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        sensor_inactivity_record_t* record = &sensor_inactivity_records[i];

        if (!record->is_valid)
        {
            /* Invalid record. */
            continue;
        }

        if (record->is_inactive)
        {
            /* Still inactive. */
            continue;
        }

        uint32_t dt = minute_count - record->t_last_detection;

        if (dt >= SENSOR_INACTIVITY_THRESHOLD_MIN)
        {
            record->is_inactive = true;
        }
    }
}

/**
    Check for and flag hyperactive sensors.
 */
static void evaluate_sensor_hyperactivity(void)
{
    for (uint16_t i = 0; i < MAX_SENSOR_COUNT; i++)
    {
        sensor_hyperactivity_record_t * record = &sensor_hyperactivity_records[i];

        if (!record->is_valid)
        {
            /* Invalid record, skip it. */
            continue;
        }

        /* Check if it is hyperactive. */
        evaluate_sensor_hyperactivity_record(record);
    }
}

/**
    Check for and flag a specific hyperactive sensor.
 */
static void evaluate_sensor_hyperactivity_record(sensor_hyperactivity_record_t * record)
{
    uint32_t tmax = 0;
    uint32_t tmin = UINT32_MAX;

    /* Calculate the duration the events were collected over. */
    for (uint16_t i = 0; i < SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE; i++)
    {
        uint32_t t = record->activity_timestamps[i];

        if (t > tmax)
        {
            tmax = t;
        }
        if (t < tmin)
        {
            tmin = t;
        }
    }

    if (tmin == 0)
    {
        /* The detection buffer is not full, so the frequency cannot be too high. */
        record->sensor_hyperactive = false;
        return;
    }

    if (tmin == tmax)
    {
        /* Detections all occured within the same minute, so the sensor is definitely hyperactive. */
        record->sensor_hyperactive = true;
        return;
    }

    /* Edge condition checks passed, so the buffer is full and the events occured over more than a minute. */

    float dt = (float)(tmax - tmin);

    float detection_frequency = SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE / dt; /* In detection/minute */

    if (detection_frequency >= SENSOR_HYPERACTIVITY_FREQUENCY_THRES)
    {
        record->sensor_hyperactive = true;
    }
    else
    {
        record->sensor_hyperactive = false;
    }
}