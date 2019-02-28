/**
file: mm_sensor_algorithm_config.h
brief:
notes:
*/
#ifndef MM_SENSOR_ALGORITHM_CONFIG_H
#define MM_SENSOR_ALGORITHM_CONFIG_H

/**********************************************************
					    INCLUDES
**********************************************************/

#include "stdint.h"

/**********************************************************
                    ALGORITHM TUNING
**********************************************************/

/**
    Sensor algorithm configuration constants container.
*/
typedef struct
{
    uint16_t max_grid_size_x;             
	uint16_t max_grid_size_y;
	uint16_t max_number_nodes;
	uint16_t max_sensors_per_node;
	uint16_t max_sensor_count;

    float activity_variable_min;    
    float activity_variable_max;  

    float common_sensor_weight_factor;
    float base_sensor_weight_factor_pir;
    float base_sensor_weight_factor_lidar;
    float road_proximity_factor_0;
    float road_proximity_factor_1;
    float road_proximity_factor_2;

    float common_sensor_trickle_factor;
    float base_sensor_trickle_factor_pir;
    float base_sensor_trickle_factor_lidar;
    float road_trickle_proximity_factor_0;
    float road_trickle_proximity_factor_1;
    float road_trickle_proximity_factor_2;

	uint16_t sensor_inactivity_threshold_min;
	uint16_t sensor_hyperactivity_event_window_size;
    float sensor_hyperactivity_frequency_thres; // events / SENSOR_HYPERACTIVITY_DETECTION_PERIOD

    float activity_variable_decay_factor;
    int activity_decay_period_ms;

    /* Road-side (RS), non-road-side (NRS) */
	float possible_detection_threshold_rs;
	float possible_detection_threshold_nrs;

    float detection_threshold_rs;
    float detection_threshold_nrs;

	uint16_t minimum_concern_signal_duration_s;
	uint16_t minimum_alarm_signal_duration_s;
} mm_sensor_algorithm_config_t;

/**
    Default sensor algorithm configuration constants.
*/
mm_sensor_algorithm_config_t const MM_SENSOR_ALGORITHM_CONFIG_DEFAULT =
{
    3,  // max_grid_size_x
    3,  // max_grid_size_y
    9,  // max_number_nodes
    2,  // max_sensors_per_node
    18, // max_sensor_count

    1.0f,   // activity_variable_min
    12.0f,  // activity_variable_max

    1.0f, // common_sensor_weight_factor
    3.0f, // base_sensor_weight_factor_pir
    3.5f, // base_sensor_weight_factor_lidar
    1.4f, // road_proximity_factor_0
    1.2f, // road_proximity_factor_1
    1.0f, // road_proximity_factor_2

    1.0f,       // common_sensor_trickle_factor
    1.003f,     // base_sensor_trickle_factor_pir
    1.0035f,    // base_sensor_trickle_factor_lidar
    1.004f,     // road_trickle_proximity_factor_0
    1.002f,     // road_trickle_proximity_factor_1
    1.0f,       // road_trickle_proximity_factor_2

    (60 * 24 ), // sensor_inactivity_threshold_min
    120,        // sensor_hyperactivity_event_window_size
    1.0f,       // sensor_hyperactivity_frequency_thres

    0.99f, // activity_variable_decay_factor
    1000,  // activity_decay_period_ms

    3.0f, // possible_detection_threshold_rs
    4.0f, // possible_detection_threshold_nrs

    6.0f, // detection_threshold_rs
    7.0f, // detection_threshold_nrs

    30, // minimum_concern_signal_duration_s
    60 // minimum_alarm_signal_duration_s
};


#endif /* MM_SENSOR_ALGORITHM_CONFIG_H */

