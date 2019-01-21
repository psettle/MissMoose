/*
file: mm_led_control.h
brief: Methods for controlling LEDs over blaze
notes:
*/
#ifndef MM_LED_CONTROL_H
#define MM_LED_CONTROL_H


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
//Possible LED colours
typedef enum
{
    led_colour_yellow,
    led_colour_red
} led_colours_t;

//How the LEDs should behave
typedef enum
{
    leds_off,
    leds_blinking, //LEDs will blink on and off
    leds_on_continuously //LEDs will remain on

} led_function_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/
void mm_led_control_init(void);

void mm_led_control_update_node_leds
    (
        uint16_t target_node_id,
        led_function_t led_function,
        led_colours_t led_colour
    );


#endif