
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
                    ALGORITHM CONSTANTS
**********************************************************/

/**
    Static sensor algorithm constants.
*/
#define MAX_GRID_SIZE_X                     ( 3 )
#define MAX_GRID_SIZE_Y                     ( 3 )
#define MAX_NUMBER_NODES                    ( MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y )
#define MAX_SENSORS_PER_NODE                ( 2 )
#define MAX_SENSOR_COUNT                    ( MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE )

#define SENSOR_INACTIVITY_THRESHOLD_MIN         ( 60 * 24 )
#define SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE  ( 120 )
#define SENSOR_HYPERACTIVITY_FREQUENCY_THRES    ( 1.0 ) // events / SENSOR_HYPERACTIVITY_DETECTION_PERIOD

/**********************************************************
                    ALGORITHM TUNING
**********************************************************/

/**
    Dynamic sensor algorithm constants container.
*/
typedef struct
{
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

    float activity_variable_decay_factor;
    uint16_t activity_decay_period_ms;

    /* Road-side (RS), non-road-side (NRS) */
    float possible_detection_threshold_rs;
    float possible_detection_threshold_nrs;

    float detection_threshold_rs;
    float detection_threshold_nrs;

    uint16_t minimum_concern_signal_duration_s;
    uint16_t minimum_alarm_signal_duration_s;
} mm_sensor_algorithm_config_t;


/**********************************************************
                     DECLARATIONS
**********************************************************/

/**
    Initializes the dynamic sensor algorithm configuration
    constants. Should never be used more than once!
*/
void mm_sensor_algorithm_config_init(mm_sensor_algorithm_config_t const * config);

/**
    Gets the dynamic sensor algorithm configuration constants. Assumes that
    they have been previously configured using mm_sensor_algorithm_config_init
    once before!
*/
mm_sensor_algorithm_config_t const * mm_sensor_algorithm_config(void);

#endif /* MM_SENSOR_ALGORITHM_CONFIG_H */

