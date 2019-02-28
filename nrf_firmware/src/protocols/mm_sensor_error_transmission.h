/*
file: mm_sensor_error_transmission.h
brief: Methods for transmitting led data to the monitoring application over ANT
notes:
*/
#ifndef MM_SENSOR_ERROR_TRANSMISSION_H
#define MM_SENSOR_ERROR_TRANSMISSION_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdint.h>
#include <mm_sensor_transmission.h>

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Initialize sensor error state transmission over ANT.
*/
void mm_sensor_error_transmission_init(void);

/**
 * Set a sensor's hyperactivity error state to be broadcast over ANT.
*/
void mm_sensor_error_transmission_send_hyperactivity_update
    (
    uint16_t node_id,
    sensor_type_t sensor_type,
    sensor_rotation_t sensor_rotation,
    bool error_occuring
    );

/**
 * Set a sensor's inactivity error state to be broadcast over ANT.
*/
void mm_sensor_error_transmission_send_inactivity_update
    (
    uint16_t node_id,
    sensor_type_t sensor_type,
    sensor_rotation_t sensor_rotation,
    bool error_occuring
    );

#endif // MM_SENSOR_ERROR_TRANSMISSION_H