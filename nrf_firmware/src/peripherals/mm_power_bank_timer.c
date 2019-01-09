/**
file: mm_power_bank_timer.c
brief: Pulse a gpio pin at a set time to keep the power bank on.
notes: The power bank we have chosen will turn off if
       there is a lower current draw for an extended period
       of time. In order to keep it on, a gpio pin is pulsed
       for a short time to control current flowing through a
       mosfet and small resistor.
       This small current draw should have a hopefully
       minimal impact on battery life.
*/


/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdio.h>
#include "app_timer.h"
#include "mm_power_bank_timer_pub.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define POWER_BANK_PIN 4 ///< J102.05
#define LED_DEBUG true

/**********************************************************
                           TYPES
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void set_current_draw(bool draw_current);
static void power_bank_timer_handler(void * p_context);
void mm_power_bank_disable(void);

/**********************************************************
                       VARIABLES
**********************************************************/
APP_TIMER_DEF(power_bank_timer_id);

static uint32_t pin_active_high = true;     /* Whether we treat the pin as active high or active low. Active high is typical. */
static uint16_t on_length_ms;           /* The amount of time the LEDs should stay on for in a big on-off cycle */
static uint16_t off_length_ms;          /* The amount of time the LEDs should stay off for in a big on-off cycle */
static bool     current_draw_state;     /* Keeps track of whether we're in the state where we draw current or not. */

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * @brief Performs some initial setup like creating timers and configuring gpios
 *
 * @param[in] active_high   Whether the pin used to control current draw from the power
 *                          bank is active high or active low.
 */
void mm_power_bank_init(bool active_high)
{
    uint32_t err_code;
    pin_active_high = active_high;
    // Variable to keep track of whether we've initialized, so we don't create the timer twice!
    static bool initialized = false;

    if(!initialized)
    {
        // Create a timer for changing when the LEDs turn on and off
        err_code = app_timer_create(&power_bank_timer_id, APP_TIMER_MODE_SINGLE_SHOT, power_bank_timer_handler);
        APP_ERROR_CHECK(err_code);
        initialized = true;
    }

    // Set the pin to be in the inactive state for now, so we don't draw any current.
    nrf_drv_gpiote_out_config_t out_en_config = GPIOTE_CONFIG_OUT_SIMPLE(!pin_active_high);
    err_code = nrf_drv_gpiote_out_init(POWER_BANK_PIN, &out_en_config);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Sets the pin to not draw any more current from the power bank,
 *        Uninits the gpio and stops the timer.
 */
void mm_power_bank_disable(void)
{
    // Uninit the gpio pin
    uint32_t err_code;
    nrf_drv_gpiote_out_uninit(POWER_BANK_PIN);

    // Stop the timer
    err_code = app_timer_stop(power_bank_timer_id);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Sets the amount of time the pin should draw power, and not draw power.
 *
 * @param[in] on_ticks_ms   Number of ms that the LED should stay on for.
 *                          NOTE that if this value is 0, the LED will just stay off.
 * @param[in] off_ticks_ms  Number of ms that the LED should stay off for.
 *                          NOTE that if this value is 0, the LED will just stay on.
 */
void mm_power_bank_set_on_off_cycle(uint16_t on_ticks_ms, uint16_t off_ticks_ms)
{
    uint32_t err_code;
    on_length_ms = on_ticks_ms;
    off_length_ms = off_ticks_ms;

    err_code = app_timer_stop(power_bank_timer_id);
    APP_ERROR_CHECK(err_code);

    if(on_ticks_ms > 0)
    {
        current_draw_state = true;
        err_code = app_timer_start(power_bank_timer_id, APP_TIMER_TICKS(on_length_ms), NULL);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        current_draw_state = false;
    }
    set_current_draw(current_draw_state);
}

/**
 * @brief Helper function to set the corrent state of the pin
 * to draw current or not, given whether the pin is active high or low.
 *
 * @param[in] draw_current     True to draw current, false to not draw current.
 */
static void set_current_draw(bool draw_current)
{
    if(pin_active_high)
    {
        // Active high! Set the pin high to draw current, clear the pin to not draw current.
        if(draw_current)
        {
            nrf_drv_gpiote_out_set(POWER_BANK_PIN);
        }
        else
        {
            nrf_drv_gpiote_out_clear(POWER_BANK_PIN);
        }
    }
    else
    {
        // Active low! Clear the pin to draw current, set high to not draw current.
        if(draw_current)
        {
            nrf_drv_gpiote_out_clear(POWER_BANK_PIN);
        }
        else
        {
            nrf_drv_gpiote_out_set(POWER_BANK_PIN);
        }
    }
    #if LED_DEBUG
        bsp_board_led_invert(3);
    #endif
}

/**
 * @brief Function to be called in timer interrupts for controlling whether we're drawing current or not.
 *
 * @param[in] p_context     General purpose pointer (unused).
 */
static void power_bank_timer_handler(void * p_context)
{
    uint32_t err_code;
    UNUSED_PARAMETER(p_context);

    if(current_draw_state == true)
    {
        // Change to stop drawing current!
        current_draw_state = false;
        set_current_draw(current_draw_state);
        err_code = app_timer_start(power_bank_timer_id, APP_TIMER_TICKS(off_length_ms), NULL);
        APP_ERROR_CHECK(err_code);
    }
    else if(current_draw_state == false)
    {
        // Change to start drawing current again!
        current_draw_state = true;
        set_current_draw(current_draw_state);
        err_code = app_timer_start(power_bank_timer_id, APP_TIMER_TICKS(on_length_ms), NULL);
        APP_ERROR_CHECK(err_code);
    }
}

/**
 *@}
 **/
