/**@file
 * @defgroup pir_st_00081 PIR motion detection examples
 * @{
 * @ingroup pir_st_00081
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
#include <string.h>
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "pir_st_00081_pub.h"
#include "nrf_error.h"
#include "app_timer.h"
#include "app_scheduler.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAXIMUM_NUM_PIRS            3
#define TOO_MANY_PIR_ERROR          NRF_ERROR_NOT_SUPPORTED
#define PIR_NOT_INITIALIZZED_ERROR  NRF_ERROR_NOT_SUPPORTED

#define USE_ENABLE_PIN              false /* Whether or not the Enable pin of the sensor is to be used. */
#if USE_ENABLE_PIN //No point in BUTTON_ENABLE_TEST if USE_ENABLE_PIN is false
    #define BUTTON_ENABLE_TEST      false //Set to true if you want to run this test
#else
    #define BUTTON_ENABLE_TEST      false //Always false in this case
#endif

#define PIR_REINFORCEMENT           ( true ) // Whether we should send the listeners a notification at least every 15s.
#define PIR_REINFORCEMENT_PERIOD_S  ( 15 )
#define ONE_SECOND_MS               ( 1000 )
#define ONE_SECOND_TICKS            APP_TIMER_TICKS(ONE_SECOND_MS)

/*Define pins for PIR enable and detectoion pins. Tried to select general I/O pins that were beside
each other based on the J102 track numbers. Placed into arrays to make code cleaner.
PIR_EN_OUT - The enable pin. PIR_PIN_IN - The signalling pin.
https://www.thisisant.com/assets/resources/Datasheets/D00001687_D52_Module_Datasheet.v.2.0.pdf - Page 15+16 for pin info

Available P0##'s that don't have Molex connections:
002 (j102.01), 018 (j102.02), 003 (j102.03), 004 (j102.05), 005 (j102.07)
028 (j102.09), 025 (j102.14), 026 (j102.16), 013 (j102.17), 027 (j102.18)
NOTE: 029 and 030 currently reserved for LIDAR (Dec 29, 2018)
*/
#define PIR1_PIN_IN         (2) //Pin J102.01
#define PIR2_PIN_IN         (3) //Pin J102.03
#define PIR3_PIN_IN         (4) //Pin J102.05

#define PIR1_PIN_EN_OUT     (18) //Pin J102.02
#define PIR2_PIN_EN_OUT     (5)  //Pin J102.07
#define PIR3_PIN_EN_OUT     (28) //Pin J102.09

#define MAX_EVT_HANDLERS    (4)
#define MISSED_EVENT_CHECK  true // Make sure we're not missing events being sent out due to main updates being called less frequently than updates.

/* Whether or not to utilize on-board LEDs for debugging. */
static bool led_debug;

static const uint8_t pir_input_pins[] = {PIR1_PIN_IN, PIR2_PIN_IN, PIR3_PIN_IN};
static const uint8_t pir_enable_pins[] = {PIR1_PIN_EN_OUT, PIR2_PIN_EN_OUT, PIR3_PIN_EN_OUT};

#if BUTTON_ENABLE_TEST
    //Button 0, 1, and 2: Referred to as BSP_BUTTON_0, BSP_BUTTON_1, BSP_BUTTON_2
    static const uint8_t pir_enable_ctrl_buttons[] = {BSP_BUTTON_0, BSP_BUTTON_1, BSP_BUTTON_2};
#endif

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void pir_gpiote_init(uint8_t num_pir_sensors);
static void pir_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
/* Processes timer timeout */
static void on_pin_event(void* evt_data, uint16_t evt_size);
static void pir_event_dispatch(pir_evt_t* evt_data);

#if BUTTON_ENABLE_TEST
    //Initializes enable
    static void pir_enable_button_init(uint8_t num_pir_sensors);
    //Handles a button being pressed.
    static void pir_button_press_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
#endif

#if(PIR_REINFORCEMENT)
    /* These functions are only used if we're reinforcing the pir data. */
    static void one_second_timer_handler(void * p_context);
    static void on_second_elapsed(void* p_unused, uint16_t size_0);
    static void pir_reinforcement_1s();
#endif

/**********************************************************
                       VARIABLES
**********************************************************/
//The number of PIR sensors that were initialized.
static uint8_t pir_sensor_count;
//The enable/disable status of the PIR sensors
static bool pirs_enabled[3] = {false, false, false};
//The state of the PIR sensors - High or low (Detecting something or not detecting)
static pir_event_type_t pirs_detecting[3] = {PIR_EVENT_CODE_NO_DETECTION, PIR_EVENT_CODE_NO_DETECTION, PIR_EVENT_CODE_NO_DETECTION};

/* Timer instance for reinforcement */
#if(PIR_REINFORCEMENT)
    APP_TIMER_DEF(m_pir_second_timer_id);
    /* Keep a counter of time since the last event notification for each PIR. */
    static uint32_t reinforcement_second_counters[MAXIMUM_NUM_PIRS] = {0,0,0};
#endif

/* Array of structs to hold the pinouts for individual PIR sensors */
static pir_pinout_t pir_sensors[MAXIMUM_NUM_PIRS];

static pir_evt_handler_t pir_evt_handlers[MAX_EVT_HANDLERS];

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * @brief Function for initializing the wide-angle PIR sensor.
 *
 * @param[in] num_pir_sensors The number of PIR sensors on the node (Maximum 3!).
 * @param[in] use_led_debug   Whether or not to use the on-board LEDs for debugging.
 */
void pir_st_00081_init(uint8_t num_pir_sensors, bool use_led_debug)
{
    //Error handling if num_pir_sensors > 3. Alternative: If greater than max, set to max?
    if(num_pir_sensors > MAXIMUM_NUM_PIRS){
        //Too many PIRs selected, throw an error.
        APP_ERROR_CHECK(TOO_MANY_PIR_ERROR);
    }

    led_debug = use_led_debug;
    pir_sensor_count = num_pir_sensors;

    pir_gpiote_init(pir_sensor_count);

    #if(PIR_REINFORCEMENT)
        ret_code_t err_code;
        /* Initialize 1 second timer, to be used for reinforcement. */\
        err_code = app_timer_create(&m_pir_second_timer_id, APP_TIMER_MODE_REPEATED, one_second_timer_handler);
        APP_ERROR_CHECK(err_code);

        err_code = app_timer_start(m_pir_second_timer_id, ONE_SECOND_TICKS, NULL);
        APP_ERROR_CHECK(err_code);
    #endif
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
    //Sensor IDs start from 0. So if the pir_sensor_count is 2, 0 and 1 are valid inputs. So >= 2 is not.
    if(pir_sensor_id >= pir_sensor_count){
        APP_ERROR_CHECK(PIR_NOT_INITIALIZZED_ERROR);
    }
    #if USE_ENABLE_PIN
        pirs_enabled[pir_sensor_id] = true; //This PIR is now enabled
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
    //Sensor IDs start from 0. So if the pir_sensor_count is 2, 0 and 1 are valid inputs. So >= 2 is not.
    if(pir_sensor_id >= pir_sensor_count){
        APP_ERROR_CHECK(PIR_NOT_INITIALIZZED_ERROR);
    }
    #if USE_ENABLE_PIN
        pirs_enabled[pir_sensor_id] = false; //This PIR is now disabled
        nrf_drv_gpiote_out_set(pir_sensors[pir_sensor_id].pir_en_pin_out);
    #endif
}

/**
 * @brief Function for checking if the wide-angle PIR sensor is enabled or disabled.
 *
 * @param[in] pir_sensor_id The ID of the pir sensor to check the enable status of
 */
bool check_pir_st_00081_enabled(uint8_t pir_sensor_id){
    //Sensor IDs start from 0. So if the pir_sensor_count is 2, 0 and 1 are valid inputs. So >= 2 is not.
    if(pir_sensor_id >= pir_sensor_count){
        APP_ERROR_CHECK(PIR_NOT_INITIALIZZED_ERROR);
    }
    return pirs_enabled[pir_sensor_id];
}

/**
 * @brief Function for checking if the wide-angle PIR sensor is detecting anything or not.
 *
 * @param[in] pir_sensor_id The ID of the pir sensor to check the detection status of
 * @param[in] sensor_event  The detecting status of the sensor is returned in this value.
 */
void check_pir_st_00081_detecting(uint8_t pir_sensor_id, pir_event_type_t* sensor_event)
{
    //Sensor IDs start from 0. So if the pir_sensor_count is 2, 0 and 1 are valid inputs. So >= 2 is not.
    if(pir_sensor_id >= pir_sensor_count){
        APP_ERROR_CHECK(PIR_NOT_INITIALIZZED_ERROR);
    }
    memcpy(sensor_event, &(pirs_detecting[pir_sensor_id]), sizeof(pir_event_type_t));
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

    for (uint8_t i = 0; i < num_pir_sensors; i++){
        //Set pir_en_pin_out and pir_pin_in
        pir_sensors[i].pir_en_pin_out = pir_enable_pins[i];
        pir_sensors[i].pir_pin_in = pir_input_pins[i];

        /* If use_enable_pin true, set up enable pin */
#if USE_ENABLE_PIN
        nrf_drv_gpiote_out_config_t out_en_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

        err_code = nrf_drv_gpiote_out_init(pir_sensors[i].pir_en_pin_out, &out_en_config);
        APP_ERROR_CHECK(err_code);
        pir_st_00081_enable(i); //The ID of the PIR sensor to enable
#else
        //The enable pins are not used. The PIR should always be listed as enabled.
        pirs_enabled[i] = true; //This PIR is now enabled
#endif

        /* Sense changes in either direction. Pull down the pin. */
        nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
        in_config.pull = NRF_GPIO_PIN_PULLDOWN;

        err_code = nrf_drv_gpiote_in_init(pir_sensors[i].pir_pin_in, &in_config, pir_in_pin_handler);
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_event_enable(pir_sensors[i].pir_pin_in, true);

        //Default setting: PIRs are not detecting anything
        pirs_detecting[i] = PIR_EVENT_CODE_NO_DETECTION;

    }

#if BUTTON_ENABLE_TEST
    //Initializes buttons to enable or disable the PIR sensors.
    pir_enable_button_init(num_pir_sensors);
#endif

}

/**
 * @brief Interrupt handlers for a changing signal from the wide angle PIR sensor.
 *
 * @details This function is called from the GPIOTE interrupt handler after the output
 * from the wide angle PIR sensor changes.
 *
 * @param[in] pin       The pin hooked up to the GPIOTE interrupt for the wide angle PIR sensor.
 * @param[in] action    Polarity setting for the GPIOTE channel.
 */
static void pir_in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint32_t err_code;
    pir_pin_change_evt_t pin_evt;
    pin_evt.pin = pin;
    pin_evt.pin_state = nrf_drv_gpiote_in_is_set(pin);

    err_code = app_sched_event_put(&pin_evt, sizeof(pir_pin_change_evt_t), on_pin_event);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for handling a change in the signal from the wide angle PIR sensor.
 *
 * @details Run the bulk of the processing to be done after a PIR input pin change.
 *
 * @param[in] evt_data    pir_pin_change_evt_t data about the pin event
 * @param[in] evt_size    size of the pin event.
 */
static void on_pin_event(void* evt_data, uint16_t evt_size)
{
    pir_pin_change_evt_t* pin_evt = (pir_pin_change_evt_t*)evt_data;

    /*LEDs are indexed from 0 to 3. LED commands:
     * bsp_board_led_off, bsp_board_led_on, bsp_board_led_invert
     *
     * Match the pin with the PIR sensor - If pin in matches pin, that's the PIR that detected something.*/
    for(uint8_t i = 0; i < MAXIMUM_NUM_PIRS; i++)
    {
        if (pir_sensors[i].pir_pin_in == pin_evt->pin)
        {
            //If the input pin for that PIR sensor is set:
            if(pin_evt->pin_state)
            {
                if(led_debug)
                {
                    bsp_board_led_on(i);
                }
                pirs_detecting[i] = PIR_EVENT_CODE_DETECTION;
            }
            else
            {
                if(led_debug)
                {
                    bsp_board_led_off(i);
                }
                pirs_detecting[i] = PIR_EVENT_CODE_NO_DETECTION;
            }

            pir_evt_t pir_evt;
            pir_evt.event = pirs_detecting[i];
            pir_evt.sensor_index = i;
            pir_event_dispatch(&pir_evt);
            #if(PIR_REINFORCEMENT)
                reinforcement_second_counters[i] = 0;
            #endif
        }
    }
}



/**
 * @brief Add a new pir event listener
 *
 * @param[in] pir_evt_handler The event handler to add as a listener.
 */
void pir_evt_handler_register(pir_evt_handler_t pir_evt_handler)
{
    uint32_t i;

    for (i = 0; i < MAX_EVT_HANDLERS; i++)
    {
        if (pir_evt_handlers[i] == NULL)
        {
            pir_evt_handlers[i] = pir_evt_handler;
            break;
        }
    }

    APP_ERROR_CHECK(i == MAX_EVT_HANDLERS);
}

/**
 * @brief Tell the listeners that there's recently been a change in detection,
 *        and an index of the PIR that changed.
 * @param[in] evt_data actual data for the sensor event
 */
static void pir_event_dispatch(pir_evt_t* evt_data)
{
    // Forward PIR event to listeners
    for (uint32_t i = 0; i < MAX_EVT_HANDLERS; i++)
    {
        if (pir_evt_handlers[i] != NULL)
        {
            pir_evt_handlers[i](evt_data);
        }
        else
        {
            break;
        }
    }
}

#if BUTTON_ENABLE_TEST
    //Initializes buttons to enable or disable the PIR sensors.
    static void pir_enable_button_init(uint8_t num_pir_sensors){

    /* Sense changes in either direction on the control pin. Pull up the pin,
       since we expect to use an IO board button for this. (Based on ir_led_transmit.c)*/
    //Perform for each PIR
    ret_code_t err_code;
    for(uint8_t i = 0; i < num_pir_sensors; i++){
            nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
            in_config.pull = NRF_GPIO_PIN_PULLUP;

            err_code = nrf_drv_gpiote_in_init(pir_enable_ctrl_buttons[i], &in_config, pir_button_press_handler);
            APP_ERROR_CHECK(err_code);

            nrf_drv_gpiote_in_event_enable(pir_enable_ctrl_buttons[i], true);
    }

    }
    //Handles a button being pressed.
    static void pir_button_press_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
        //Check that the pin was set - Otherwise the handler will run both when the button is pressed and depressed.
        //If it was set, then toggle the enable line for that PIR sensor
        if(nrf_drv_gpiote_in_is_set(pin)){
            for(uint8_t i = 0; i < MAXIMUM_NUM_PIRS; i++){
                if(pir_enable_ctrl_buttons[i] == pin){
                    nrf_drv_gpiote_out_toggle(pir_sensors[i].pir_en_pin_out);
                }
            }
        }

    }
#endif


#if(PIR_REINFORCEMENT)
    /**
        Timer handler to process second tick.
    */
    static void one_second_timer_handler(void * p_context)
    {
        /* Send off event to be handled in main context */
        uint32_t err_code = app_sched_event_put(NULL, 0, on_second_elapsed);
        APP_ERROR_CHECK(err_code);
    }

    /**
     * Handle events to be carried out when 1 second elapses.
     * Invoked from main context.
     */
    static void on_second_elapsed(void* p_unused, uint16_t size_0)
    {
        pir_reinforcement_1s();
    }

    /**
     * Check to see if enough time has elapsed without telling
     * the sensor manager anything. If so, pack up and send a notification.
     */
    static void pir_reinforcement_1s()
    {
        for(uint8_t i = 0; i < pir_sensor_count; i++)
        {
            reinforcement_second_counters[i]++;
            /* Only send an update on multiples of the reinforcment period. */
            if(reinforcement_second_counters[i] % PIR_REINFORCEMENT_PERIOD_S != 0)
            {
                return;
            }
            /* We made it this far so we can reset the counter */
            reinforcement_second_counters[i] = 0;

            /* Check that the sensor is enabled */
            if(check_pir_st_00081_enabled(i))
            {
                pir_evt_t pir_evt;
                pir_evt.event = pirs_detecting[i];
                pir_evt.sensor_index = i;
                /* Send out a notification for the PIR sensor*/
                pir_event_dispatch(&pir_evt);
            }
        }
    }
#endif

/**
 *@}
 **/
