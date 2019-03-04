
/**
file: mm_activity_variable_growth_lidar.c
brief:
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"

#include "mm_sensor_algorithm_config.h"
#include "mm_activity_variable_growth_lidar_prv.h"
#include "mm_activity_variable_growth_prv.h"
#include "mm_activity_variable_growth_sensor_records_prv.h"
#include "mm_position_config.h"
#include "mm_monitoring_dispatch.h"
#include "mm_sensor_error_check.h"

/**********************************************************
                        MACROS
**********************************************************/

/**********************************************************
                          TYPES
**********************************************************/

typedef struct
{
    lidar_region_t   region;        /* Region as originally measured. */
    int8_t           xpos;          /* Effective detection position. */
    int8_t           ypos;          /* Effective detection position.*/
    total_rotation_t direction;     /* Effective detection direction. */
} abstract_lidar_detection_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Calulate which AV region a lidar detection occured in.
 * 
 * [out] which region, a position to consider the detection from, and a total direction of the detection
 * 
 * return false if evt was invalid.
 */
static bool sensor_evt_to_lidar_detection(lidar_evt_data_t const * evt, abstract_lidar_detection_t* detection);

/**
 * Process a lidar record, and send an abstract detection event out if the record is active.
 */
static void lidar_on_second_evt(sensor_record_t const * record);

/**
 * Fetch a virtual location for a lidar detection using the detection region and the original sensor position.
 */
static void sensor_record_to_lidar_detection(sensor_record_t const * record, abstract_lidar_detection_t* detection);

/**
 * Grabs the position for a node in the grid. Creates a 'mock' default position if there isn't a node there.
 * 
 * [out] A mm_node_position_t object describing a node at x,y. That node may or may not actually exist.
 */
static void fetch_or_mock_node_position(int8_t x, int8_t y, mm_node_position_t* position_out);

/**
 * Collect the constants that define how AVs grow in reponse to lidar events.
 */
static void generate_growth_constants(abstract_lidar_detection_t const * detection, activity_variable_sensor_constants_t* constants);

/**
 * Collect the constants that define how AVs grow in reponse to second events while a lidar is detecting.
 */
static void generate_trickle_constants(abstract_lidar_detection_t const * detection, activity_variable_sensor_constants_t* constants);

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * Translates a lidar detection event into an abstract detection event.
 */
void translate_lidar_detection(sensor_evt_t const * sensor_evt)
{
    lidar_evt_data_t const * evt = &sensor_evt->lidar_data;

    /* Translate into an abstract detection: */
    abstract_lidar_detection_t detection;
    if(!sensor_evt_to_lidar_detection(evt, &detection))
    {
        /* Invalid event. */
        return;
    }

    /* Send monitoring dispatch */
    mm_monitoring_dispatch_send_lidar_data
        (
        evt->node_id,
        evt->sensor_rotation,
        evt->distance_measured,
        detection.region
        );

    /* Is the sensor hyperactive? */
    if(mm_sensor_error_is_sensor_hyperactive(sensor_evt))
    {
        /* If so, don't process further. */
        return;
    }

    /* Fetch the sensor record to compare against. */
    sensor_record_t* record = get_sensor_record(evt->node_id, evt->sensor_rotation, SENSOR_TYPE_LIDAR);

    if(record->detection_status == detection.region)
    {
        /* It isn't a new detection because it's the same data, no need to apply it. */
        return;
    }

    if(detection.region == LIDAR_REGION_REGION_NONE)
    {
        /* End of detection, modify record and stop processing it. */
        record->detection_status = LIDAR_REGION_REGION_NONE;
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
    record->detection_status = detection.region;
}

/**
 * Translates an on second event into 0 or more abstract detection events.
 */
void translate_on_second_evt_lidar(void)
{
    /* Iterate through lidar records. */
    sensor_record_iterator_t it =
    {
        .sensor_type = SENSOR_TYPE_LIDAR,
        .next_id = 0
    };

    for(sensor_record_t* record = next_sensor_record(&it); record; record = next_sensor_record(&it))
    {
        lidar_on_second_evt(record);
    }
}

static void lidar_on_second_evt(sensor_record_t const * record)
{
    /* Translate into an abstract detection: */
    abstract_lidar_detection_t detection;
    sensor_record_to_lidar_detection(record, &detection);

    if(detection.region == LIDAR_REGION_REGION_NONE)
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
 * Calculate which region a lidar detection belongs to
 */ 
static bool sensor_evt_to_lidar_detection(lidar_evt_data_t const * evt, abstract_lidar_detection_t* detection)
{
    memset(detection, 0, sizeof(abstract_lidar_detection_t));
    /* The regions can shift around a lot due to the offset system, so we need 2 values:
            - distance to first node
            - distance to second node
        keep in mind that either of those nodes may not exist, so we will populate a 'default' position for them in that case */
    mm_node_position_t const * position = get_position_for_node(evt->node_id);


    if(position == NULL)
    {
        /* Unknown position, therefore invalid event. */
        return false;
    }

    /* Figure out what direction the lidar is pointing in. */
    total_rotation_t rotation = (evt->sensor_rotation + position->node_rotation) % TOTAL_ROTATION_360;
    int8_t dx = 0;
    int8_t dy = 0;
    get_grid_direction(rotation, &dx, &dy);

    /* Fetch positions for 1 forward and 2 forward, replace with local mockups where nodes don't exist. */
    mm_node_position_t position_1_forward;
    mm_node_position_t position_2_forward;
    int8_t x = position->grid_position_x;
    int8_t y = position->grid_position_y;
    fetch_or_mock_node_position(x + 1 * dx, y + 1 * dy, &position_1_forward);
    fetch_or_mock_node_position(x + 2 * dx, y + 2 * dy, &position_2_forward);

    /* Fetch relevant offsets for distance calculation: */
    int8_t node_0_forward_offset = 0;
    int8_t node_1_forward_offset = 0;
    int8_t node_2_forward_offset = 0;

    node_0_forward_offset += dx * position->grid_offset_x;
    node_1_forward_offset += dx * position_1_forward.grid_offset_x;
    node_2_forward_offset += dx * position_2_forward.grid_offset_x;
    node_0_forward_offset += dy * position->grid_offset_y;
    node_1_forward_offset += dy * position_1_forward.grid_offset_y;
    node_2_forward_offset += dy * position_2_forward.grid_offset_y;

    uint16_t distance_to_1_node_cm = NODE_SEPERATION_CM + (node_1_forward_offset - node_0_forward_offset) * NODE_OFFSET_SCALE_CM;
    uint16_t distance_to_2_node_cm = 2 * NODE_SEPERATION_CM + (node_2_forward_offset - node_0_forward_offset) * NODE_OFFSET_SCALE_CM;

    /* Now we know distances to both regions... is the distance within it? */
    uint16_t distance_measured = evt->distance_measured;

    if(distance_measured < distance_to_1_node_cm)
    {
        detection->region = LIDAR_REGION_REGION_0;
        detection->xpos = x;
        detection->ypos = y;
        detection->direction = rotation;
    }
    else if(distance_measured < distance_to_2_node_cm)
    {
        detection->region = LIDAR_REGION_REGION_1;
        detection->xpos = x + dx;
        detection->ypos = y + dy;
        detection->direction = rotation;
    }
    else
    {
        detection->region = LIDAR_REGION_REGION_NONE;    
    }

    return true;
}

/**
 * Fetch a virtual location for a lidar detection using the detection region and the original sensor position.
 */
static void sensor_record_to_lidar_detection(sensor_record_t const * record, abstract_lidar_detection_t* detection)
{
    memset(detection, 0, sizeof(abstract_lidar_detection_t));

    detection->region = record->detection_status;

    if(record->detection_status == LIDAR_REGION_REGION_NONE)
    {
        /* No detection, so no need to know where it came from. */
        return;
    }

    mm_node_position_t const * position = get_position_for_node(record->node_id);
    total_rotation_t rotation = (record->sensor_rotation + position->node_rotation) % TOTAL_ROTATION_360;
    detection->direction = rotation;

    int8_t x = position->grid_position_x;
    int8_t y = position->grid_position_y;

    if(record->detection_status == LIDAR_REGION_REGION_0)
    {
        /* If first region, it came from the original position. */
        detection->xpos = x;
        detection->ypos = y;   
        return;
    }

    /* Now we need to mock out a position in the direction of the sensor: */
    int8_t dx = 0;
    int8_t dy = 0;
    get_grid_direction(rotation, &dx, &dy);

    detection->xpos = x + dx;
    detection->ypos = y + dy;   
}

static void fetch_or_mock_node_position(int8_t x, int8_t y, mm_node_position_t* position_out)
{
    mm_node_position_t const * p_position = get_node_for_position(x, y);

    if(p_position == NULL)
    {
        memset(position_out, 0, sizeof(mm_node_position_t));
        position_out->grid_position_x = x;
        position_out->grid_position_y = y;
    }
    else
    {
        memcpy(position_out, p_position, sizeof(mm_node_position_t));
    }
}

static void generate_growth_constants(abstract_lidar_detection_t const * detection, activity_variable_sensor_constants_t* constants)
{
    memset(constants, 0, sizeof(activity_variable_sensor_constants_t));

    constants->common_sensor_weight_factor = mm_sensor_algorithm_config()->common_sensor_weight_factor;
    constants->base_sensor_weight_factor   = mm_sensor_algorithm_config()->base_sensor_weight_factor_lidar;

    switch(detection->ypos)
    {
        case 1:
            constants->road_proximity_factor = mm_sensor_algorithm_config()->road_proximity_factor_0;
            break;
        case 0:
            constants->road_proximity_factor = mm_sensor_algorithm_config()->road_proximity_factor_1;
            break;
        case -1:
            constants->road_proximity_factor = mm_sensor_algorithm_config()->road_proximity_factor_2;
            break;
        default:
            /* Invalid grid position given grid height */
            APP_ERROR_CHECK(true);
            break;
    }
}

static void generate_trickle_constants(abstract_lidar_detection_t const * detection, activity_variable_sensor_constants_t* constants)
{
    memset(constants, 0, sizeof(activity_variable_sensor_constants_t));

    constants->common_sensor_weight_factor = mm_sensor_algorithm_config()->common_sensor_trickle_factor;
    constants->base_sensor_weight_factor   = mm_sensor_algorithm_config()->base_sensor_trickle_factor_lidar;

    switch(detection->ypos)
    {
        case 1:
            constants->road_proximity_factor = mm_sensor_algorithm_config()->road_trickle_proximity_factor_0;
            break;
        case 0:
            constants->road_proximity_factor = mm_sensor_algorithm_config()->road_trickle_proximity_factor_1;
            break;
        case -1:
            constants->road_proximity_factor = mm_sensor_algorithm_config()->road_trickle_proximity_factor_2;
            break;
        default:
            /* Invalid grid position given grid height */
            APP_ERROR_CHECK(true);
            break;
    }
}
