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
 * A diagram of how to wire up this transmitter and the receiver is available on the capstone google drive
 * under Capstone > Sensors > IR-hookup.png
 */

#include <stdio.h>
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "ir_led_transmit_pub.h"
#include "app_pwm.h"
#include "nrf_delay.h"
#include "nrf_timer.h"
#include "nrf_drv_timer.h"

#define CONTINUOUS 0
#define BUTTON 1
#define TIMED 2


#define CONTROL TIMED

APP_PWM_INSTANCE(PWM1,2);                   /**< Create the instance "PWM1" using TIMER2. */

#if CONTROL == BUTTON
    static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
#elif CONTROL == TIMED
    /**@brief Timer time-out handler type. */
    typedef void (*simple_timer_timeout_handler_t)(void * p_context);                  /**< Registered timer mode. */
    const nrf_drv_timer_t SIMPLE_TIMER = NRF_DRV_TIMER_INSTANCE(1);
    static void simple_timer_event_handler(nrf_timer_event_t event_type, void * p_context);
    uint32_t simple_timer_init(void);
    uint32_t simple_timer_start(uint16_t timeout_ticks);
#endif

static volatile bool        ready_flag = false; /**< A flag indicating PWM status. */
static uint32_t             duty_cycle = 0;     /**< The current duty cycle for the transmitter. */
                                                /**< 0 is off, 50 is transmitting normally. */
#if CONTROL == TIMED
    /**
     * @brief State of timer configuration
     */
    typedef enum
    {
        CHANGE_TO_SHORT_TIME,   ///< Need to change the timer to have a short interval
        CHANGE_TO_LONG_TIME,    ///< Need to change the timer to have a long interval
        SET_TO_LONG,            ///< Timer has been changed to the long time.
        SET_TO_SHORT            ///< Timer has been changed to the short time.
    } timer_change_t;

    /**
     * @brief States of simple timer state machine.
     */
    typedef enum
    {
        TIMER_STATE_IDLE = 0,
        TIMER_STATE_INITIALIZED,
        TIMER_STATE_STOPPED,
        TIMER_STATE_STARTED
    }simple_timer_states_t;

    static uint32_t              short_time = ( 50 );   /**< Technically in us, but we count it 1000 times so it becomes ms. */
    static uint32_t              long_time =  ( 450 );  /**< Technically in us, but we count it 1000 times so it becomes ms. */
    static uint32_t              timer_ticks_setting;
    static uint32_t              timer_cycles = 0;
    static timer_change_t        timer_change_flag;               /**< Registered time-out handler context. */
    static simple_timer_states_t simple_timer_state = TIMER_STATE_IDLE; /** < State machine state. */

#endif

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
    #if CONTROL == TIMED
        if(timer_change_flag == CHANGE_TO_SHORT_TIME)
        {
            timer_change_flag = SET_TO_SHORT;
            duty_cycle = 50;
            ready_flag = false;
            app_pwm_enable(&PWM1);
        }
        else if(timer_change_flag == CHANGE_TO_LONG_TIME)
        {
            timer_change_flag = SET_TO_LONG;
            duty_cycle = 0;

            app_pwm_disable(&PWM1);
            ready_flag = true;
        }
    #endif
}

/**
 * @brief Function for initializing the IR transmitter, utilizing the PWM library.
 *
 * @param[in] ctrl_pin      A pin for controlling when the transmitter should pulse. Typically a button.
 *                          only used if the control type is BUTTON.
 * @param[in] output_pin    The pin to be pulsed for controlling the transmitter.
 *
 */
void ir_led_transmit_init(nrf_drv_gpiote_pin_t ctrl_pin, nrf_drv_gpiote_pin_t output_pin)
{
    ret_code_t err_code;
    transmit_pin = output_pin;
    control_pin = ctrl_pin;

    #if CONTROL == BUTTON
        /* Sense changes in either direction on the control pin. Pull up the pin,
         * since we expect to use an IO board button for this. */
        nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
        in_config.pull = NRF_GPIO_PIN_PULLUP;

        err_code = nrf_drv_gpiote_in_init(control_pin, &in_config, control_pin_handler);
        APP_ERROR_CHECK(err_code);

        nrf_drv_gpiote_in_event_enable(control_pin, true);
    #endif

    #if CONTROL == TIMED
        timer_change_flag = SET_TO_SHORT;
        err_code = simple_timer_init();
        APP_ERROR_CHECK(err_code);
        timer_ticks_setting = short_time;
        err_code = simple_timer_start(timer_ticks_setting);
        APP_ERROR_CHECK(err_code);
    #endif

    /* 2-channel PWM, 38000Hz, output on selected pin. */
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH((1000000.0/38000.0), transmit_pin);

    /* Initialize and enable PWM. */
    err_code = app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback);
    APP_ERROR_CHECK(err_code);

    #if CONTROL == CONTINUOUS
        duty_cycle = 50;
        app_pwm_enable(&PWM1);
        while(app_pwm_channel_duty_set(&PWM1, 0, duty_cycle) == NRF_ERROR_BUSY);
        ready_flag = false;
    #elif CONTROL == BUTTON
        app_pwm_disable(&PWM1);
        ready_flag = true;
    #elif CONTROL == TIMED
        duty_cycle = 50;
        ready_flag = false;
        app_pwm_enable(&PWM1);
    #endif
}

#if CONTROL == TIMED
    /**
     * @brief Handler for timer events. Roughly ganked from app_simple_timer.c
     */
    static void simple_timer_event_handler(nrf_timer_event_t event_type, void * p_context)
    {
        uint32_t err_code;
        switch (event_type)
        {
            case NRF_TIMER_EVENT_COMPARE0:
                if( ++timer_cycles >= 1000) // 1Mhz timer counter only has 16 bits, so we do 1000 cycles so we can time stuff in ms.
                {
                    simple_timer_state = TIMER_STATE_STOPPED;
                    timer_change_flag = timer_change_flag == SET_TO_LONG ? CHANGE_TO_SHORT_TIME : CHANGE_TO_LONG_TIME;
                    timer_ticks_setting = timer_ticks_setting == long_time ? short_time : long_time;
                    bsp_board_led_invert(1); // This is a helpful indicator bonus, but for some reason when i take out this line everything breaks.
                    timer_cycles = 0;
                }
                err_code = simple_timer_start(timer_ticks_setting);
                APP_ERROR_CHECK(err_code);
                break;

            default:
                //Do nothing.
                break;
        }
    }

    /**
     * @brief Function for initializing the timer. Roughly ganked from app_simple_timer.c
     */
    uint32_t simple_timer_init(void)
    {
        uint32_t err_code = NRF_SUCCESS;
        nrf_drv_timer_config_t t_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
        t_cfg.mode = NRF_TIMER_MODE_TIMER;
        t_cfg.bit_width = NRF_TIMER_BIT_WIDTH_16;
        t_cfg.frequency = (nrf_timer_frequency_t)4;
        err_code = nrf_drv_timer_init(&SIMPLE_TIMER, &t_cfg, simple_timer_event_handler);

        if (NRF_SUCCESS == err_code)
        {
            simple_timer_state = TIMER_STATE_INITIALIZED;
        }

        return err_code;
    }

    /**
     * @brief Function for starting the timer. Roughly ganked from app_simple_timer.c
     */
    uint32_t simple_timer_start(uint16_t timeout_ticks)
    {
        nrf_timer_short_mask_t timer_short = NRF_TIMER_SHORT_COMPARE0_STOP_MASK;

        if (TIMER_STATE_IDLE == simple_timer_state)
        {
            return NRF_ERROR_INVALID_STATE;
        }

        if (TIMER_STATE_STARTED == simple_timer_state)
        {
            nrf_drv_timer_pause(&SIMPLE_TIMER);
            simple_timer_state = TIMER_STATE_STOPPED;
        }

        if (TIMER_STATE_STOPPED == simple_timer_state)
        {
            nrf_drv_timer_clear(&SIMPLE_TIMER);
        }

        nrf_drv_timer_extended_compare(
                &SIMPLE_TIMER, NRF_TIMER_CC_CHANNEL0, (uint32_t)timeout_ticks, timer_short, true);

        if (simple_timer_state == TIMER_STATE_STOPPED)
        {
            nrf_drv_timer_resume(&SIMPLE_TIMER);
        }
        else
        {
            nrf_drv_timer_enable(&SIMPLE_TIMER);
        }

        simple_timer_state = TIMER_STATE_STARTED;

        return NRF_SUCCESS;
    }
#endif
#if CONTROL == BUTTON
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
            app_pwm_disable(&PWM1);
            ready_flag = true;
        }
        else
        {
            app_pwm_enable(&PWM1);
            duty_cycle = 50;
            ready_flag = false;
        }
    }
#endif

/**
 *@}
 **/
