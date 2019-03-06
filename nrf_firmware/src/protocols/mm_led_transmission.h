/*
file: mm_led_transmission.h
brief: Methods for transmitting led data to the monitoring application over ANT
notes:

Author: Elijah Pennoyer

*/
#ifndef MM_LED_TRANSMISSION_H
#define MM_LED_TRANSMISSION_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdint.h>
#include <mm_led_control.h> // For enums

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
    Initialize LED state transmission over ANT.
*/
void mm_led_transmission_init(void);

/**
    Broadcast latest LED state over ANT.
*/
void mm_led_transmission_send_led_update
    (
    uint16_t node_id,
    led_function_t current_led_function,
    led_colours_t current_led_colour
    );

#endif // MM_LED_TRANSMISSION_H