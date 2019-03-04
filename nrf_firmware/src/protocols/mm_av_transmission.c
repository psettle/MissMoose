/**
file: mm_led_control.c
brief: Methods for controlling LEDs over blaze
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_av_transmission.h"
#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       VARIABLES
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Initialize activity variable state transmission over ANT.
*/
void mm_av_transmission_init(void)
{
    /**
     * TODO: Actual implementation! Adding this for
     * now for use in the testing framework. To be fully implemented
     * later for sending the AV values to the monitoring application.
     */
}

/**
 * Broadcast all activity variable state over ANT.
 */
void mm_av_transmission_send_all_avs(void)
{
    for (uint8_t x = 0; x < MAX_AV_SIZE_X; ++x)
    {
        for (uint8_t y = 0; y < MAX_AV_SIZE_Y; ++y)
        {
            /* Get the region status for AV transmission... */
            activity_variable_state_t av_status = mm_get_status_for_av(&AV(x, y));

            /* Broadcast raw AV values to monitoring application over ANT. */
            mm_av_transmission_send_av_update(x, y, AV(x, y), av_status);
        }
    }
}

/**
    Broadcast latest activity variable state over ANT.
*/
void mm_av_transmission_send_av_update
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    mm_activity_variable_t av_value,
    activity_variable_state_t av_status
    ) 
{
    /**
     * TODO: Actual implementation! Adding this for
     * now for use in the testing framework. To be fully implemented
     * later for sending the AV values to the monitoring application.
     */
}
