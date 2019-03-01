/**
file: mm_activity_variable_growth_prv.h
brief:
notes:
*/
#ifndef MM_ACTIVITY_VARIABLE_GROWTH_PRV_H
#define MM_ACTIVITY_VARIABLE_GROWTH_PRV_H

/**********************************************************
                          INCLUDES
**********************************************************/

#include "mm_activity_variable_growth.h"

/**********************************************************
                          TYPES
**********************************************************/

typedef enum
{
    TOTAL_ROTATION_0,
    TOTAL_ROTATION_45,
    TOTAL_ROTATION_90,
    TOTAL_ROTATION_135,
    TOTAL_ROTATION_180,
    TOTAL_ROTATION_225,
    TOTAL_ROTATION_270,
    TOTAL_ROTATION_315,
    TOTAL_ROTATION_360,

    TOTAL_ROTATION_COUNT
} total_rotation_t;

typedef struct
{
    float common_sensor_weight_factor;
    float base_sensor_weight_factor;
    float road_proximity_factor;
} activity_variable_sensor_constants_t;

typedef struct
{
    int8_t xpos;                    /* Where the detection came from. */
    int8_t ypos;                    /* Where the detection came from.*/
    total_rotation_t rotation;      /* Which direction the detection came from. */
    activity_variable_sensor_constants_t constants; /* Constants to use while applying the detection. */
} abstract_sensor_detection_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Processes an abstract detection.
 */
void process_abstract_detection(abstract_sensor_detection_t* detection);

/**
 * Get the direction along the grid in a [rotation] direction.
 */
void get_grid_direction(total_rotation_t rotation, int8_t* dx, int8_t* dy);

#endif /* MM_ACTIVITY_VARIABLE_GROWTH_H */