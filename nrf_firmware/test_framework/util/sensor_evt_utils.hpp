/**
file: sensor_evt_utils.hpp
brief:
notes:
*/
#ifndef SENSOR_EVT_UTILS_HPP
#define SENSOR_EVT_UTILS_HPP

/**********************************************************
INCLUDES
**********************************************************/

#include <vector>
extern "C" {
#include "mm_sensor_error_check.h"
}

/**********************************************************
CONSTANTS
**********************************************************/

/**********************************************************
DECLARATIONS
**********************************************************/

/* Create big list of pir sensor events so that we can use a function to check if a sensor is hyperactive. */
void create_all_pir_sensor_evts(std::vector<sensor_evt_t>& evts, bool detection);

/* Create big list of lidar sensor events so that we can use a function to check if a sensor is hyperactive. */
void create_all_lidar_sensor_evts(std::vector<sensor_evt_t>& evts, uint16_t distance);

/* Helper function for making events so we can call is_sensor_hyperactive */
sensor_evt_t create_lidar_sensor_evt
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    uint16_t distance_measured
);

/* Helper function for making events so we can call is_sensor_hyperactive */
sensor_evt_t create_pir_sensor_evt
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    bool detection
);

#endif /* SENSOR_EVT_UTILS_HPP */