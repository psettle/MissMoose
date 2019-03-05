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

#endif /* MM_AV_TRANSMISSION_H */
