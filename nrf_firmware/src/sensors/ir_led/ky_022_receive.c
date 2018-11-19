/**@file
 * @defgroup ky_022 IR receiver
 * @{
 * @ingroup ky_022
 *
 * @brief More information about the KY-022 is available here:
 * https://arduinomodules.info/ky-022-infrared-receiver-module/
 * More information about IR communication is available here:
 * https://learn.sparkfun.com/tutorials/ir-communication/all
 *
 * This receiver has 3 pins - Vcc, Gnd, and a single output pin.
 * The output pin is High (1) when receiving a 38kHz signal and Low (0) otherwise.
 *
 * A conventional use of this sensor involves sending 38kHz signals in bursts at very specific intervals
 * in order to transmit data, but we're just going to use it to detect whether or not a carrier signal
 * is being received, so it essentially becomes a break-beam sensor. Like your sibling walking between
 * your remote and the TV.
 */

#include <stdio.h>
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "ky_022_receive_pub.h"

#define USE_LED_INDICATOR true /* Whether or not a detected change should be indicated on an LED. */

static void pir_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
static nrf_drv_gpiote_pin_t pin_input;
static nrf_drv_gpiote_pin_t pin_indicator;

/**
 * @brief Function for initializing the ky-022 receiver.
 *
 * @details We want to configure an input pin and an output indicator pin. Also configure
 * GPIOTE to give an interrupt on input pin value change.
 *
 * @param[in] input_pin       The pin hooked up to the GPIOTE interrupt for the receiver.
 * @param[in] indicator_pin   The pin which we want to toggle in response to a change in the received signal.
 */
void ky_022_init(nrf_drv_gpiote_pin_t input_pin, nrf_drv_gpiote_pin_t indicator_pin)
{
    ret_code_t err_code;
    pin_input = input_pin;
    pin_indicator = indicator_pin;

    #if USE_LED_INDICATOR
        nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

        err_code = nrf_drv_gpiote_out_init(pin_indicator, &out_config);
        APP_ERROR_CHECK(err_code);
        nrf_drv_gpiote_out_set(pin_indicator);
    #endif

    /* Sense changes in either direction. The output of ky-022 is active-low, so pull up the pin. */
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(pin_input, &in_config, pir_in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(pin_input, true);
}

/**
 * @brief Function for deinitializing the ky-022 receiver's gpiote settings and events.
 */
void ky_022_deinit(void)
{

}

/**
 * @brief Function for handling a change in the signal from the PIR sensor.
 *
 * @details This function is called from the GPIOTE interrupt handler after the output
 * from the PIR sensor changes.
 *
 * @param[in] pin       The pin hooked up to the GPIOTE interrupt for the PIR sensor.
 * @param[in] action    Polarity setting for the GPIOTE channel.
 */
static void pir_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    #if USE_LED_INDICATOR
        if(nrf_drv_gpiote_in_is_set(pin_input))
        {
            nrf_drv_gpiote_out_set(pin_indicator);
        }
        else
        {
            nrf_drv_gpiote_out_clear(pin_indicator);
        }
    #endif
}

/**
 *@}
 **/
