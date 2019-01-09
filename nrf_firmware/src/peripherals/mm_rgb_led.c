/**
file: mm_rgb_led.c
brief: Interface for controling an RGB LED strip
notes:
*/


/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdio.h>
#include "mm_rgb_led_pub.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "ir_led_transmit_pub.h"
#include "low_power_pwm.h"
#include "app_timer.h"
#include "mm_power_bank_timer_pub.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define RGB_LED_RED    27 ///< J102.18
#define RGB_LED_GREEN  13 ///< J102.17
#define RGB_LED_BLUE   28 ///< J102.09

#define RGB_LED_RED_MASK    (1<<RGB_LED_RED)
#define RGB_LED_GREEN_MASK  (1<<RGB_LED_GREEN)
#define RGB_LED_BLUE_MASK   (1<<RGB_LED_BLUE)

#define BUTTON_CONTROL   true           ///< Allow using the on-board buttons to change the LED colour and on-off cycle
#define LED_DEBUG        true
#define LED_STARTUP_TEST true           ///< Whether or not to have a brief period of 5s on startup with the LEDs set to WHTE.

#define LED_DEFAULT_ON_DURATION  0      ///< In ms. In other words, off.
#define LED_DEFAULT_OFF_DURATION 1000   ///< In ms. How long to set the LEDs to be off on startup.

/* For two reasons it's helpful to run an LED test on startup!
   The first is that setting all the LEDs to white for 5 seconds is an easy,
   visual way to test the R, G, and B, components of each LED, brightness,
   wiring, and the battery pack.
   The second reason is that The battery pack is strange and likes to have a
   high current draw on startup, before it lets itself be kept on with a much
   smaller current draw. */
#if LED_STARTUP_TEST
    #define LED_STARTUP_DURATION 5000   ///< in ms
    #define LED_STARTUP_COLOUR   MM_RGB_COLOUR_WHITE
#endif
/**********************************************************
                           TYPES
**********************************************************/

typedef enum
{
    LEDS_OFF,   ///< The LEDs are currently set to be off.
    LEDS_ON,    ///< The LEDs are currently set to be on.
} power_cycle_state_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void rgb_led_pwm_handler(void * p_context);
static void rgb_led_power_cycle_timer_handler(void * p_context);
static void rgb_led_apply_colour(uint32_t colour_hex);
static void rgb_led_apply_colour_individual(uint8_t red_duty_cycle, uint8_t green_duty_cycle, uint8_t blue_duty_cycle);

#if BUTTON_CONTROL
    static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
#endif

/**********************************************************
                       VARIABLES
**********************************************************/
APP_TIMER_DEF(power_cycle_timer_id);

static uint32_t current_colour;         /* The current colour being displayed on the LEDs. */
static uint16_t on_cycle_length_ms;     /* The amount of time the LEDs should stay on for in a big on-off cycle */
static uint16_t off_cycle_length_ms;    /* The amount of time the LEDs should stay off for in a big on-off cycle */
static power_cycle_state_t on_off_cycle_state; /* Keeps track of whether the LEDs are set to be on or off.
                                                  When the colour is changed, the LEDs are immediately set to be
                                                  on, and the cycle is reset. */
static bool control_power_bank;         /* Whether or not we want to allow current to periodically be drawn
                                           out from the power bank to keep it on. */
static bool led_startup_test_done;      /* flag for whether the 5s startup test has passed. */

static low_power_pwm_t low_power_pwm_red;
static low_power_pwm_t low_power_pwm_green;
static low_power_pwm_t low_power_pwm_blue;

#if BUTTON_CONTROL
    static uint32_t colour_cycle[9] = {MM_RGB_COLOUR_RED, 0xFF8800, 0x88FF00, MM_RGB_COLOUR_GREEN, 0x00FF88, 0x0088FF, MM_RGB_COLOUR_BLUE, 0x8800FF, 0xFF0088};
    static uint8_t  colour_cycle_index = 0;

    /* Some sets of numbers that make for an interesting demonstration */
    static uint16_t on_cycles[5] =  {0, 500, 1000, 1000, 1000};
    static uint16_t off_cycles[5] = {1000, 500, 2000, 3000, 0};
    static uint8_t  on_off_cycle_index = 0;
#endif
/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * @brief sets the desired colour of the LEDs
 * @param[in] colour_hex    32-bit hex code of the colour to set the LEDs to.
 *                          The most significant 8 bits are unused, so it's really more like
 *                          a 24-bit value: 0xRRGGBB
 */
void mm_rgb_led_set_colour(uint32_t colour_hex)
{
    current_colour = colour_hex;
    /* Apply the desired colour right away, if the LEDs are supposed to be on. */
    /* If the LEDs are off, keep them off and let the timers handle the colour change. */
    if(on_off_cycle_state == LEDS_ON)
    {
        rgb_led_apply_colour(colour_hex);
    }
    /* This is a public function; stop goofing around and let the user do what they want. */
    led_startup_test_done = true;
}

/**
 * @brief sets the desired colour of the LEDs, with separate values for colour.
 * @param[in] red_colour    8-bit Value for the intensity of the red LED.
 * @param[in] green_colour  8-bit Value for the intensity of the green LED.
 * @param[in] blue_colour   8-bit Value for the intensity of the blue LED.
 */
void mm_rgb_led_set_colour_individual(uint8_t red_colour, uint8_t green_colour, uint8_t blue_colour)
{
    uint32_t colour = 0;
    colour |= (red_colour & 0xFF)   << 16;
    colour |= (green_colour & 0xFF) << 8;
    colour |= (blue_colour & 0xFF)  << 0;
    mm_rgb_led_set_colour(colour);
    /* This is a public function; stop goofing around and let the user do what they want. */
    led_startup_test_done = true;
}

/**
 * @brief Actually applies the current colour of the LEDs
 * @details This resets the current on-off power cycle of the LED strip, and instantly turns the LEDs
 *          on at the desired colour.
 */
static void rgb_led_apply_colour(uint32_t colour_hex)
{
    uint8_t red   = (colour_hex >> 16) & 0xFF;
    uint8_t green = (colour_hex >> 8)  & 0xFF;
    uint8_t blue  = (colour_hex >> 0)  & 0xFF;
    rgb_led_apply_colour_individual(red, green, blue);
}

/**
 * @brief Actually applies the current colour of the LEDs, with separate values for colour.
 * @param[in] red_duty_cycle    8-bit Value for the intensity of the red LED.
 * @param[in] green_duty_cycle  8-bit Value for the intensity of the green LED.
 * @param[in] blue_duty_cycle   8-bit Value for the intensity of the blue LED.
 */
static void rgb_led_apply_colour_individual(uint8_t red_duty_cycle, uint8_t green_duty_cycle, uint8_t blue_duty_cycle)
{
    uint32_t err_code;
    err_code = low_power_pwm_duty_set(&low_power_pwm_red, red_duty_cycle);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_duty_set(&low_power_pwm_green, green_duty_cycle);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_duty_set(&low_power_pwm_blue, blue_duty_cycle);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Sets the amount of time the whole strip should be on, and off.
 * @param[in] on_ticks_ms   Number of ms that the LED should stay on for.
 *                          NOTE that if this value is 0, the LED will just stay off.
 * @param[in] off_ticks_ms  Number of ms that the LED should stay off for.
 *                          NOTE that if this value is 0, the LED will just stay on.
 */
void mm_rgb_set_on_off_cycle(uint16_t on_ticks_ms, uint16_t off_ticks_ms)
{
    uint32_t err_code;
    on_cycle_length_ms = on_ticks_ms;
    off_cycle_length_ms = off_ticks_ms;
    err_code = app_timer_stop(power_cycle_timer_id);
    APP_ERROR_CHECK(err_code);
    if(on_off_cycle_state == LEDS_ON)
    {
        // If we're in the on state, set the timer to change back to the off state,
        // as long as the off state has an actual duration
    	if(off_cycle_length_ms > 0)
    	{
    		err_code = app_timer_start(power_cycle_timer_id, APP_TIMER_TICKS(off_cycle_length_ms), NULL);
    	}
    }
    else
    {
        // If we're in the off state, set the timer to change back to the on state,
        // as long as the on state has an actual duration
    	if(on_cycle_length_ms > 0)
    	{
    		err_code = app_timer_start(power_cycle_timer_id, APP_TIMER_TICKS(on_cycle_length_ms), NULL);
    	}
    }

    /* Power bank control */
    if(control_power_bank)
    {
        if(on_cycle_length_ms > 0)
        {
            /* If we're keeping the LEDs on, stop drawing extra current from the power bank. */
            mm_power_bank_set_on_off_cycle(0, POWER_BANK_DEFAULT_OFF_DURATION);
        }
        else
        {
            /* Otherwise, start periodically drawing current from the power bank again. */
            mm_power_bank_set_on_off_cycle(POWER_BANK_DEFAULT_ON_DURATION, POWER_BANK_DEFAULT_OFF_DURATION);
        }
    }
    APP_ERROR_CHECK(err_code);
    /* This is a public function; stop goofing around and let the user do what they want. */
    led_startup_test_done = true;
}

/**
 * @brief Function to be called in timer interrupt caused by the PWM timers for the LEDs
 *
 * @param[in] p_context     General purpose pointer (unused).
 */
static void rgb_led_pwm_handler(void * p_context)
{
    // Nothing to do at this time.
}

/**
 * @brief Function to be called in timer interrupts for controlling the power cycle of the LEDs
 *
 * @param[in] p_context     General purpose pointer (unused).
 */
static void rgb_led_power_cycle_timer_handler(void * p_context)
{
    uint32_t err_code;
    UNUSED_PARAMETER(p_context);
    #if LED_STARTUP_TEST
        if(!led_startup_test_done)
        {
            // The test is done, just turn the leds off and let them stay off.
            led_startup_test_done = true;
            current_colour = MM_RGB_LED_DUTY_0;
            rgb_led_apply_colour(current_colour);
            on_off_cycle_state = LEDS_OFF;
            mm_rgb_set_on_off_cycle(LED_DEFAULT_ON_DURATION, LED_DEFAULT_OFF_DURATION);
            return; // Return early in this case. we've already called set_on_off_cycle to adjust the timers.
        }
    #endif

    if(on_off_cycle_state == LEDS_ON)
    {
        // Since the off_cycle length has an actual duration, turn off the LEDs and set the timer.
        // Otherwise we want to just let the LEDs stay on until someone changes the times using mm_rgb_set_on_off_cycle.
        if(off_cycle_length_ms > 0)
        {
            rgb_led_apply_colour(MM_RGB_LED_DUTY_0);
            on_off_cycle_state = LEDS_OFF;
            err_code = app_timer_start(power_cycle_timer_id, APP_TIMER_TICKS(off_cycle_length_ms), NULL);
            APP_ERROR_CHECK(err_code);
        }
    }
    else if(on_off_cycle_state == LEDS_OFF)
    {
        // Since the on_cycle length has an actual duration, turn on the LEDs and set the timer.
        // Otherwise we want to just let the LEDs stay off until someone changes the times using mm_rgb_set_on_off_cycle.
        if(on_cycle_length_ms > 0)
        {
            rgb_led_apply_colour(current_colour);
            on_off_cycle_state = LEDS_ON;
            err_code = app_timer_start(power_cycle_timer_id, APP_TIMER_TICKS(on_cycle_length_ms), NULL);
            APP_ERROR_CHECK(err_code);
        }
    }
}

/**
 * @brief Function for initializing the RGB LED, utilizing the PWM library.
 *
 * @param[in] trick_power_bank  Whether or not to do some trickery to keep the power bank on.
 */
void mm_rgb_led_init(bool trick_power_bank)
{
    ret_code_t err_code;
    low_power_pwm_config_t low_power_pwm_config;
    control_power_bank = trick_power_bank;

    #if BUTTON_CONTROL
        /* Sense a push on the control pin. Pull up the pin,
         * since we expect to use an IO board button for this. */
        nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
        in_config.pull = NRF_GPIO_PIN_PULLUP;

        // One button for changing the colour
        err_code = nrf_drv_gpiote_in_init(BSP_BUTTON_0, &in_config, control_pin_handler);
        APP_ERROR_CHECK(err_code);
        nrf_drv_gpiote_in_event_enable(BSP_BUTTON_0, true);

        // One button for changing how often the leds are on/off.
        err_code = nrf_drv_gpiote_in_init(BSP_BUTTON_2, &in_config, control_pin_handler);
        APP_ERROR_CHECK(err_code);
        nrf_drv_gpiote_in_event_enable(BSP_BUTTON_2, true);

    #endif

    // Trick the power bank if requested
    if(control_power_bank)
    {
        mm_power_bank_init(true);
        mm_power_bank_set_on_off_cycle(POWER_BANK_DEFAULT_ON_DURATION, POWER_BANK_DEFAULT_OFF_DURATION);
    }

    /* Initialize and enable PWM - RED. */
    APP_TIMER_DEF(lpp_timer_red);
    low_power_pwm_config.active_high    = true;
    low_power_pwm_config.period         = LOW_POWER_PWM_CONFIG_PERIOD;
    low_power_pwm_config.bit_mask       = RGB_LED_RED_MASK;
    low_power_pwm_config.p_timer_id     = &lpp_timer_red;
    low_power_pwm_config.p_port         = NRF_GPIO;

    err_code = low_power_pwm_init((&low_power_pwm_red), &low_power_pwm_config, rgb_led_pwm_handler);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_duty_set(&low_power_pwm_red, MM_RGB_LED_DUTY_0);
    APP_ERROR_CHECK(err_code);

    /* Initialize and enable PWM - GREEN. */
    APP_TIMER_DEF(lpp_timer_green);
    low_power_pwm_config.active_high    = true;
    low_power_pwm_config.period         = LOW_POWER_PWM_CONFIG_PERIOD;
    low_power_pwm_config.bit_mask       = RGB_LED_GREEN_MASK;
    low_power_pwm_config.p_timer_id     = &lpp_timer_green;
    low_power_pwm_config.p_port         = NRF_GPIO;

    err_code = low_power_pwm_init((&low_power_pwm_green), &low_power_pwm_config, rgb_led_pwm_handler);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_duty_set(&low_power_pwm_green, MM_RGB_LED_DUTY_0);
    APP_ERROR_CHECK(err_code);

    /* Initialize and enable PWM - BLUE. */
    APP_TIMER_DEF(lpp_timer_blue);
    low_power_pwm_config.active_high    = true;
    low_power_pwm_config.period         = LOW_POWER_PWM_CONFIG_PERIOD;
    low_power_pwm_config.bit_mask       = RGB_LED_BLUE_MASK;
    low_power_pwm_config.p_timer_id     = &lpp_timer_blue;
    low_power_pwm_config.p_port         = NRF_GPIO;

    err_code = low_power_pwm_init((&low_power_pwm_blue), &low_power_pwm_config, rgb_led_pwm_handler);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_duty_set(&low_power_pwm_blue, MM_RGB_LED_DUTY_0);
    APP_ERROR_CHECK(err_code);

    /* Actually start all the pwm instances */
    err_code = low_power_pwm_start((&low_power_pwm_red), low_power_pwm_red.bit_mask);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_start((&low_power_pwm_green), low_power_pwm_green.bit_mask);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_start((&low_power_pwm_blue), low_power_pwm_blue.bit_mask);
    APP_ERROR_CHECK(err_code);

    // Create a timer for changing when the LEDs turn on and off
    err_code = app_timer_create(&power_cycle_timer_id, APP_TIMER_MODE_SINGLE_SHOT, rgb_led_power_cycle_timer_handler);
    APP_ERROR_CHECK(err_code);

    #if LED_STARTUP_TEST
        // Initially set the LEDs to be on.
        current_colour = LED_STARTUP_COLOUR;
        on_off_cycle_state = LEDS_ON;
        rgb_led_apply_colour(current_colour);
        mm_rgb_set_on_off_cycle(LED_STARTUP_DURATION, LED_STARTUP_DURATION);
        led_startup_test_done = false;
    #else
        led_startup_test_done = true;
        // Initially set the LEDs to be off.
        current_colour = MM_RGB_LED_DUTY_0;
        on_off_cycle_state = LEDS_OFF;
        mm_rgb_set_on_off_cycle(LED_DEFAULT_ON_DURATION, LED_DEFAULT_OFF_DURATION);
    #endif

}

#if BUTTON_CONTROL
    /**
     * @brief Function for manually changing the duty cycle of the red, blue, or green channel.
     *
     * @param[in] pin       The pin hooked up to the GPIOTE interrupt for the control pin.
     * @param[in] action    Polarity setting for the GPIOTE channel.
     */
    static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
    {
        switch(pin)
        {
            case BSP_BUTTON_0:
                // Just rotate through a set of predefined colours
                #if(LED_DEBUG)
                    bsp_board_led_invert(0);
                #endif
                colour_cycle_index = colour_cycle_index == 8 ? 0 : colour_cycle_index + 1;
                mm_rgb_led_set_colour(colour_cycle[colour_cycle_index]);
                break;
            case BSP_BUTTON_2:
                // Change how often the LED strip is actually on
                #if(LED_DEBUG)
                    bsp_board_led_invert(2);
                #endif
                on_off_cycle_index = on_off_cycle_index == 4 ? 0 : on_off_cycle_index + 1;
                mm_rgb_set_on_off_cycle(on_cycles[on_off_cycle_index], off_cycles[on_off_cycle_index]);
                break;
        }
    }
#endif

/**
 *@}
 **/
