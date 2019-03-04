
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

/**
     Activity variable definition, can be accessed as AV(x, y).

     X is indexed left -> right. (In direction of North-American traffic)
     Y is indexed top -> bottom. (Moving away from the road)

     Variable (x, y) is located at activity_variables[y * MAX_AV_SIZE_Y + x]
*/
static mm_activity_variable_t activity_variables[ACTIVITY_VARIABLES_NUM];

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_activity_variables_init(void)
{
    /* Initialize activity variables. */
    memset(&(activity_variables[0]), 0, sizeof(activity_variables));

    for(uint16_t i = 0; i < ACTIVITY_VARIABLES_NUM; ++i)
    {
        activity_variables[i] = mm_sensor_algorithm_config()->activity_variable_min;
    }
}


mm_activity_variable_t* mm_av_access(uint8_t x, uint8_t y)
{
    return &activity_variables[y * MAX_AV_SIZE_Y + x];
}


/**
    Gets the status for an AV based on the appropriate detection threshold value.
*/
activity_variable_state_t mm_get_status_for_av(mm_activity_variable_t const * av)
{
    /* Check if provided av is roadside. */
    bool is_roadside = false;
    for (uint8_t x = 0; x < MAX_AV_SIZE_X; ++x)
    {
        if (av == mm_av_access(x, 1))
        {
            is_roadside = true;
            break;
        }
    }

    /* Collect the correct thresholds. */
    float low_thresh = is_roadside ? mm_sensor_algorithm_config()->possible_detection_threshold_rs : mm_sensor_algorithm_config()->possible_detection_threshold_nrs;
    float high_thresh = is_roadside ? mm_sensor_algorithm_config()->detection_threshold_rs : mm_sensor_algorithm_config()->detection_threshold_nrs;

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
