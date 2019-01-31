
/**
file: mm_activity_variables.c
brief:
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include <string.h>

#include "mm_sensor_algorithm_config.h"
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

     Variable (x, y) is located at activity_variables[x * MAX_AV_SIZE_X + y]
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
        activity_variables[i] = ACTIVITY_VARIABLE_MIN;
    }
}

mm_activity_variable_t* mm_av_access(uint8_t x, uint8_t y)
{
    return &activity_variables[x * MAX_AV_SIZE_X + y];
}
