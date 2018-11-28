/**
file: mm_blaze_control.h
brief:
notes:
*/

#ifndef MM_BLAZE_CONTROL_H
#define MM_BLAZE_CONTROL_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "ant_blaze_defines.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

typedef ant_blaze_rx_message_handler_t mm_blaze_message_handler_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_blaze_init(void);

void mm_blaze_register_message_listener(mm_blaze_message_handler_t rx_handler);

/* Send a message through blaze */
void mm_blaze_send_message(ant_blaze_message_t * message);

#endif /* MM_BLAZE_CONTROL_H */
