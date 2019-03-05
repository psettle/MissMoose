/**
file: mm_activity_variable_growth_pir_prv.h
brief:
notes:
*/
#ifndef MM_ACTIVITY_VARIABLE_GROWTH_PIR_PRV_H
#define MM_ACTIVITY_VARIABLE_GROWTH_PIR_PRV_H

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
 * Translates a pir detection event into an abstract detection event.
 */
void translate_pir_detection(sensor_evt_t const * sensor_evt);

/**
 * Translates an on second event into 0 or more abstract detection events.
 */
void translate_on_second_evt_pir(void);

#endif /* MM_ACTIVITY_VARIABLE_GROWTH_PIR_PRV_H */
