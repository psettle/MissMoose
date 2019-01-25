
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

/* Ease of access macro, optional usage. */
#define AV(x, y)                ( activity_variables[(x) * MAX_AV_SIZE_X + (y)] )

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

/* If AV outside min and max val, set to closest limit. */
static void check_av_limits(uint8_t x, uint8_t y);

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

void mm_av_multiply(uint8_t x, uint8_t y, float factor)
{
    AV(x, y) *= factor;
    check_av_limits(x, y);
}

mm_activity_variable_t mm_av_access(uint8_t x, uint8_t y)
{
    return AV(x, y);
}

mm_activity_variable_t* mm_av_fetch(uint8_t x, uint8_t y)
{
    return &AV(x, y);
}

void mm_av_set(uint8_t x, uint8_t y, mm_activity_variable_t val)
{
    AV(x, y) = val;
    check_av_limits(x, y);
}

static void check_av_limits(uint8_t x, uint8_t y)
{
    if(AV(x, y) < ACTIVITY_VARIABLE_MIN)
    {
        AV(x, y) = ACTIVITY_VARIABLE_MIN;
    }
    else if(AV(x, y) > ACTIVITY_VARIABLE_MAX)
    {
        AV(x, y) = ACTIVITY_VARIABLE_MAX;
    }
}