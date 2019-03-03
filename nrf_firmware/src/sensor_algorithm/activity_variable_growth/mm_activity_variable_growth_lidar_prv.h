/**
file: mm_activity_variable_growth_lidar_prv.h
brief:
notes:
*/
#ifndef MM_ACTIVITY_VARIABLE_GROWTH_LIDAR_PRV_H
#define MM_ACTIVITY_VARIABLE_GROWTH_LIDAR_PRV_H

/**********************************************************
                          INCLUDES
**********************************************************/

#include "mm_sensor_transmission.h"
#include "mm_sensor_algorithm_config.h"

/**********************************************************
                          TYPES
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Translates a lidar detection event into an abstract detection event.
 */
void translate_lidar_detection(sensor_evt_t const * sensor_evt);

/**
 * Translates an on second event into 0 or more abstract detection events.
 */
void translate_on_second_evt_lidar(void);

#endif /* MM_ACTIVITY_VARIABLE_GROWTH_LIDAR_PRV_H */
