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
//For memset
#include <string.h>
//APP_ERROR_CHECK
#include "app_error.h"
//For MM_GATEWAY_ID
#include "mm_blaze_static_config.h"
//For testing sending LED messages (Needed for button control)
#include "bsp.h"
#include "nrf_drv_gpiote.h"

/**********************************************************
                        CONSTANTS
**********************************************************/
#define LED_ON_TICK_MS                          (500) //How long LEDs remain on while blinking
#define LED_OFF_TICK_MS                         (500) //How long LEDs remain off while blinking

#define PAGE_NUM_INDEX                          (0)
#define LED_FUNCTION_INDEX                      (1)
#define LED_COLOUR_INDEX                        (2)

#define BLAZE_LED_STATUS_TRANSMISSION_PAGE_NUM  (0x03)

//Used to test sending LED control messages from the gateway to the node by pressing buttons
#ifdef MM_BLAZE_GATEWAY
#define TEST_LED_MESSAGES                       (True)
#define TEST_LED_MESSAGES_COLOUR_CONTROL_PIN    (BSP_BUTTON_0)
#define TEST_LED_MESSAGES_LEDS_ON_PIN           (BSP_BUTTON_2)
#define TEST_LED_MESSAGES_LEDS_OFF_PIN          (BSP_BUTTON_3)
#endif
/**********************************************************
                          ENUMS
**********************************************************/


/**********************************************************
                       DECLARATIONS
**********************************************************/
#ifndef MM_BLAZE_GATEWAY
static void blaze_rx_handler( ant_blaze_message_t msg );
#endif //MM_BLAZE_GATEWAY
static void update_led_settings(led_function_t led_function, led_colours_t led_colour);
#ifdef TEST_LED_MESSAGES
static void test_led_messages_init_buttons(void);
static void led_test_button_press_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
#endif //TEST_LED_MESSAGES

/**********************************************************
                       VARIABLES
**********************************************************/
static hardware_config_t configuration;
#ifdef TEST_LED_MESSAGES
static led_colours_t current_test_led_colour = LED_COLOURS_YELLOW;
#endif

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

#endif // #ifndef MM_BLAZE_GATEWAY


#ifdef TEST_LED_MESSAGES
    test_led_messages_init_buttons();
#endif // #ifdef #ifdef TEST_LED_MESSAGES


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

	payload[PAGE_NUM_INDEX] = BLAZE_LED_STATUS_TRANSMISSION_PAGE_NUM;
	payload[LED_FUNCTION_INDEX] = led_function;
    payload[LED_COLOUR_INDEX] = led_colour;

    memset( &payload[3], 0xFF, 2); //Set bytes 3-4 to 0xFF

    blaze_message.p_data = &payload[0];

    mm_blaze_send_message( &blaze_message );

}

#endif


#ifndef MM_BLAZE_GATEWAY
//Respond to LED status transmission messages, do nothing otherwise. 
static void blaze_rx_handler( ant_blaze_message_t msg )
{
    uint8_t const * payload = msg.p_data;

    if(payload[PAGE_NUM_INDEX] == BLAZE_LED_STATUS_TRANSMISSION_PAGE_NUM){
        led_function_t led_function = payload[LED_FUNCTION_INDEX];
        led_colours_t led_colour = payload[LED_COLOUR_INDEX];
        update_led_settings(led_function, led_colour);
    }

}
#endif

static void update_led_settings(led_function_t led_function, led_colours_t led_colour)
{
    //It's an error if this node does not have LEDs
    APP_ERROR_CHECK(configuration != HARDWARE_CONFIG_PIR_LIDAR_LED);

    // leds_blinking, //LEDs will blink on and off
    // leds_on_continuously //LEDs will remain on
    //mm_rgb_led_set_colour
    uint32_t colour_hex_code = 0x00;
    switch (led_colour)
    {
        case LED_COLOURS_YELLOW:
            colour_hex_code = MM_RGB_COLOUR_YELLOW;
            break;
        case LED_COLOURS_RED:
            colour_hex_code = MM_RGB_COLOUR_RED;
            break;
        default:
            APP_ERROR_CHECK(true);
            break;
    }

    switch (led_function)
    {
        case LED_FUNCTION_LEDS_OFF:
            mm_rgb_set_on_off_cycle(0, LED_OFF_TICK_MS); //Set on_ticks to 0 - LED is never on
            break;
        case LED_FUNCTION_LEDS_BLINKING:
            mm_rgb_led_set_colour(colour_hex_code);
            mm_rgb_set_on_off_cycle(LED_ON_TICK_MS, LED_OFF_TICK_MS);
            break;
        case LED_FUNCTION_LEDS_ON_CONTINUOUSLY:
            mm_rgb_led_set_colour(colour_hex_code);
            mm_rgb_set_on_off_cycle(LED_ON_TICK_MS, 0); //Set off_ticks to 0 - LED is never on
            break;
        default:
            APP_ERROR_CHECK(true);
            break;
    }
    
}


#ifdef TEST_LED_MESSAGES

static void test_led_messages_init_buttons(void)
{
    //Configure control buttons
    uint32_t err_code;

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    //Could make this into a loop by putting the pin values into an array?

    //Configure TEST_LED_MESSAGES_COLOUR_CONTROL_PIN button
    err_code = nrf_drv_gpiote_in_init
        (
            TEST_LED_MESSAGES_COLOUR_CONTROL_PIN, 
            &in_config, 
            led_test_button_press_handler
        );
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(TEST_LED_MESSAGES_COLOUR_CONTROL_PIN, true);

    //Configure TEST_LED_MESSAGES_LEDS_ON_PIN button
    err_code = nrf_drv_gpiote_in_init
        (
            TEST_LED_MESSAGES_LEDS_ON_PIN, 
            &in_config, 
            led_test_button_press_handler
        );
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(TEST_LED_MESSAGES_LEDS_ON_PIN, true);

    //Configure TEST_LED_MESSAGES_LEDS_OFF_PIN button
    err_code = nrf_drv_gpiote_in_init
        (
            TEST_LED_MESSAGES_LEDS_OFF_PIN, 
            &in_config, 
            led_test_button_press_handler
        );
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(TEST_LED_MESSAGES_LEDS_OFF_PIN, true);

}

static void led_test_button_press_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    /* Check that the input button is actually set. */
    if(!nrf_drv_gpiote_in_is_set(pin))
    {
    	return;
    }

    /* Differentiate between the three buttons
    *  TEST_LED_MESSAGES_COLOUR_CONTROL_PIN - Cycle between colours
    *  TEST_LED_MESSAGES_LEDS_ON_PIN - Send a message to turn LEDs on
    *  TEST_LED_MESSAGES_LEDS_OFF_PIN - Send a message to turn LEDs off
    */
   switch(pin){
    case TEST_LED_MESSAGES_COLOUR_CONTROL_PIN:
        if(current_test_led_colour == LED_COLOURS_RED)
        {
            current_test_led_colour = LED_COLOURS_YELLOW;
        }
        else
        {
            current_test_led_colour = LED_COLOURS_RED;
        }
        break;
    case TEST_LED_MESSAGES_LEDS_ON_PIN:
        //Note that the target node will currently (Jan 24, 2019) be hardcoded to be node 1
        mm_led_control_update_node_leds(1, LED_FUNCTION_LEDS_BLINKING, current_test_led_colour);
        break;
    case TEST_LED_MESSAGES_LEDS_OFF_PIN:
        mm_led_control_update_node_leds(1, LED_FUNCTION_LEDS_OFF, current_test_led_colour);
        break;
    default:
        APP_ERROR_CHECK(true);
   }
}

#endif // #ifdef TEST_LED_MESSAGES



