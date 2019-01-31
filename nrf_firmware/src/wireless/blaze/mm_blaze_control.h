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
                        TYPES
**********************************************************/

typedef ant_blaze_rx_message_handler_t mm_blaze_message_handler_t;

/**
 * Type for serializing blaze message
 * (So then can be saved and processed later)
 */
typedef struct
{
    /* data contains message payload, p_data is an invalid pointer because message has been serialized. */
    ant_blaze_message_t message;    
    uint8_t             data[ANT_BLAZE_MAX_MESSAGE_LENGTH];
} mm_blaze_message_serialized_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_blaze_init(uint16_t assigned_node_id, uint16_t assigned_network_id);

/* Messages may arrive in interrupt context, use mm_blaze_message_serialized_t to kick to main. */
void mm_blaze_register_message_listener(mm_blaze_message_handler_t rx_handler);

/* Send a message through blaze */
void mm_blaze_send_message(ant_blaze_message_t * message);

/* Pack a blaze message into a serializable format */
void mm_blaze_pack_message(ant_blaze_message_t const * unpacked, mm_blaze_message_serialized_t* packed);

#endif /* MM_BLAZE_CONTROL_H */
