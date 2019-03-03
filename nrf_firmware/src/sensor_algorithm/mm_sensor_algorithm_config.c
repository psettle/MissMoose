/**
file: mm_sensor_algorithm_config.c
brief:
notes:
*/

/**********************************************************
					    INCLUDES
**********************************************************/

#include "mm_sensor_algorithm_config.h"

/**********************************************************
             ALGORITHM CONFIGURATION INSTANCE
**********************************************************/

/* Singleton container for the dynamic sensor algorithm constants. */
mm_sensor_algorithm_config_t sensor_algorithm_config;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Initializes the dynamic sensor algorithm configuration
    constants. Should never be used more than once!
*/
void mm_sensor_algorithm_config_init(mm_sensor_algorithm_config_t const config)
{
    sensor_algorithm_config = config;
}

/**
    Gets the dynamic sensor algorithm configuration constants. Assumes that
    they have been previously configured using mm_sensor_algorithm_config_init
    once before!
*/
mm_sensor_algorithm_config_t const * mm_sensor_algorithm_config(void)
{
    return &sensor_algorithm_config;
}
