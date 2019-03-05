/**
file: mm_sensor_transmission.hpp
brief: Header file for test framework functions used to simulate sensors detecting data
notes:

Author: Elijah Pennoyer
*/

#ifndef MM_SENSOR_TRANSMISSION_HPP
#define MM_SENSOR_TRANSMISSION_HPP

/**********************************************************
                        INCLUDES
**********************************************************/
extern "C" {
    #include "mm_sensor_transmission.h"
}

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/* Sends a PIR data event to the gateway node for processing. 
   For use in the testing framework. */
void test_send_pir_data
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    bool detection
);


/* Sends a LIDAR data event to the gateway node for processing. 
   For use in the testing framework.*/
void test_send_lidar_data
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    uint16_t distance_measured
);


void test_send_pir_data
(
    int8_t              x_pos,
    int8_t              y_pos,
    sensor_rotation_t   total_rotation,
    bool detection
);

void test_send_lidar_data
(
    int8_t              x_pos,
    int8_t              y_pos,
    sensor_rotation_t   total_rotation,
    uint16_t distance_measured
);


#endif