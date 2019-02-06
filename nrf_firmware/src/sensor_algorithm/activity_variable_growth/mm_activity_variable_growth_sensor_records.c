
/**
file: mm_activity_variable_growth_sensor_records.c
brief:
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include "string.h"

#include "app_error.h"

#include "mm_activity_variable_growth_sensor_records_prv.h"
#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        MACROS
**********************************************************/

/**********************************************************
                          TYPES
**********************************************************/

/**********************************************************
                       VARIABLES
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

static sensor_record_t      sensor_records[MAX_SENSOR_COUNT];

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * Initialize all sensor records to default values.
 */
void init_sensor_records(void)
{
    memset(&(sensor_records[0]), 0, sizeof(sensor_records));
}

/**
 * Fetch a detection record for a sensor, create one if needed.
 */ 
sensor_record_t* get_sensor_record(uint16_t node_id, sensor_rotation_t sensor_rotation, sensor_type_t sensor_type)
{  
    sensor_record_t * empty = NULL; 
    for(uint16_t i = 0; i < MAX_SENSOR_COUNT; ++i)
    {
        sensor_record_t * record = &(sensor_records[i]);

        if(!record->is_valid)
        {
            /* Grab an empty slot while we're here */
            if(NULL == empty)
            {
                empty = record;
            }

            continue;
        }

        if(record->node_id != node_id)
        {
            continue;
        }

        if(record->sensor_rotation != sensor_rotation)
        {
            continue;
        }

        if(record->sensor_type != sensor_type)
        {
            continue;
        }

        /* Existing record */
        return record;
    }

    /* No existing record, should have grabbed a slot while looking */
    APP_ERROR_CHECK(!empty);

    /* Return a new record. */
    memset(empty, 0, sizeof(sensor_record_t));
    empty->node_id = node_id;
    empty->sensor_rotation = sensor_rotation;
    empty->sensor_type = sensor_type;
    empty->detection_status = 0; /*Default. */
    empty->is_valid = true;
    return empty;
}

sensor_record_t* next_sensor_record(sensor_record_iterator_t* iterator)
{
    for(uint16_t i = iterator->next_id; i < MAX_SENSOR_COUNT; ++i)
    {
        if(sensor_records[i].sensor_type == iterator->sensor_type &&
           sensor_records[i].is_valid)
        {
            iterator->next_id = i + 1;
            return &sensor_records[i];
        }
    }

    iterator->next_id = UINT16_MAX;
    return NULL;
}