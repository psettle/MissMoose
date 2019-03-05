/**
file: mm_led_control.c
brief: Methods for controlling LEDs over blaze
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

//For memset
#include <string.h>

//APP_ERROR_CHECK
#include "app_error.h"
//to task delagation to main
#include "app_scheduler.h"
//For testing sending LED messages (Needed for button control)
#include "bsp.h"
#include "nrf_drv_gpiote.h"

#include "mm_led_control.h"
//For hardware configuration, determining if an LED is present on this node
#include "mm_switch_config.h" 
//Blaze functions - For registering listeners and sending messages over blaze
#include "mm_blaze_control.h"
//LED control functions
#include "mm_rgb_led_pub.h"
//For MM_GATEWAY_ID
#include "mm_blaze_static_config.h"
//To transmit LED state information to the monitoring application
#include "mm_led_transmission.h"


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
    #define TEST_LED_MESSAGES                       (false)
    #define TEST_LED_MESSAGES_COLOUR_CONTROL_PIN    (BSP_BUTTON_0) //Button A
    #define TEST_LED_MESSAGES_LEDS_ON_OFF_PIN       (BSP_BUTTON_2) //Button C
#else
    #define TEST_LED_MESSAGES                       (false) //Always false if not a gateway
#endif
/**********************************************************
                          ENUMS
**********************************************************/


/**********************************************************
                       DECLARATIONS
**********************************************************/
#ifndef MM_BLAZE_GATEWAY
    /**
     * Interrupt handler for blaze rx events.
     *
     * Filter events and pass to main if relevant.
     */
    static void blaze_rx_handler(ant_blaze_message_t msg);

    /**
     * Main handler for updates to led output.
     */
    static void on_led_control_evt(void* p_data, uint16_t size);
#endif /* MM_BLAZE_GATEWAY */

    /**
     * Maps blaze API (function, colour) to driver API (colour, t_on, t_off).
     */
    static void update_led_settings(led_function_t led_function, led_colours_t led_colour);

#if TEST_LED_MESSAGES
    /**
     * Initialize buttons for mocking messages from the gateway.
     */
    static void test_led_messages_init_buttons(void);

    /**
     * Interrupt handler for test button events.
     */
    static void led_test_button_press_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

    /**
     * Main hander for button events. Only called if button was pressed down.
     *
     * p_data -> nrf_drv_gpiote_pin_t*
     */
    static void on_button_event(void* p_data, uint16_t size);
#endif //TEST_LED_MESSAGES

/**********************************************************
                       VARIABLES
**********************************************************/
static hardware_config_t configuration;

#if TEST_LED_MESSAGES
    static led_colours_t test_led_msgs_current_colour = LED_COLOURS_YELLOW;
    static bool test_led_msgs_currently_transmitting = false;
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


#if TEST_LED_MESSAGES
    test_led_messages_init_buttons();
#endif // #if TEST_LED_MESSAGES
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
    if(target_node_id == MM_GATEWAY_ID)
    {
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

    // Tell the monitoring application that the LED has been updated
    mm_led_transmission_send_led_update(target_node_id, led_function, led_colour);
}
#endif

#ifndef MM_BLAZE_GATEWAY
static void blaze_rx_handler(ant_blaze_message_t msg)
{
    /* Filter message for types we care about. */
    switch(msg.p_data[PAGE_NUM_INDEX])
    {
        case BLAZE_LED_STATUS_TRANSMISSION_PAGE_NUM:
            break;
        default:
            return;
    }

    /* Pack message to serializable format. */
    mm_blaze_message_serialized_t evt;
    mm_blaze_pack_message(&msg, &evt);

    /* Kick event to main */
    uint32_t err_code;
    err_code = app_sched_event_put(&evt, sizeof(evt), on_led_control_evt);
    APP_ERROR_CHECK(err_code);
}

static void on_led_control_evt(void* p_data, uint16_t size)
{
    mm_blaze_message_serialized_t const * msg = (mm_blaze_message_serialized_t const *)p_data;
    uint8_t const * payload = &msg->data[0];

    /* Message is pre-filtered, so it's always a control message */
    APP_ERROR_CHECK(payload[PAGE_NUM_INDEX] != BLAZE_LED_STATUS_TRANSMISSION_PAGE_NUM);

    led_function_t led_function = payload[LED_FUNCTION_INDEX];
    led_colours_t  led_colour   = payload[LED_COLOUR_INDEX];
    update_led_settings(led_function, led_colour);
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

#if TEST_LED_MESSAGES

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

    //Configure TEST_LED_MESSAGES_LEDS_ON_OFF_PIN button
    err_code = nrf_drv_gpiote_in_init
        (
            TEST_LED_MESSAGES_LEDS_ON_OFF_PIN, 
            &in_config, 
            led_test_button_press_handler
        );
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(TEST_LED_MESSAGES_LEDS_ON_OFF_PIN, true);

}

static void led_test_button_press_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    /* Check that the input button is actually set. */
    if(!nrf_drv_gpiote_in_is_set(pin))
    {
        return;
    }

    /* Kick event to main */
    uint32_t err_code;
    err_code = app_sched_event_put(&pin, sizeof(pin), on_button_event);
    APP_ERROR_CHECK(err_code);
}

static void on_button_event(void* p_data, uint16_t size)
{
    nrf_drv_gpiote_pin_t const * pin = (nrf_drv_gpiote_pin_t const *)p_data;

    /* Differentiate between the the buttons
     * TEST_LED_MESSAGES_COLOUR_CONTROL_PIN - Cycle between colours
     * TEST_LED_MESSAGES_LEDS_ON_OFF_PIN - Cycle between LEDs on and LEDs off
     */
    switch(*pin)
    {
        case TEST_LED_MESSAGES_COLOUR_CONTROL_PIN:
            if(test_led_msgs_current_colour == LED_COLOURS_RED)
            {
                test_led_msgs_current_colour = LED_COLOURS_YELLOW;
            }
            else
            {
                test_led_msgs_current_colour = LED_COLOURS_RED;
            }
            break;
        case TEST_LED_MESSAGES_LEDS_ON_OFF_PIN:
            /* Node is hardcoded to 1 because that is the first node id to be configured
                and having this module use the position system for testing is a complicated test setup. */
            if(test_led_msgs_currently_transmitting)
            {
                mm_led_control_update_node_leds(1, LED_FUNCTION_LEDS_OFF, test_led_msgs_current_colour);
                test_led_msgs_currently_transmitting = false;
            }
            else
            {
                mm_led_control_update_node_leds(1, LED_FUNCTION_LEDS_BLINKING, test_led_msgs_current_colour);
                test_led_msgs_currently_transmitting = true;
            }
            break;
        default:
            /* Unexpected pin. */
            APP_ERROR_CHECK(true);
    }
}

#endif // #if TEST_LED_MESSAGES



