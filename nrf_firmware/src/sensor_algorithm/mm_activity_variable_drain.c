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
                       DECLARATIONS
**********************************************************/

/**
    Applies the activity variable drain factor to all activity variables.
*/
void mm_apply_activity_variable_drain_factor(void)
{
    for ( uint8_t x = 0; x < MAX_AV_SIZE_X; x++ )
    {
        for ( uint8_t y = 0; y < MAX_AV_SIZE_Y; y++ )
        {
            if ( AV(x, y) > mm_sensor_algorithm_config()->activity_variable_min )
            {
                AV(x, y) *= mm_sensor_algorithm_config()->activity_variable_decay_factor;

                /* Enforce ACTIVITY_VARIABLE_MIN */
                if ( AV(x, y) < mm_sensor_algorithm_config()->activity_variable_min )
                {
                    AV(x, y) = mm_sensor_algorithm_config()->activity_variable_min;
                }
            }
        }
    }
}
