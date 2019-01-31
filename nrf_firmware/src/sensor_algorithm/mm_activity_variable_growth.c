
/**
file: mm_activity_variable_growth.c
brief:
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"

#include "mm_activity_variable_growth.h"
#include "mm_activity_variables.h"
#include "mm_sensor_algorithm_config.h"
#include "mm_position_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_ADJACENT_ACTIVITY_VARIABLES ( 2 )   /* The max number of regions a sensor can affect per detection. */   
#define NODE_SEPERATION_CM              ( 800 )
#define NODE_OFFSET_SCALE_CM            ( 5 )

/**********************************************************
                        MACROS
**********************************************************/

/**********************************************************
                          TYPES
**********************************************************/

typedef struct
{
    mm_activity_variable_t* avs[MAX_ADJACENT_ACTIVITY_VARIABLES];
    uint8_t                 av_count;
} activity_variable_set_t;

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
} sensor_record_t;

typedef struct
{
    float common_sensor_weight_factor;
    float base_sensor_weight_factor;
    float road_proximity_factor;
} activity_variable_sensor_constants_t;

/**********************************************************
                       VARIABLES
**********************************************************/

static mm_node_position_t   node_positions[MAX_NUMBER_NODES];
static sensor_record_t      sensor_records[MAX_SENSOR_COUNT];

/**********************************************************
                       DECLARATIONS
**********************************************************/

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
static void apply_activity_variable_trickle_to_record(sensor_record_t const * record);

/**
 * Apply growth given avs and constants.
 */ 
static void grow_activity_variables(activity_variable_set_t* av_set, activity_variable_sensor_constants_t const * constants);

/**
 * Fetch a detection record for a sensor, create one if needed.
 */ 
static sensor_record_t* get_sensor_record(sensor_evt_t const * evt);

/**
 * Set a new sensor detection record to initial values.
 */ 
static void init_sensor_record(sensor_evt_t const * evt, sensor_record_t* p_record);

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
static void get_effective_location_for_lidar_detection(mm_node_position_t const * position, sensor_record_t const * record, mm_node_position_t* effective_position);

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
                       DEFINITIONS
**********************************************************/

/**
 * Initialize growth logic.
 */
void mm_activity_variable_growth_init(void)
{
    mm_fetch_node_positions(&node_positions[0]);
}

/**
 * On sensor detection.
 */
void mm_activity_variable_growth_on_sensor_detection(sensor_evt_t const * evt)
{
    //TODO: check for sensor disable
    apply_activity_variable_addition(evt);
}

/**
 * Called once per second.
 */
void mm_activity_variable_growth_on_second_elapsed(void)
{
    apply_activity_variable_trickle_factor();
}

/**
    Called before clearing node position changed flag.
*/
void mm_activity_variable_growth_on_node_positions_update(void)
{
    mm_fetch_node_positions(&node_positions[0]);
}

/**
    Check for change in node positions, and fetch if needed.
*/
static void mm_fetch_node_positions(mm_node_position_t * node_positions_pointer)
{
    if( have_node_positions_changed() )
    {
        memcpy(node_positions_pointer, get_node_positions(), sizeof( MAX_NUMBER_NODES ) );

        /* Don't clear flag, that could affect other users, the main algorithm file will manage that. */
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
        apply_activity_variable_trickle_to_record(&(sensor_records[i]));
    }
}

/**
 * Apply growth factors to activity variables on pir detection.
 */ 
static void apply_activity_variable_addition_pir(sensor_evt_t const * evt)
{
    pir_evt_data_t const * pir_evt = &evt->pir_data;

    /* Do we know the node that generated the event? */
    mm_fetch_node_positions(&node_positions[0]);
    mm_node_position_t const * position = get_position_for_node(pir_evt->node_id);

    if(NULL == position)
    {
        /* We don't know the node, so we can't use its data. */
        return;
    }

    /* Get the sensor record. (A record will be allocted if one does not exist.) */
    sensor_record_t* record = get_sensor_record(evt);

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
    mm_fetch_node_positions(&node_positions[0]);
    mm_node_position_t const * position = get_position_for_node(lidar_evt->node_id);

    if(NULL == position)
    {
        /* We don't know the node, so we can't use its data. */
        return;
    }

    /* Get the sensor record. (A record will be allocted if one does not exist.) */
    sensor_record_t* record = get_sensor_record(evt);

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
static void apply_activity_variable_trickle_to_record(sensor_record_t const * record)
{
    if(!record->is_valid)
    {
        return;
    }

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
    mm_fetch_node_positions(&node_positions[0]);
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
    mm_node_rotation_t rotation = evt->sensor_rotation + position->node_rotation;
    rotation %= NODE_ROTATION_COUNT;

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
static void get_effective_location_for_lidar_detection(mm_node_position_t const * position, sensor_record_t const * record, mm_node_position_t* effective_position)
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
    mm_node_rotation_t rotation = record->sensor_rotation + position->node_rotation;
    rotation %= NODE_ROTATION_COUNT;

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
static sensor_record_t* get_sensor_record(sensor_evt_t const * evt)
{
    sensor_record_t * p_empty = NULL;
    sensor_record_t const * p_list_end = &sensor_records[MAX_SENSOR_COUNT];
 
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
    for(sensor_record_t* p_list =  &(sensor_records[0]); p_list <= p_list_end; ++p_list)
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
    init_sensor_record(evt, p_empty);
    return p_empty;
}

/**
 * Initialize a new sensor detection record.
 */
static void init_sensor_record(sensor_evt_t const * evt, sensor_record_t* p_record)
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
    memset(av_set, 0, sizeof(activity_variable_set_t));

    /* Find sensor rotation relative to node rotation 0 */
    mm_node_rotation_t rotation = position->node_rotation + sensor_rotation;
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
    av_set->avs[av_set->av_count] = mm_av_access(x, y);
    av_set->av_count++;
}
