
/**
file: mm_activity_variables.c
brief:
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mm_activity_variables.h"

/**********************************************************
                        MACROS
**********************************************************/

/**********************************************************
                          TYPES
**********************************************************/

/**********************************************************
                       VARIABLES
**********************************************************/

static mm_sensor_algorithm_config_t const * sensor_algorithm_config;

/**
     Activity variable definition, can be accessed as AV(x, y).

     X is indexed left -> right. (In direction of North-American traffic)
     Y is indexed top -> bottom. (Moving away from the road)

     Variable (x, y) is located at activity_variables[y * MAX_AV_SIZE_Y + x]
*/
static mm_activity_variable_t * activity_variables;

static uint16_t max_av_size_x;
static uint16_t max_av_size_y;
static uint16_t activity_variables_num;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_activity_variables_init(mm_sensor_algorithm_config_t const * config)
{
	sensor_algorithm_config = config;
	max_av_size_x = (sensor_algorithm_config->max_grid_size_x) - 1;
	max_av_size_y = (sensor_algorithm_config->max_grid_size_y) - 1;
	activity_variables_num = max_av_size_x * max_av_size_y;

	activity_variables = malloc(activity_variables_num * sizeof(mm_activity_variable_t));
	
	/* Initialize activity variables. */
    memset(&(activity_variables[0]), 0, sizeof(activity_variables));
    
    for(uint16_t i = 0; i < activity_variables_num; ++i)
    {
        activity_variables[i] = sensor_algorithm_config->activity_variable_min;
    }
}


mm_activity_variable_t* mm_av_access(uint8_t x, uint8_t y)
{
    return &activity_variables[y * max_av_size_y + x];
}


/**
    Gets the status for an AV based on the appropriate detection threshold value.
*/
activity_variable_state_t mm_get_status_for_av(mm_activity_variable_t const * av)
{
    /* Check if provided av is roadside. */
    bool is_roadside = false;
    for (uint8_t x = 0; x < max_av_size_x; ++x)
    {
        if (av == mm_av_access(x, 1))
        {
            is_roadside = true;
            break;
        }
    }

    /* Collect the correct thresholds. */
    float low_thresh = is_roadside ? sensor_algorithm_config->possible_detection_threshold_rs : sensor_algorithm_config->possible_detection_threshold_nrs;
    float high_thresh = is_roadside ? sensor_algorithm_config->detection_threshold_rs : sensor_algorithm_config->detection_threshold_nrs;

    /* Check against the thresholds. */
    if (*av < low_thresh)
    {
        return ACTIVITY_VARIABLE_STATE_IDLE;
    }
    else if (*av < high_thresh)
    {
        return ACTIVITY_VARIABLE_STATE_POSSIBLE_DETECTION;
    }
    else
    {
        return ACTIVITY_VARIABLE_STATE_DETECTION;
    }
}

/**
 * Get the max AV size in the X direction.
 */
uint16_t mm_get_max_av_size_x()
{
	return max_av_size_x;
}

/**
 * Get the max AV size in the Y direction.
 */
uint16_t mm_get_max_av_size_y()
{
	return max_av_size_y;
}

/**
 * Get the number of AVs.
 */
uint16_t mm_get_activity_variables_num()
{
	return activity_variables_num;
}
