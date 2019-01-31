/*
 * mm_activity_variable_drain.h
 *
 *  Created on: Jan 31, 2019
 *      Author: nataliewong
 */

#ifndef SRC_SENSOR_ALGORITHM_MM_ACTIVITY_VARIABLE_DRAIN_H_
#define SRC_SENSOR_ALGORITHM_MM_ACTIVITY_VARIABLE_DRAIN_H_

/**********************************************************
                        INCLUDES
**********************************************************/

/**********************************************************
                        CONSTANTS
**********************************************************/

#define ACTIVITY_VARIABLE_DECAY_FACTOR      ( 0.97f )
#define ACTIVITY_DECAY_PERIOD_MS            ( ONE_SECOND_MS )

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
    Applies the activity variable drain factor to all activity variables.
*/
void apply_activity_variable_drain_factor(void);

#endif /* SRC_SENSOR_ALGORITHM_MM_ACTIVITY_VARIABLE_DRAIN_H_ */
