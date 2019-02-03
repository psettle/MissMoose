/**
file: mm_sensor_algorithm.h
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
    Record that a sensor has been active (has had a detection event).
*/
void mm_record_sensor_activity(sensor_evt_t const * evt, uint16_t minute_count);

/**
    Checks to ensure all sensors have made at least 1 detection in the last 24 hours
*/
void mm_check_for_sensor_inactivity(void);

/**
    Checks for any sensors which are hyperactive. Flags hyperactive sensors so their events will be ignored.
*/
void mm_check_for_sensor_hyperactivity(void);

/**
    Returns true if the sensor in the given event is marked as hyperactive
*/
bool mm_is_sensor_hyperactive(sensor_evt_t const * evt);

/**
    Called before clearing node position changed flag.
*/
void mm_sensor_error_check_on_node_positions_update(void);

#endif /* MM_SENSOR_ERROR_CHECK_H */