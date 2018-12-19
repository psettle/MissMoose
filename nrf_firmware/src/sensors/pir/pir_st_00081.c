/**@file
 * @defgroup pir_28027 PIR motion detection examples
 * @{
 * @ingroup pir_28027
 *
 * @brief Example of setting up pins and interrupts for the Parallax Wide Angle PIR sensor.
 * http://simplytronics.com/docs/index.php?title=ST-00081
 *
 * This PIR sensor has 4 pins - Vcc, Gnd, En, and a single output pin.
 * The output pin is High (1) when motion is detected, and Low (0) otherwise.
 * Use of the En (Enable) input pin is optional, and can be disconnected.
 *  - When the Enable pin is Low, the sensor is disabled to save power.
 *  - When the Enable pin is High or disconnected, the sensor is enabled.
 *  - Note that disabling and enabling the sensor requires it to warm up again.
 *
 * This sensor has a warmup period of 60 seconds after startup. Ideally, there should be minimal
 * movement in front of the sensor during that time.
 *
 * The sensor has a range of up to 30 feet in optimal conditions, in a 180 degree Field of View.
 */

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdio.h>
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "pir_st_00081_pub.h"

#define USE_ENABLE_PIN      false /* Whether or not the Enable pin of the sensor is to be used. */
#define USE_LED_INDICATOR   true /* Whether or not a detected change should be indicated on an LED. */
/**********************************************************
                        CONSTANTS
**********************************************************/


#ifdef BSP_BUTTON_1
    #define PIR_PIN_IN BSP_BUTTON_1
#endif
#ifndef PIR_PIN_IN
    #error "Please indicate input pin. For sanity checking, you can set this to BSP_BUTTON_1 and change the pin configuration to pullup, which will allow you to pretend you're getting input from the button instead of the sensor."
#endif

#if USE_ENABLE_PIN
    #ifdef BSP_BUTTON_2
        #define PIR_EN_PIN_OUT BSP_BUTTON_2
    #endif
    #ifndef PIR_EN_PIN_OUT
        #error "Please indicate output pin. This pin should be connected to the EN pin of the sensor."
    #endif
#endif

/* Choose the pin to use for some output indicator of a sensing change */
#if USE_LED_INDICATOR
    #ifdef BSP_LED_2
        #define PIR_INDICATOR_PIN_OUT BSP_LED_2
    #endif
    #ifndef PIR_INDICATOR_PIN_OUT
        #error "Please indicate output pin. This pin could be an indicator LED to show that something is detected."
    #endif
#endif
/**********************************************************
                       DECLARATIONS
**********************************************************/

static void pir_gpiote_init(void);
static void pir_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

/**********************************************************
                       DEFINITIONS
**********************************************************/
/**
 * @brief Function for initializing the wide-angle PIR sensor.
 */
void pir_st_00081_init(void)
{
    pir_gpiote_init();
}


/**
 * @brief Function for disabling the wide-angle PIR sensor.
 *
 * @details Note that after disabling and re-enabling the sensor,
 * it will have to go through it's 60-second initialization phase.
 */
void pir_st_00081_disable(void)
{
    #if USE_ENABLE_PIN
        nrf_drv_gpiote_out_clear(PIR_EN_PIN_OUT);
    #endif
}

/**
 * @brief Function for enabling the wide-angle PIR sensor.
 *
 * @details Note that after disabling and re-enabling the sensor,
 * it will have to go through it's 60-second initialization phase.
 */
void pir_st_00081_enable(void)
{
    #if USE_ENABLE_PIN
        nrf_drv_gpiote_out_set(PIR_EN_PIN_OUT);
    #endif
}

/**
 * @brief Function for configuring the GPIO pins and interrupts used by the PIR sensor.
 *
 * @details We want to configure PIR_PIN_IN as input, and PIR_INDICATOR_PIN_OUT for output. Also configure
 * GPIOTE to give an interrupt on PIR_PIN_IN change.
 * If the sensor's enable pin is to be used, configure an output pin for that as well.
 */
static void pir_gpiote_init(void)
{
    ret_code_t err_code;

    #if USE_LED_INDICATOR
        nrf_drv_gpiote_out_config_t out_led_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

        err_code = nrf_drv_gpiote_out_init(PIR_INDICATOR_PIN_OUT, &out_led_config);
        APP_ERROR_CHECK(err_code);
    #endif

    #if USE_ENABLE_PIN
        nrf_drv_gpiote_out_config_t out_en_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

        err_code = nrf_drv_gpiote_out_init(PIR_EN_PIN_OUT, &out_en_config);
        APP_ERROR_CHECK(err_code);
        pir_st_00081_enable();
    #endif

    /* Sense changes in either direction. Pull down the pin. */
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLDOWN;

    err_code = nrf_drv_gpiote_in_init(PIR_PIN_IN, &in_config, pir_in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIR_PIN_IN, true);
}

/**
 * @brief Function for handling a change in the signal from the wide angle PIR sensor.
 *
 * @details This function is called from the GPIOTE interrupt handler after the output
 * from the wide angle PIR sensor changes.
 *
 * @param[in] pin       The pin hooked up to the GPIOTE interrupt for the wide angle PIR sensor.
 * @param[in] action    Polarity setting for the GPIOTE channel.
 */
static void pir_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    #if USE_LED_INDICATOR
        if(nrf_drv_gpiote_in_is_set(PIR_PIN_IN))
        {
            nrf_drv_gpiote_out_set(PIR_INDICATOR_PIN_OUT);
        }
        else
        {
            nrf_drv_gpiote_out_clear(PIR_INDICATOR_PIN_OUT);
        }
    #endif
}

/**
 *@}
 **/
