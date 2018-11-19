/**@file
 * @defgroup ir_led Transmitter
 * @{
 * @ingroup ir_led
 *
 * @brief More information about IR LED transmitters are available here
 * https://learn.sparkfun.com/tutorials/ir-communication/all
 *
 * The transmitter is really just a special IR led. It's controlled by a power pin and a ground pin.
 * The purpose of this code is to make a GPIO pin pulse at 38kHz, which will control a MOSFET
 * that gives the LED lots of current from the Vcc line at 38kHz.
 *
 * A diagram of how to wire up this transmitter is available here:
 * //todo
 */

#include <stdio.h>
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "ir_led_transmit_pub.h"
#include "app_pwm.h"
#include "nrf_delay.h"

#define CONTINUOUS_TRANSMISSION false       // Pulse the transmitting LED all the time.
                                            // Only recommended if you have a power supply other than batteries.

APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.

#if CONTINUOUS_TRANSMISSION == false
    static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
#endif
static volatile bool        ready_flag = false; // A flag indicating PWM status.
static uint32_t             duty_cycle = 0;     // The current duty cycle for the transmitter.
                                                // 0 is off, 50 is transmitting normally.
static nrf_drv_gpiote_pin_t transmit_pin;
static nrf_drv_gpiote_pin_t control_pin;

/**
 * @brief PWM ready callback function - called by the PWM library.
 */
void pwm_ready_callback(uint32_t pwm_id)
{
    ready_flag = true;
}

/**
 * @brief A call for updating the ir led transmitter.
 *
 * @details There are some parts of configuration that seem to require a continuous update
 * loop until the changes take place. Calling into this function in a loop from main is necessary.
 * In the future, it would be much cooler if the initialization provided a callback function
 * that main could call, instead of main having to explicitly call this exact function.
 *
 */
void ir_led_transmit_update(void)
{
    if(!ready_flag)
    {
        if(app_pwm_channel_duty_set(&PWM1, 0, duty_cycle) != NRF_ERROR_BUSY)
        {
            ready_flag = true;
        }
    }
}

/**
 * @brief Function for initializing the IR transmitter, utilizing the PWM library.
 *
 * @param[in] ctrl_pin   A pin for controlling when the transmitter should pulse. Typically a button.
 * @param[in] output_pin    The pin to be pulsed for controlling the transmitter.
 *
 */
void ir_led_transmit_init(nrf_drv_gpiote_pin_t ctrl_pin, nrf_drv_gpiote_pin_t output_pin)
{
    ret_code_t err_code;
    transmit_pin = output_pin;
    control_pin = ctrl_pin;

    #if CONTINUOUS_TRANSMISSION == false
        /* Sense changes in either direction on the control pin. Pull up the pin,
         * since we expect to use an IO board button for this. */
        nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
        in_config.pull = NRF_GPIO_PIN_PULLUP;

        err_code = nrf_drv_gpiote_in_init(control_pin, &in_config, control_pin_handler);
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_event_enable(control_pin, true);
    #endif

    /* 2-channel PWM, 38000Hz, output on selected pin. */
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH((1000000.0/38000.0), transmit_pin);

    /* Initialize and enable PWM. */
    err_code = app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM1);

    #if CONTINUOUS_TRANSMISSION
        duty_cycle = 50;
        while(app_pwm_channel_duty_set(&PWM1, 0, duty_cycle) == NRF_ERROR_BUSY);
    #else

        duty_cycle = 0;
        ready_flag = false;
    #endif
}

#if CONTINUOUS_TRANSMISSION == false
    /**
     * @brief Function for handling a change in the pin controlling IR transmission.
     *
     * @param[in] pin       The pin hooked up to the GPIOTE interrupt for the control pin.
     * @param[in] action    Polarity setting for the GPIOTE channel.
     */
    static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
    {
        if(nrf_drv_gpiote_in_is_set(control_pin))
        {
            duty_cycle = 0;
            ready_flag = false;
        }
        else
        {
            duty_cycle = 50;
            ready_flag = false;
        }
    }
#endif

/**
 *@}
 **/
