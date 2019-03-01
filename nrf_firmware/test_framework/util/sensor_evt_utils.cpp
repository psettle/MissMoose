/**
file: sensor_evt_utils.cpp
brief:
notes:
*/

/**********************************************************
INCLUDES
**********************************************************/

#include <stdint.h>
#include <vector>
#include "sensor_evt_utils.hpp"

extern "C" {
#include "mm_sensor_error_check.h"
}

/**********************************************************
CONSTANTS
**********************************************************/

/**********************************************************
DEFINITIONS
**********************************************************/

void create_all_pir_sensor_evts(std::vector<sensor_evt_t>& evts, bool detection)
{
    evts.push_back(create_pir_sensor_evt(1, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(2, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(3, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(4, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(5, SENSOR_ROTATION_0, detection));
    evts.push_back(create_pir_sensor_evt(5, SENSOR_ROTATION_270, detection));
    evts.push_back(create_pir_sensor_evt(6, SENSOR_ROTATION_0, detection));
    evts.push_back(create_pir_sensor_evt(6, SENSOR_ROTATION_270, detection));
    evts.push_back(create_pir_sensor_evt(7, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(8, SENSOR_ROTATION_0, detection));
    evts.push_back(create_pir_sensor_evt(8, SENSOR_ROTATION_270, detection));
    evts.push_back(create_pir_sensor_evt(9, SENSOR_ROTATION_90, detection));
}

void create_all_lidar_sensor_evts(std::vector<sensor_evt_t>& evts, uint16_t distance)
{
    evts.push_back(create_lidar_sensor_evt(1, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(2, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(3, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(4, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(7, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(9, SENSOR_ROTATION_0, distance));
}

/* Helper function for making events so we can call is_sensor_hyperactive */
sensor_evt_t create_lidar_sensor_evt
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    uint16_t distance_measured
)
{
    //Create a sensor_event_t with SENSOR_TYPE_LIDAR
    sensor_evt_t sensor_evt;
    memset(&sensor_evt, 0, sizeof(sensor_evt));

    sensor_evt.sensor_type = SENSOR_TYPE_LIDAR;
    sensor_evt.lidar_data.node_id = node_id;
    sensor_evt.lidar_data.sensor_rotation = sensor_rotation;
    sensor_evt.lidar_data.distance_measured = distance_measured;

    return sensor_evt;
}

/* Helper function for making events so we can call is_sensor_hyperactive */
sensor_evt_t create_pir_sensor_evt
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    bool detection
)
{
    // Create a sensor_event_t with SENSOR_TYPE_PIR
    sensor_evt_t sensor_evt;
    memset(&sensor_evt, 0, sizeof(sensor_evt));

    sensor_evt.sensor_type = SENSOR_TYPE_PIR;
    sensor_evt.pir_data.node_id = node_id;
    sensor_evt.pir_data.sensor_rotation = sensor_rotation;
    sensor_evt.pir_data.detection = detection;

    return sensor_evt;
}
