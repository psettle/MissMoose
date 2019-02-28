/*
 * mm_activity_variable_drain.h
 *
 *  Created on: Jan 31, 2019
 *      Author: nataliewong
 */

#ifndef MM_ACTIVITY_VARIABLE_DRAIN_H
#define MM_ACTIVITY_VARIABLE_DRAIN_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                        MACROS
**********************************************************/

/**********************************************************
                        TYPES
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Sets the sensor algorithm configuration constants. Should never
 * be called more than once.
 */
void mm_set_activity_variable_drain_config(mm_sensor_algorithm_config_t const * config);

/**
    Applies the activity variable drain factor to all activity variables.
*/
void mm_apply_activity_variable_drain_factor(void);

#endif /* MM_ACTIVITY_VARIABLE_DRAIN_H */
