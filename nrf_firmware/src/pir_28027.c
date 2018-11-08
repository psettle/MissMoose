/**@file
 * @defgroup pir_28027 PIR motion detection examples
 * @{
 * @ingroup pir_28027
 *
 * @brief Example of setting up pins and interrupts for the Parallax PIR sensor.
 * https://www.parallax.com/sites/default/files/downloads/555-28027-PIR-Sensor-Product-Guide-v2.3.pdf
 *
 * This PIR sensor has 3 pins - Vcc, Gnd, and a single output pin.
 * The output pin is High (1) when motion is detected, and Low (0) otherwise.
 *
 * This sensor has a warmup period of 40 seconds after startup. Ideally, there should be minimal
 * movement in front of the sensor during that time.
 *
 * The sensor has a range of up to 30 feet in optimal conditions, in a 90 degree Field of View.
 */

#include <stdio.h>
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "pir_28027_pub.h"

#define USE_LED_INDICATOR true /* Whether or not a detected change should be indicated on an LED. */

#ifdef BSP_BUTTON_0
    #define PIR_PIN_IN BSP_BUTTON_0
#endif
#ifndef PIR_PIN_IN
    #error "Please indicate input pin. For sanity checking, you can set this to BSP_BUTTON_0 and change the pin configuration to pullup, which will allow you to pretend you're getting input from the button instead of the sensor."
#endif

/* Choose the pin to use for some output indicator of a sensing change */
#if USE_LED_INDICATOR
    #ifdef BSP_LED_2
        #define PIR_PIN_OUT BSP_LED_2
    #endif
    #ifndef PIR_PIN_OUT
        #error "Please indicate output pin"
    #endif
#endif

static void pir_gpiote_init(void);
static void pir_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

/**
 * @brief Function for initializing the PIR sensor.
 */
void pir_28027_init(void)
{
    pir_gpiote_init();
}


/**
 * @brief Function for configuring the GPIO pins and interrupts used by the PIR sensor.
 *
 * @details We want to configure PIR_PIN_IN as input, and PIR_PIN_OUT for output. Also configure
 * GPIOTE to give an interrupt on PIR_PIN_IN change.
 */
static void pir_gpiote_init(void)
{
    ret_code_t err_code;

    #if USE_LED_INDICATOR
        nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

        err_code = nrf_drv_gpiote_out_init(PIR_PIN_OUT, &out_config);
        APP_ERROR_CHECK(err_code);
    #endif

    /* Sense changes in either direction. Pull down the pin. */
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLDOWN;

    err_code = nrf_drv_gpiote_in_init(PIR_PIN_IN, &in_config, pir_in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIR_PIN_IN, true);
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
        if(nrf_drv_gpiote_in_is_set(PIR_PIN_IN))
        {
            nrf_drv_gpiote_out_set(PIR_PIN_OUT);
        }
        else
        {
            nrf_drv_gpiote_out_clear(PIR_PIN_OUT);
        }
    #endif
}

/**
 *@}
 **/
