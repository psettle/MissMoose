/**@file mm_hardware_test.c
 *
 * @brief This file is useful for verifying that each of the
 *        different node configurations are working as expected.
 */

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_hardware_test_pub.h"

#include <string.h>
#include "pir_st_00081_pub.h"
#include "mm_rgb_led_pub.h"
#include "lidar_pub.h"
#include "app_timer.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define TIMED_UPDATE_PERIOD     10   ///< In ms
#define LIDAR_COOLDOWN_PERIOD   5000 ///< In ms, the duration of time after a lidar detection
                                     ///< that we want to signal for
#define PIR_COOLDOWN_PERIOD     5000 ///< In ms, the duration of time after a PIR detection
                                     ///< that we want to signal for

/**
 * @brief states that the more complicated case of using
 *        a PIR, LIDAR, and LED strip can be in.
 */
typedef enum
{
    NO_DETECTION = 0,
    PIR_DETECTION,
    LIDAR_DETECTION,
    LIDAR_PIR_DETECTION,
}detection_test_states_t;


/**********************************************************
                       DECLARATIONS
**********************************************************/

static void timer_event_handler(void * p_context);

static void process_lidar_evt(lidar_evt_t * evt);

static void process_pir_evt(pir_evt_t * evt);

static void state_transition_update(detection_test_states_t new_state);

/**********************************************************
                        VARIABLES
**********************************************************/

/* This is used in main to check if we should call the lidar update function. */
static bool lidar_in_use;
static uint16_t lidar_event_cooldown = 0;
static uint16_t pir_event_cooldown = 0;

static detection_test_states_t state = NO_DETECTION;

/* Timer instance. */
APP_TIMER_DEF(m_hardware_test_timer);

/**********************************************************
                       DEFINITIONS
**********************************************************/


/**
 * @brief Reads the dip switches to find out what the hardware
 *        configuration should be, then initializes sensors
 *        as appropriate.
 */
void mm_hardware_test_init(hardware_config_t config)
{
    ret_code_t err_code;

    switch(config)
    {
        case HARDWARE_CONFIG_PIR_PIR:
            /* If the node type is just 2 PIRs, set up the PIRs using the LED debugging. */
            lidar_in_use = false;
            pir_st_00081_init(2, true);
            break;

        case HARDWARE_CONFIG_PIR_LIDAR:
            /* If the node type is 1 PIR and 1 LIDAR, initialize them and use their own LED debugging. */
            lidar_in_use = true;
            pir_st_00081_init(1, true);
            lidar_init(true);
            break;

        case HARDWARE_CONFIG_PIR_LIDAR_LED:
            lidar_in_use = true;
            mm_rgb_led_init(false);
            pir_st_00081_init(1, true);
            lidar_init(true);

            /* Set ourselves up as a listener for the lidar */
            lidar_evt_handler_register(process_lidar_evt);

            /* Set ourselves up as a listener for the pir sensors */
            pir_evt_handler_register(process_pir_evt);

            /* If the configuration uses the LED strip, set up a timer for something fancier. */
            err_code = app_timer_create(&m_hardware_test_timer, APP_TIMER_MODE_REPEATED, timer_event_handler);
            APP_ERROR_CHECK(err_code);

            err_code = app_timer_start(m_hardware_test_timer, APP_TIMER_TICKS(TIMED_UPDATE_PERIOD), NULL);
            APP_ERROR_CHECK(err_code);
            break;

        default:
        	APP_ERROR_CHECK(true);
    }
}

/**
 * @brief Function for updating the hardware test from main.
 */
void mm_hardware_test_update_main(void)
{
    if(lidar_in_use)
    {
        lidar_update_main();
    }
}

/**
 * @brief Handles updates from the LIDAR sensor.
 *
 * @param[in] evt The event information from the lidar detection.
 */
static void process_lidar_evt(lidar_evt_t * evt)
{
    switch (evt->event)
    {
        // For simplicity, let's only care about the case where the distance has JUST changed.
        case LIDAR_EVENT_CODE_MEASUREMENT_CHANGE:
            /* Reset the cooldown period */
            lidar_event_cooldown = LIDAR_COOLDOWN_PERIOD;
            break;

        default:
            break;
    }
}

/**
 * @brief Handles updates from the PIR sensors.
 *
 * @param[in] evt The event information from the pir detection.
 */
static void process_pir_evt(pir_evt_t * evt)
{
    switch (evt->event)
    {
        // For simplicity, let's only care about the fact that some PIR has suddenly detected something.
        case PIR_EVENT_CODE_DETECTION:
            /* Reset the cooldown period */
            pir_event_cooldown = PIR_COOLDOWN_PERIOD;
            break;

        default:
            break;
    }
}

/**
 * @brief Handler for timer events.
 *
 * @param[in] p_context Unused information related to the timer event.
 */
static void timer_event_handler(void * p_context)
{
    /* Reduce the lidar event cooldown by however fast this update is happening */
    lidar_event_cooldown = lidar_event_cooldown == 0 ? 0 : lidar_event_cooldown - TIMED_UPDATE_PERIOD;
    /* Reduce the pir event cooldown by however fast this update is happening */
    pir_event_cooldown = pir_event_cooldown == 0 ? 0 : pir_event_cooldown - TIMED_UPDATE_PERIOD;


    /* Determine what the current state should be */
    if(pir_event_cooldown == 0 && lidar_event_cooldown == 0)
    {
        state_transition_update(NO_DETECTION);
    }
    else if(pir_event_cooldown > 0 && lidar_event_cooldown == 0)
    {
        state_transition_update(PIR_DETECTION);
    }
    else if(pir_event_cooldown == 0 && lidar_event_cooldown > 0)
    {
        state_transition_update(LIDAR_DETECTION);
    }
    else if(pir_event_cooldown > 0 && lidar_event_cooldown > 0)
    {
        state_transition_update(LIDAR_PIR_DETECTION);
    }
}

/**
 * @brief Transitions to the new_state, if it's different from the current state.
 *
 * @param[in] new_state The new state we ought to transition to.
 */
static void state_transition_update(detection_test_states_t new_state)
{
    if(new_state == state)
    {
        return;
    }
    else
    {
        state = new_state;
    }
    switch(state)
    {
        case NO_DETECTION:
            /* Change the LEDs to be off */
            mm_rgb_led_set_colour(MM_RGB_LED_DUTY_0);
            mm_rgb_set_on_off_cycle(0, 500);
            break;
        case PIR_DETECTION:
            /* Change the LEDs to flashing yellow */
            mm_rgb_led_set_colour(MM_RGB_COLOUR_YELLOW);
            mm_rgb_set_on_off_cycle(500, 500);
            break;
        case LIDAR_DETECTION:
            /* Change the LEDs to flashing purple */
            mm_rgb_led_set_colour(MM_RGB_COLOUR_PURPLE);
            mm_rgb_set_on_off_cycle(500, 500);
            break;
        case LIDAR_PIR_DETECTION:
            /* Change the LEDs to solid red. */
            mm_rgb_led_set_colour(MM_RGB_COLOUR_RED);
            mm_rgb_set_on_off_cycle(500, 0);
            break;
    }
}
