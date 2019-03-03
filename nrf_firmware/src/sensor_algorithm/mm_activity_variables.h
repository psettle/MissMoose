/**
file: mm_activity_variables.h
brief:
notes:
    x is indexed left -> right. (In direction of North-American traffic)
    y is indexed top -> bottom. (Moving away from the road)
*/
#ifndef MM_ACTIVITY_VARIABLES_H
#define MM_ACTIVITY_VARIABLES_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdint.h>

#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_AV_SIZE_X           ( MAX_GRID_SIZE_X - 1 )
#define MAX_AV_SIZE_Y           ( MAX_GRID_SIZE_Y - 1 )
#define ACTIVITY_VARIABLES_NUM  ( MAX_AV_SIZE_X * MAX_AV_SIZE_Y )

/**********************************************************
                        MACROS
**********************************************************/

/*
   Ease of access macros, optional usage:
   ex.
   AV(0, 1) = 1.2f;
   AV_TOP_LEFT = 1.2f;
   AV_TOP_LEFT = AV_TOP_RIGHT;
   
   */
#define AV(x, y)                (*(mm_av_access((x),(y))))
#define AV_TOP_LEFT             AV(0, 0)
#define AV_TOP_RIGHT            AV(1, 0)
#define AV_BOTTOM_LEFT          AV(0, 1)
#define AV_BOTTOM_RIGHT         AV(1, 1)

/**********************************************************
                        TYPES
**********************************************************/

typedef float mm_activity_variable_t;

typedef enum
{
    ACTIVITY_VARIABLE_STATE_IDLE,
    ACTIVITY_VARIABLE_STATE_POSSIBLE_DETECTION,
    ACTIVITY_VARIABLE_STATE_DETECTION,

    ACTIVITY_VARIABLE_STATE_COUNT
} activity_variable_state_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Initialize activity variables.
 */
void mm_activity_variables_init(mm_sensor_algorithm_config_t const * config);

/**
 * Access AV value.
 */
mm_activity_variable_t* mm_av_access(uint8_t x, uint8_t y);

/**
 * Check which threshold an activity variable falls under.
 */
activity_variable_state_t mm_get_status_for_av(mm_activity_variable_t const * av);

#endif /* MM_ACTIVITY_VARIABLES_H */