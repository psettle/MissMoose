/*
 * mm_activity_variable_drain.c
 *
 *  Created on: Jan 31, 2019
 *      Author: nataliewong
 */

/**********************************************************
                       INCLUDES
**********************************************************/

#include "mm_activity_variable_drain.h"
#include "mm_activity_variables.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          TYPES
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**********************************************************
                       VARIABLES
**********************************************************/

static mm_sensor_algorithm_config_t const * sensor_algorithm_config;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Sets the sensor algorithm configuration constants. Should never
 * be called more than once.
 */
void mm_activity_variable_drain_init(mm_sensor_algorithm_config_t const * config)
{
	sensor_algorithm_config = config;
}

/**
    Applies the activity variable drain factor to all activity variables.
*/
void mm_apply_activity_variable_drain_factor(void)
{
    for ( uint8_t x = 0; x < mm_get_max_av_size_x(); x++ )
    {
        for ( uint8_t y = 0; y < mm_get_max_av_size_y(); y++ )
        {
            if ( AV(x, y) > sensor_algorithm_config->activity_variable_min )
            {
                AV(x, y) *= sensor_algorithm_config->activity_variable_decay_factor;

                /* Enforce ACTIVITY_VARIABLE_MIN */
                if ( AV(x, y) < sensor_algorithm_config->activity_variable_min )
                {
                    AV(x, y) = sensor_algorithm_config->activity_variable_min;
                }
            }
        }
    }
}
