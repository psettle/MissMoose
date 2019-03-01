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

#define MAX_GRID_SIZE_X                     ( 3 )
#define MAX_GRID_SIZE_Y                     ( 3 )
#define MAX_NUMBER_NODES                    ( MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y )
#define MAX_SENSORS_PER_NODE                ( 2 )
#define MAX_SENSOR_COUNT                    ( MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE )     

#define MAX_NUMBER_NODES                    ( MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y )
#define MAX_SENSORS_PER_NODE                ( 2 )
#define MAX_SENSOR_COUNT                    ( MAX_NUMBER_NODES * MAX_SENSORS_PER_NODE )  

#define ACTIVITY_VARIABLE_MIN               ( 1.0f )
#define ACTIVITY_VARIABLE_MAX               ( 12.0f )

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

#define SENSOR_INACTIVITY_THRESHOLD_MIN         ( 1 )
#define SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE  ( 120 )
#define SENSOR_HYPERACTIVITY_FREQUENCY_THRES    ( 1.0 ) // events / SENSOR_HYPERACTIVITY_DETECTION_PERIOD

#define ACTIVITY_VARIABLE_DECAY_FACTOR      ( 0.99f )
#define ACTIVITY_DECAY_PERIOD_MS            ( ONE_SECOND_MS )

/* Road-side (RS), non-road-side (NRS) */
#define POSSIBLE_DETECTION_THRESHOLD_RS     ( 3.0f )
#define POSSIBLE_DETECTION_THRESHOLD_NRS    ( 4.0f )

#define DETECTION_THRESHOLD_RS              ( 6.0f )
#define DETECTION_THRESHOLD_NRS             ( 7.0f )

#define MINIMUM_CONCERN_SIGNAL_DURATION_S ( 60 )
#define MINIMUM_ALARM_SIGNAL_DURATION_S   ( 30 )

#endif /* MM_SENSOR_ALGORITHM_CONFIG_H */

