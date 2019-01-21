/**
file: mm_led_control.c
brief: Methods for controlling LEDs over blaze
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_led_control.h"
//For hardware configuration, determining if an LED is present on this node
#include "mm_switch_config.h" 
//Blaze functions - For registering listeners and sending messages over blaze
#include "mm_blaze_control.h"
//LED control functions
#include "mm_rgb_led_pub.h"
//For memset?
#include <string.h>

/**********************************************************
                        CONSTANTS
**********************************************************/
static const uint16_t led_on_tick_ms = 500; //How long LEDs remain on while blinking
static const uint16_t led_off_tick_ms = 500; //How long LEDs remain off while blinking

#define PAGE_NUM_INDEX                          0
#define LED_FUNCTION_INDEX                      1
#define LED_COLOUR_INDEX                        2

#define BLAZE_LED_STATUS_TRANSMISSION_PAGE_NUM  0x03

/**********************************************************
                          ENUMS
**********************************************************/


/**********************************************************
                       DECLARATIONS
**********************************************************/
static void blaze_rx_handler( ant_blaze_message_t msg );
static void update_led_settings(led_function_t led_function, led_colours_t led_colour);

/**********************************************************
                       VARIABLES
**********************************************************/
static hardware_config_t configuration;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_led_control_init(void)
{
    //Find if this node has LEDs based on the hardware configuration
    configuration = read_hardware_config();

#ifndef MM_BLAZE_GATEWAY
    //If this node has LEDs and is not the gateway, register a blaze listener to handle incoming LED pages.
    if(configuration == HARDWARE_CONFIG_PIR_LIDAR_LED)
    {
        mm_blaze_register_message_listener(blaze_rx_handler);
    }
#endif
}

#ifdef MM_BLAZE_GATEWAY

void mm_led_control_update_node_leds
    (
        uint16_t target_node_id,
        led_function_t led_function,
        led_colours_t led_colour
    )
{
    //If the node the message is for is actually the gateway (i.e. the current node), just update the LEDs.
    if(target_node_id == MM_GATEWAY_ID){ 
        update_led_settings(led_function, led_colour);
        return;
    }
    //Otherwise - Create and send a blaze message to the specified node.
    ant_blaze_message_t blaze_message;
	memset( &blaze_message, 0, sizeof( blaze_message ) );

	blaze_message.address = target_node_id;
	blaze_message.index = 0; //Always 0, used internally by blaze code
	blaze_message.length = 5; //Payload length

    uint8_t payload [5];
	memset( &payload[0], 0, sizeof( payload ) );

	payload[PAGE_NUM_INDEX] = BLAZE_LIDAR_DATA_PAGE_NUMBER;
	payload[LED_FUNCTION_INDEX] = led_function;
    payload[LED_COLOUR_INDEX] = led_colour;

    memset( &payload[3], 0xFF, 2); //Set bytes 3-4 to 0xFF

    blaze_message.p_data = &payload[0];

    mm_blaze_send_message( &blaze_message );

}

#endif



//Respond to LED status transmission messages, do nothing otherwise. 
static void blaze_rx_handler( ant_blaze_message_t msg )
{
    uint8_t const * payload = msg.p_data;

    if(payload[0] == BLAZE_LED_STATUS_TRANSMISSION_PAGE_NUM){
        led_function_t led_function = payload[LED_FUNCTION_INDEX];
        led_colours_t led_colour = payload[LED_COLOUR_INDEX];
        update_led_settings(led_function, led_colour);
    }

}

static void update_led_settings(led_function_t led_function, led_colours_t led_colour)
{
    //If this node does not have LEDs, throw an error
    if(configuration != HARDWARE_CONFIG_PIR_LIDAR_LED)
    {
        APP_ERROR_CHECK(true);
    }


    // leds_blinking, //LEDs will blink on and off
    // leds_on_continuously //LEDs will remain on
    //mm_rgb_led_set_colour
    uint32_t colour_hex_code = 0x00;
    switch (led_colour)
    {
        case led_colour_yellow:
            colour_hex_code = MM_RGB_COLOUR_YELLOW;
            break;
        case led_colour_red:
            colour_hex_code = MM_RGB_COLOUR_RED;
            break;
        default:
            APP_ERROR_CHECK(true);
            break;
    }

    switch (led_function)
    {
        case leds_off:
            mm_rgb_set_on_off_cycle(0, led_off_tick_ms); //Set on_ticks to 0 - LED is never on
            break;
        case leds_blinking:
            mm_rgb_led_set_colour(colour_hex_code);
            mm_rgb_set_on_off_cycle(led_on_tick_ms, led_off_tick_ms);
            break;
        case leds_on_continuously:
            mm_rgb_led_set_colour(colour_hex_code);
            mm_rgb_set_on_off_cycle(led_on_tick_ms, 0); //Set off_ticks to 0 - LED is never on
            break;
        default:
            APP_ERROR_CHECK(true);
            break;
    }
    
}

