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

#endif /* MM_SENSOR_ALGORITHM_CONFIG_H */

