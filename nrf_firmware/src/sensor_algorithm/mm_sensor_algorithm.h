/**
file: mm_sensor_algorithm.h
brief:
notes:
*/
#ifndef MM_SENSOR_ALGORITHM_H
#define MM_SENSOR_ALGORITHM_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Initialize and start the sensor data processing algorithm.
 *
 * Note: Requires sensor_transmission.h is initialized.
 */
void mm_sensor_algorithm_init(mm_sensor_algorithm_config_t const * config);

#ifdef MM_ALLOW_SIMULATED_TIME
    /**
     * Simulate a second passing, only use for simulating time, not in production.
     */
    void mm_sensor_algorithm_on_second_elapsed(void);
#endif

#endif /* MM_SENSOR_ALGORITHM_H */
