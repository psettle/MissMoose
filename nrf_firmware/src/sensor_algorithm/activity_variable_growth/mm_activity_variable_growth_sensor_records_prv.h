/**
file: mm_activity_variable_sensor_records_prv.h
brief:
notes:
*/
#ifndef MM_ACTIVITY_VARIABLE_GROWTH_SENSOR_RECORDS_PRV_H
#define MM_ACTIVITY_VARIABLE_GROWTH_SENSOR_RECORDS_PRV_H

/**********************************************************
                          INCLUDES
**********************************************************/

#include "mm_sensor_transmission.h"
#include "mm_sensor_algorithm_config.h"

/**********************************************************
                          TYPES
**********************************************************/

typedef struct
{
    uint16_t          node_id;
    sensor_type_t     sensor_type;
    sensor_rotation_t sensor_rotation;
    uint8_t           detection_status; /* User defined status byte, 0 default. */
    bool is_valid;
} sensor_record_t;

typedef struct
{
    sensor_type_t sensor_type;
    uint16_t      next_id;  
} sensor_record_iterator_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Initialize all sensor records to default values.
 */
void init_sensor_records(mm_sensor_algorithm_config_t const * config);

/**
 * Fetch a detection record for a sensor, create one if needed.
 */ 
sensor_record_t* get_sensor_record(uint16_t node_id, sensor_rotation_t sensor_rotation, sensor_type_t sensor_type);

/**
 * Iterate through sensor records for specific sensor types.
 * 
 * sensor_record_iterator_t it = { SENSOR_TYPE_XXX, 0 };
 * sensor_record_t* item = next_sensor_record(&it);
 * 
 * Returns NULL after list has been exhausted.
 */ 
sensor_record_t* next_sensor_record(sensor_record_iterator_t* iterator);

#endif /* MM_ACTIVITY_VARIABLE_GROWTH_SENSOR_RECORDS_PRV_H */