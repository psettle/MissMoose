
/**
file: mm_activity_variable_growth_pir.c
brief:
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"

#include "mm_sensor_algorithm_config.h"
#include "mm_activity_variable_growth_pir_prv.h"
#include "mm_activity_variable_growth_prv.h"
#include "mm_activity_variable_growth_sensor_records_prv.h"
#include "mm_position_config.h"

/**********************************************************
                        MACROS
**********************************************************/

/**********************************************************
                          TYPES
**********************************************************/

typedef struct
{
    bool             detected;      /* Is the detction active. */
    int8_t           xpos;          /* Effective detection position. */
    int8_t           ypos;          /* Effective detection position.*/
    total_rotation_t direction;     /* Effective detection direction. */
} abstract_pir_detection_t;

/**********************************************************
                       VARIABLES
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Calulate which AV region a pir detection occured in.
 * 
 * [out] which region, a position to consider the detection from, and a total direction of the detection
 * 
 * return false if evt was invalid.
 */
static bool sensor_evt_to_pir_detection(pir_evt_data_t const * evt, abstract_pir_detection_t* detection);

/**
 * Process a pir record, and send an abstract detection event out if the record is active.
 */
static void pir_on_second_evt(sensor_record_t const * record);

/**
 * Fetch a virtual location for a pir detection using the detection region and the original sensor position.
 */
static void sensor_record_to_pir_detection(sensor_record_t const * record, abstract_pir_detection_t* detection);

/**
 * Collect the constants that define how AVs grow in reponse to pir events.
 */
static void generate_growth_constants(abstract_pir_detection_t const * detection, activity_variable_sensor_constants_t* constants);

/**
 * Collect the constants that define how AVs grow in reponse to second events while a pir is detecting.
 */
static void generate_trickle_constants(abstract_pir_detection_t const * detection, activity_variable_sensor_constants_t* constants);

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * Translates a pir detection event into an abstract detection event.
 */
void translate_pir_detection(pir_evt_data_t const * evt)
{
    /* Translate into an abstract detection: */
    abstract_pir_detection_t detection;
    if(!sensor_evt_to_pir_detection(evt, &detection))
    {
        /* Invalid event. */
        return;
    }

    /* Fetch the sensor record to compare against. */
    sensor_record_t* record = get_sensor_record(evt->node_id, evt->sensor_rotation, SENSOR_TYPE_PIR);

    if(record->detection_status == detection.detected)
    {
        /* It isn't a new detection because it's the same data, no need to apply it. */
        return;
    }

    if(!detection.detected)
    {
        /* End of detection, modify record and stop processing it. */
        record->detection_status = false;
        return;
    }

    /* By now it is a new detection, generate an abstract event to grow the activity variables. */
    abstract_sensor_detection_t abstract_detection;
    memset(&abstract_detection, 0, sizeof(abstract_detection));
    abstract_detection.xpos = detection.xpos;
    abstract_detection.ypos = detection.ypos;
    abstract_detection.rotation = detection.direction;
    generate_growth_constants(&detection, &abstract_detection.constants);

    process_abstract_detection(&abstract_detection);

    /* Update detection state. */
    record->detection_status = true;
}

/**
 * Translates an on second event into 0 or more abstract detection events.
 */
void translate_on_second_evt_pir(void)
{
    /* Iterate through pir records. */
    sensor_record_iterator_t it =
    {
        .sensor_type = SENSOR_TYPE_PIR,
        .next_id = 0
    };

    for(sensor_record_t* record = next_sensor_record(&it); record; record = next_sensor_record(&it))
    {
        pir_on_second_evt(record);
    }
}

static void pir_on_second_evt(sensor_record_t const * record)
{
    /* Translate into an abstract detection: */
    abstract_pir_detection_t detection;
    sensor_record_to_pir_detection(record, &detection);

    if(!detection.detected)
    {
        /* Not detecting, don't send event. */
        return;
    }

    /* Still detecting, send a trickle event. */
    abstract_sensor_detection_t abstract_detection;
    memset(&abstract_detection, 0, sizeof(abstract_detection));
    abstract_detection.xpos = detection.xpos;
    abstract_detection.ypos = detection.ypos;
    abstract_detection.rotation = detection.direction;
    generate_trickle_constants(&detection, &abstract_detection.constants);

    process_abstract_detection(&abstract_detection);
}

/**
 * Translate sensor event to an abstract pir detection event.
 */ 
static bool sensor_evt_to_pir_detection(pir_evt_data_t const * evt, abstract_pir_detection_t* detection)
{
    memset(detection, 0, sizeof(abstract_pir_detection_t));

    mm_node_position_t const * position = get_position_for_node(evt->node_id);
    
    if(position == NULL)
    {
        /* We don't know where this event came from */
        return false;
    }

    /* Figure out what direction the pir is pointing in. */
    total_rotation_t rotation = (evt->sensor_rotation + position->node_rotation) % TOTAL_ROTATION_360;

    /* Populate detection */
    detection->xpos = position->grid_position_x;
    detection->ypos = position->grid_position_y;
    detection->direction = rotation;
    detection->detected = evt->detection;

    return true;
}

/**
 * Translate sensor record to an abstract pir detection event.
 */
static void sensor_record_to_pir_detection(sensor_record_t const * record, abstract_pir_detection_t* detection)
{
    memset(detection, 0, sizeof(abstract_pir_detection_t));

    if(!record->detection_status)
    {
        /* No detection, so no need to know where it came from. */
        return;
    }

    /* Figure out where the record is from. */
    mm_node_position_t const * position = get_position_for_node(record->node_id);

    if(position == NULL)
    {
        /* We don't know where this event came from, but we should because it's detecting.*/
        APP_ERROR_CHECK(true);
        return;
    }

    total_rotation_t rotation = (record->sensor_rotation + position->node_rotation) % TOTAL_ROTATION_360;

    /* Populate detection */
    detection->xpos = position->grid_position_x;
    detection->ypos = position->grid_position_y;
    detection->direction = rotation;
    detection->detected = true;
}

static void generate_growth_constants(abstract_pir_detection_t const * detection, activity_variable_sensor_constants_t* constants)
{
    memset(constants, 0, sizeof(activity_variable_sensor_constants_t));

    constants->common_sensor_weight_factor = COMMON_SENSOR_WEIGHT_FACTOR;
    constants->base_sensor_weight_factor   = BASE_SENSOR_WEIGHT_FACTOR_PIR;

    switch(detection->ypos)
    {
        case 1:
            constants->road_proximity_factor = ROAD_PROXIMITY_FACTOR_0;
            break;
        case 0:
            constants->road_proximity_factor = ROAD_PROXIMITY_FACTOR_1;
            break;
        case -1:
            constants->road_proximity_factor = ROAD_PROXIMITY_FACTOR_2;
            break;
        default:
            /* Invalid grid position given grid height */
            APP_ERROR_CHECK(true);
            break;
    }
}

static void generate_trickle_constants(abstract_pir_detection_t const * detection, activity_variable_sensor_constants_t* constants)
{
    memset(constants, 0, sizeof(activity_variable_sensor_constants_t));

    constants->common_sensor_weight_factor = COMMON_SENSOR_TRICKLE_FACTOR;
    constants->base_sensor_weight_factor   = BASE_SENSOR_TRICKLE_FACTOR_PIR;

    switch(detection->ypos)
    {
        case 1:
            constants->road_proximity_factor = ROAD_TRICKLE_PROXIMITY_FACTOR_0;
            break;
        case 0:
            constants->road_proximity_factor = ROAD_TRICKLE_PROXIMITY_FACTOR_1;
            break;
        case -1:
            constants->road_proximity_factor = ROAD_TRICKLE_PROXIMITY_FACTOR_2;
            break;
        default:
            /* Invalid grid position given grid height */
            APP_ERROR_CHECK(true);
            break;
    }
}