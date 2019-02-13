/**
file: mm_led_control.c
brief: Methods for controlling LEDs over blaze
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_av_transmission.h"

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
    Broadcast latest activity variable state over ANT.
*/
void mm_av_transmission_send_av_update
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    float av_value,
    uint8_t av_status
    ) 
{
    /**
     * TODO: Actual implementation! Adding this for
     * now for use in the testing framework. To be fully implemented
     * later for sending the AV values to the monitoring application.
     */
}
