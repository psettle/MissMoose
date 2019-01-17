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

#include "sensor_transmission.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

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
