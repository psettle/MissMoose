/**********************************************************
                        INCLUDES
**********************************************************/

#include "test_output_logger.hpp"
#include <stdlib.h>

extern "C" {
#include "mm_av_transmission.h"
#include "mm_sensor_algorithm_config.h"
}

/**********************************************************
                       VARIABLES
**********************************************************/

static mm_activity_variable_t ** av_cache;


/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Writes AV output information to the opened log file.
 * For example, the output would look like:
 *
 * <timestamp>,AV OUTPUT EVENT,x,1,y,0,val,1.04,region,2
 */
static void log_av_ouput
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    mm_activity_variable_t av_value,
    activity_variable_state_t av_status
    );

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * Initialize activity variable state transmission over ANT.
 */
void mm_av_transmission_init(void)
{
	av_cache = (mm_activity_variable_t **) malloc(mm_get_max_av_size_x() * sizeof(mm_activity_variable_t *));
	for (uint8_t i = 0; i < mm_get_max_av_size_x(); i++)
	{
		av_cache[i] = (mm_activity_variable_t *) malloc(mm_get_max_av_size_y() * sizeof(mm_activity_variable_t));
	}

	memset(av_cache, 0, sizeof(av_cache));
}

/**
 * Broadcast latest activity variable state over ANT.
 */
void mm_av_transmission_send_av_update
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    mm_activity_variable_t av_value,
    activity_variable_state_t av_status
    )
{
    /* Log the event details. */
    log_av_ouput(av_position_x, av_position_y, av_value, av_status);
}

/**
 * Broadcast all activity variable state over ANT.
 */
void mm_av_transmission_send_all_avs(void)
{
    bool send_update = false;

    for (uint8_t x = 0; x < mm_get_max_av_size_x(); ++x)
    {
        for (uint8_t y = 0; y < mm_get_max_av_size_y(); ++y)
        {
            if (AV(x, y) != av_cache[x][y])
            {
                av_cache[x][y] = AV(x, y);

                /* Get the region status for AV transmission... */
                activity_variable_state_t av_status = mm_get_status_for_av(&AV(x, y));

                /* Broadcast raw AV values to monitoring application over ANT. */
                mm_av_transmission_send_av_update(x, y, AV(x, y), av_status);
            }
        }
    }
}

/**
 * Writes AV output information to the opened log file.
 * For example, the output would look like:
 * 
 * <timestamp>,AV OUTPUT EVENT,x,1,y,0,val,1.04,region,2
 */
static void log_av_ouput
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    mm_activity_variable_t av_value,
    activity_variable_state_t av_status
    )
{
    /* Output would be like the above example. */
    log_message("AV OUTPUT EVENT,x," + std::to_string(av_position_x) + ",y," + std::to_string(av_position_y) + ",val," + std::to_string(av_value) + ",region," + std::to_string(av_status));
}