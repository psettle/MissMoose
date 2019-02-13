/**
file: mm_sensor_error_check.h
brief:
notes:
*/
#ifndef MM_SENSOR_ERROR_CHECK_H
#define MM_SENSOR_ERROR_CHECK_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_sensor_transmission.h"

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Initialize sensor error checking.
*/
void mm_sensor_error_init(void);

/**
    Record that a sensor has been active (has had a detection event).
*/
void mm_sensor_error_record_sensor_activity(sensor_evt_t const * evt, uint32_t minute_count);

/**
    Analyze collected data and update error states.
 */
void mm_sensor_error_on_minute_elapsed(uint32_t minute_count);

/**
    Called before clearing node position changed flag.
*/
void mm_sensor_error_on_node_positions_update(uint32_t minute_count);

/**
    Returns true if the sensor in the given event is marked as hyperactive
*/
bool mm_sensor_error_is_sensor_hyperactive(sensor_evt_t const * evt);


#endif /* MM_SENSOR_ERROR_CHECK_H */