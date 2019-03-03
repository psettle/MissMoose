
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
#include "mm_activity_variable_growth_prv.h"
#include "mm_activity_variables.h"

#include "mm_activity_variable_growth_prv.h"
#include "mm_activity_variable_growth_pir_prv.h"
#include "mm_activity_variable_growth_lidar_prv.h"
#include "mm_activity_variable_growth_sensor_records_prv.h"

#include "mm_av_transmission.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_ADJACENT_ACTIVITY_VARIABLES ( 2 )   /* The max number of regions a sensor can affect per detection. */

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

/**********************************************************
                       VARIABLES
**********************************************************/

static mm_sensor_algorithm_config_t const * sensor_algorithm_config;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Apply growth given avs and constants.
 */ 
static void grow_activity_variables(activity_variable_set_t* av_set, activity_variable_sensor_constants_t const * constants);

/**
 * Calculate which activity variable regions are adjacent to a made detection.
 * 
 * [out] the set of activity variables that are adjacent.
 */
static void find_adjacent_activity_variables(int8_t xpos, int8_t ypos, total_rotation_t sensor_rotation, activity_variable_set_t* av_set);

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
void mm_activity_variable_growth_init(mm_sensor_algorithm_config_t const * config)
{
	sensor_algorithm_config = config;

	set_lidar_sensor_algorithm_config(config);
	set_pir_sensor_algorithm_config(config);

	init_sensor_records();
}

/**
 * On sensor detection.
 */
void mm_activity_variable_growth_on_sensor_detection(sensor_evt_t const * evt)
{
    /* Pass event to specific sensor processing. */
    switch(evt->sensor_type)
    {
    case SENSOR_TYPE_PIR:
        translate_pir_detection(evt);
        break;
    case SENSOR_TYPE_LIDAR:
        translate_lidar_detection(evt);
        break;
    default:
        APP_ERROR_CHECK(true);
        break;
    }
}

/**
 * Called once per second.
 */
void mm_activity_variable_growth_on_second_elapsed(void)
{
    /* Pass event to specific sensor processing. */
    translate_on_second_evt_pir();
    translate_on_second_evt_lidar();
}

void process_abstract_detection(abstract_sensor_detection_t* detection)
{
    /* Which AV's does the detection apply to? */
    activity_variable_set_t av_set;
    find_adjacent_activity_variables(detection->xpos, detection->ypos, detection->rotation, &av_set);

    /* Apply growth */
    grow_activity_variables(&av_set, &detection->constants);
}

void get_grid_direction(total_rotation_t rotation, int8_t* dx, int8_t* dy)
{
    switch(rotation)
    {
        case TOTAL_ROTATION_0:
            /* Next node is in +y direction. */
            *dx = 0;
            *dy = 1;
            break;
        case TOTAL_ROTATION_90:
            /* Next node is in +x direction. */
            *dx = 1;
            *dy = 0;
            break;
        case TOTAL_ROTATION_180:
            /* Next node is in -y direction. */
            *dx = 0;
            *dy = -1;
            break;
        case TOTAL_ROTATION_270:
            /* Next node is in -x direction. */
            *dx = -1;
            *dy = 0;
            break;
        default:
            /* Current design does not support intermediate angles. */
            APP_ERROR_CHECK(true);
            break;
    }
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

        /* Apply the factor */
        *(av_set->avs[i]) *= factor;

        /* Check if max exceeded */
        if(*(av_set->avs[i]) > sensor_algorithm_config->activity_variable_max)
        {
            *(av_set->avs[i]) = sensor_algorithm_config->activity_variable_max;
        }
    }
}

/**
 * Compute which AV regions are adjacent to a sensor detection.
 */
static void find_adjacent_activity_variables(int8_t xpos, int8_t ypos, total_rotation_t rotation, activity_variable_set_t* av_set)
{
    memset(av_set, 0, sizeof(activity_variable_set_t));

    /* Convert node position (-1 to MAX_GRID_SIZE - 1) into
       'av' coordinates (0 to MAX_GRID_SIZE)) */
    uint8_t x_array = xpos + 1;
    uint8_t y_array = ypos + 1;
    /* Positon coordinates and AV coordinates use inverted Y direction. */
    y_array = MAX_GRID_SIZE_Y - y_array - 1;

    /* On any case x or y may underflow, if that happens they will be too big and won't be saved. */
    switch(rotation)
    {
        case TOTAL_ROTATION_0:
            /* Faces up, so can only affect row - 1, and column, column - 1 */
            check_add_av(x_array, y_array - 1, av_set);
            check_add_av(x_array - 1, y_array - 1, av_set);
            break;
        case TOTAL_ROTATION_90:
            /* Faces right, so can only affect row, row - 1, and column */   
            check_add_av(x_array, y_array, av_set);
            check_add_av(x_array, y_array - 1, av_set);
            break;
        case TOTAL_ROTATION_180:
            /* Faces down, so can only affect row, and column, column - 1 */ 
            check_add_av(x_array, y_array, av_set);
            check_add_av(x_array - 1, y_array, av_set);
            break;
        case TOTAL_ROTATION_270:
            /* Faces left, so can only affect row, row - 1, column - 1 */
            check_add_av(x_array - 1, y_array, av_set);
            check_add_av(x_array - 1, y_array - 1, av_set);
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