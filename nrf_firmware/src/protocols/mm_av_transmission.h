/*
file: mm_av_transmission.h
brief: Methods for controlling LEDs over blaze
notes:
*/
#ifndef MM_AV_TRANSMISSION_H
#define MM_AV_TRANSMISSION_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdint.h>
#include "mm_activity_variables.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
    Initialize activity variable state transmission over ANT.
*/
void mm_av_transmission_init(void);

/**
 * Broadcast all activity variable state over ANT.
 */
void mm_av_transmission_send_all_avs(void);

/**
    Broadcast latest activity variable state over ANT.
*/
void mm_av_transmission_send_av_update
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    mm_activity_variable_t av_value,
    activity_variable_state_t av_status
    );

#endif /* MM_AV_TRANSMISSION_H */
