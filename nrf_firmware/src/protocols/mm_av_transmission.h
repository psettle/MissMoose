/*
file: mm_av_transmission.h
brief: Methods for transmitting activity variable data to the monitoring application over ANT
notes:
*/
#ifndef MM_AV_TRANSMISSION_H
#define MM_AV_TRANSMISSION_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdint.h>

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
    Broadcast latest activity variable state over ANT.
*/
void mm_av_transmission_send_av_update
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    float av_value,
    uint8_t av_status
    );

#endif /* MM_AV_TRANSMISSION_H */