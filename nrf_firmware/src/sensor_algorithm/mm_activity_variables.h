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

/**
 * Get the max AV size in the X direction.
 */
uint16_t mm_get_max_av_size_x();

/**
 * Get the max AV size in the Y direction.
 */
uint16_t mm_get_max_av_size_y();

/**
 * Get the number of AVs.
 */
uint16_t mm_get_activity_variables_num();

#endif /* MM_ACTIVITY_VARIABLES_H */