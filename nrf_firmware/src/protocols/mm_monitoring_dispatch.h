/**
file: mm_monitoring_dispatch.h
brief: Interface functions for sending ANT page information to the monitoring application.
notes:
*/

#ifndef MM_MONITORING_DISPATCH_H
#define MM_MONITORING_DISPATCH_H

/**********************************************************
                        INCLUDES
**********************************************************/

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/
/*
A sensors position on a node is defined as a clockwise angle from a 0° direction. 
Similar to node rotation, the 0° direction of a node is defined as:
 If the node has a LIDAR sensor, the 0° direction is the direction of the most clockwise LIDAR sensor.
 If the node has only PIR sensors, its rotation is the direction of the most clockwise PIR sensor.
*/
typedef enum
{
    SENSOR_ROTATION_0,
    SENSOR_ROTATION_45,
    SENSOR_ROTATION_90,
    SENSOR_ROTATION_135,
    SENSOR_ROTATION_180,
    SENSOR_ROTATION_225,
    SENSOR_ROTATION_270,
    SENSOR_ROTATION_315
} sensor_rotation_t; 

void mm_monitoring_dispatch_main(void);

void mm_monitoring_dispatch_init(void);

void mm_monitoring_dispatch_send_lidar_data
    (
    uint16_t node_id, 
    sensor_rotation_t sensor_rotation, 
    uint16_t distance_measured
    );

void mm_monitoring_dispatch_send_pir_data
    (
    uint16_t node_id, 
    sensor_rotation_t sensor_rotation, 
    bool detection
    );


#endif /* MM_MONITORING_DISPATCH_H */
