/**
file: mm_sensor_algorithm_config.h
brief:
notes:
*/
#ifndef MM_SENSOR_ALGORITHM_CONFIG_H
#define MM_SENSOR_ALGORITHM_CONFIG_H

/**********************************************************
                    ALGORITHM TUNING
**********************************************************/

#define MAX_GRID_SIZE_X         ( 3 )
#define MAX_GRID_SIZE_Y         ( 3 )

#define ACTIVITY_VARIABLE_MIN   ( 1.0f )
#define ACTIVITY_VARIABLE_MAX   ( 12.0f )

#define COMMON_SENSOR_WEIGHT_FACTOR         ( 1.0f )
#define BASE_SENSOR_WEIGHT_FACTOR_PIR       ( 3.0f )
#define BASE_SENSOR_WEIGHT_FACTOR_LIDAR     ( 3.5f )
#define ROAD_PROXIMITY_FACTOR_0             ( 1.4f )
#define ROAD_PROXIMITY_FACTOR_1             ( 1.2f )
#define ROAD_PROXIMITY_FACTOR_2             ( 1.0f )

#define COMMON_SENSOR_TRICKLE_FACTOR        ( 1.0f )
#define BASE_SENSOR_TRICKLE_FACTOR_PIR      ( 1.003f )
#define BASE_SENSOR_TRICKLE_FACTOR_LIDAR    ( 1.0035f )
#define ROAD_TRICKLE_PROXIMITY_FACTOR_0     ( 1.004f )
#define ROAD_TRICKLE_PROXIMITY_FACTOR_1     ( 1.002f )
#define ROAD_TRICKLE_PROXIMITY_FACTOR_2     ( 1.0f )

#endif /* MM_SENSOR_ALGORITHM_CONFIG_H */

