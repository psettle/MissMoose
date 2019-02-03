/**
file: mm_activity_variable_growth.h
brief:
notes:
*/
#ifndef MM_ACTIVITY_VARIABLE_GROWTH_H
#define MM_ACTIVITY_VARIABLE_GROWTH_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_sensor_transmission.h"
#include "mm_position_config.h"

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Initialize growth logic.
 */
void mm_activity_variable_growth_init(void);

/**
 * On sensor detection.
 */
void mm_activity_variable_growth_on_sensor_detection(sensor_evt_t const * evt);

/**
 * Call once per second.
 */
void mm_activity_variable_growth_on_second_elapsed(void);

/**
    Called before clearing node position changed flag.
*/
void mm_activity_variable_growth_on_node_positions_update(void);

#endif /* MM_ACTIVITY_VARIABLE_GROWTH_H */