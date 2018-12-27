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

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAXIMUM_NUM_PIRS    3
#define TOO_MANY_PIR_ERROR  -1

#define USE_ENABLE_PIN      false /* Whether or not the Enable pin of the sensor is to be used. */

//Elijah TODO - Delete
// #ifdef BSP_BUTTON_1
//     #define PIR_PIN_IN BSP_BUTTON_1
// #endif
// #ifndef PIR_PIN_IN
//     #error "Please indicate input pin. For sanity checking, you can set this to BSP_BUTTON_1 and change the pin configuration to pullup, which will allow you to pretend you're getting input from the button instead of the sensor."
// #endif

// #if USE_ENABLE_PIN
//     #ifdef BSP_BUTTON_2
//         #define PIR_EN_PIN_OUT BSP_BUTTON_2
//     #endif
//     #ifndef PIR_EN_PIN_OUT
//         #error "Please indicate output pin. This pin should be connected to the EN pin of the sensor."
//     #endif
// #endif

/*Elijah TODO - Available P0##'s: 005 (j102.07), 018 (j102.02), 004 (j102.05), 003 (j102.03), 
026 (j102.16), 028 (j102.09), 027 (j102.18), 025 (j102.14), 013 (j102.17)
NOTE: 029 and 030 reserved for LIDAR*/

/*Define pins for PIR enable and detectoion pins. Tried to select general I/O pins that were beside 
each other based on the J102 track numbers. Placed into arrays to make code cleaner.*/
//PIR_EN_OUT - The enable pin. PIR_PIN_IN - The signalling pin.
//https://www.thisisant.com/assets/resources/Datasheets/D00001687_D52_Module_Datasheet.v.2.0.pdf - Page 15+16 for pin info
#define PIR1_PIN_IN         3 //Pin J102.03
#define PIR2_PIN_IN         4 //Pin J102.05
//Used pin 26 for this originally. Trying other pins.
#define PIR3_PIN_IN         5  // Pin J102.07

#define PIR1_PIN_EN_OUT     18 //Pin J102.02
#define PIR2_PIN_EN_OUT     28 //Pin J102.09
//Used pin 25 for this originally. Trying other pins.
#define PIR3_PIN_EN_OUT     25 //Pin J102.14


static const uint8_t pir_input_pins[] = {PIR1_PIN_IN, PIR2_PIN_IN, PIR3_PIN_IN};
static const uint8_t pir_enable_pins[] = {PIR1_PIN_EN_OUT, PIR2_PIN_EN_OUT, PIR3_PIN_EN_OUT};

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void pir_gpiote_init(uint8_t num_pir_sensors);
static void pir_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);


/**********************************************************
                       DEFINITIONS
**********************************************************/
/* Array of structs to hold the pinouts for individual PIR sensors */
pir_pinout_t pir_sensors[MAXIMUM_NUM_PIRS];

/**
 * @brief Function for initializing the wide-angle PIR sensor.
 * 
 * @param[in] num_pir_sensors The number of PIR sensors on the node (Maximum 3!).
 */
void pir_st_00081_init(uint8_t num_pir_sensors)
{
    //Elijah TODO - Error handling if num_pir_sensors > 3. Alternative: If greater than max, set to max?
    if(num_pir_sensors > MAXIMUM_NUM_PIRS){
        //Too many PIRs selected, throw an error.
        APP_ERROR_CHECK(TOO_MANY_PIR_ERROR);
    }

    pir_gpiote_init(num_pir_sensors);
}


/**
 * @brief Function for disabling the wide-angle PIR sensor.
 *
 * @details Note that after disabling and re-enabling the sensor,
 * it will have to go through it's 60-second initialization phase.
 * 
 * @param[in] pir_sensor_id The ID of the pir sensor to disable
 */
void pir_st_00081_disable(uint8_t pir_sensor_id)
{
    #if USE_ENABLE_PIN
        nrf_drv_gpiote_out_clear(pir_sensors[pir_sensor_id].pir_en_pin_out);
    #endif
}

/**
 * @brief Function for enabling the wide-angle PIR sensor.
 *
 * @details Note that after disabling and re-enabling the sensor,
 * it will have to go through it's 60-second initialization phase.
 * 
 * @param[in] pir_sensor_id The ID of the pir sensor to disable
 */
void pir_st_00081_enable(uint8_t pir_sensor_id)
{
    #if USE_ENABLE_PIN
        nrf_drv_gpiote_out_set(pir_sensors[pir_sensor_id].pir_en_pin_out);
    #endif
}

/**
 * @brief Function for configuring the GPIO pins and interrupts used by each PIR sensor.
 *
 * @details We want to configure a pir_pin_in pin as input for each sensor. 
 * Also configure GPIOTE to give an interrupt on pir_pin_in change.
 * If the sensor's enable pin is to be used, configure an output pin for that as well.
 * 
 * @param[in] num_pir_sensors The number of PIR sensors on the node (Maximum 3!).
 */
static void pir_gpiote_init(uint8_t num_pir_sensors)
{
    
    ret_code_t err_code;

    for (int i = 0; i < num_pir_sensors; i++){
        //Set pir_en_pin_out and pir_pin_in
        pir_sensors[i].pir_en_pin_out = pir_enable_pins[i];
        pir_sensors[i].pir_pin_in = pir_input_pins[i];

        /* If use_enable_pin true, set up enable pin */
#if USE_ENABLE_PIN
        nrf_drv_gpiote_out_config_t out_en_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

        err_code = nrf_drv_gpiote_out_init(pir_sensors[i].pir_en_pin_out, &out_en_config);
        APP_ERROR_CHECK(err_code);
        pir_st_00081_enable(&pir_sensors[i]);
#endif

        /* Sense changes in either direction. Pull down the pin. */
        nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
        in_config.pull = NRF_GPIO_PIN_PULLDOWN;

        err_code = nrf_drv_gpiote_in_init(pir_sensors[i].pir_pin_in, &in_config, pir_in_pin_handler);
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_event_enable(pir_sensors[i].pir_pin_in, true); 
    
    }

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
    /*Elijah TODO - Handle detection here. See if the "pin" input matches the pir_sensors[i].pir_pin_in
    * --> Need some way to differentiate between sensors!
    */
    //Looks like the LEDs are just indexed? From 0 to 3?
    //Commands - bsp_board_led_off, bsp_board_led_on, bsp_board_led_invert
    // Match the pin with the PIR sensor.
    for(int i = 0; i < MAXIMUM_NUM_PIRS; i++){
        //Elijah TODO - Clean up notes and explanations if this stuff works
        //See if the PIR pin matches the pir sensor
        if (pir_sensors[i].pir_pin_in == pin){
            //If the input pin for that PIR sensor is set:
            if (nrf_drv_gpiote_in_is_set(pir_sensors[i].pir_pin_in)){
                bsp_board_led_on(i); //TODO
            }
            else{
                bsp_board_led_off(i); //TODO
            }
        }
    }

    //This will never run - Elijah TODO, delete
    #if USE_LED_INDICATOR
#error "this shouldn't run"
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
