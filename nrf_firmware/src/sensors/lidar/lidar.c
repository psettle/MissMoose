/**********************************************************
                        INCLUDES
**********************************************************/

#include "lidar_pub.h"

#include <string.h>
#include "boards.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "app_util_platform.h"
#include "app_timer.h"
#include "mm_ant_control.h"
#include "nrf_drv_gpiote.h"
#include "app_scheduler.h"

/**********************************************************
                        CONSTANTS
**********************************************************/
#define MEDIAN(a,b,c) ((a > b) ? (b > c) ? b : (a > c) ? c : a : \
                       (b > c) ? (a > c) ? a : c : b)

#define ANT_BROADCAST_DEBUG         false // If true, broadcast the most recent filtered sample over ANT.
#define USE_POWER_CYCLING           true  // Whether or not we should try and turn the lidar off between samplings.

/* Register definitions for the LIDAR. */
#define DISTANCE_VALUE_REGISTER     (0x8FU)
#define MEASUREMENT_TYPE_REGISTER   (0x00U)
#define RECEIVER_BIAS_CORRECTION    (0x04U)

/* TWI instance ID. */
#define TWI_INSTANCE_ID             0

#define LIDAR_TWI_SDA_PIN           29 ///< J102.11
#define LIDAR_TWI_SCL_PIN           30 ///< J102.13
#define LIDAR_ENABLE_PIN            4  ///< J102.18

#define LIDAR_ADDR                  (0x62U)
#define BUFFER_LENGTH               (2)
#define ZEROS_TO_ROLL_OUT           (6)  // Maximum number of times to take unused measurements to roll out the zeros from the lidar.
#define CONTROL_TIMER_PERIOD        (2)  // ms. Using 2 ms so that we can take a bunch of measurements back to
                                         // back really quickly on startup and roll the 0's out of the lidar.

#define MEDIAN_FILTER_SIZE          (3)  // Store the last 3 samples.
#define MEDIAN_FILTER_DIFF_THRES    (30) // How different 2 consecutive median-filtered samples have to be noted as a detection.

#define MAX_EVT_HANDLERS            ( 4 )

/**
 * @brief States of the twi reading/writing state machine.
 */
typedef enum
{
    READING_START_FLAG = 0, ///< When this flag is set, time to start a reading!
    WAITING_FOR_WRITE,      ///< We have told the LIDAR to take a reading, wait for transfer complete
    INITIATE_DISTANCE_READ, ///< We initiated a reading off the LIDAR's distance register, wait for transfer complete
    WAITING_FOR_READ,       ///< We have started reading data, waiting for transfer complete
    READING_DONE            ///< We're finished the reading.
}lidar_twi_states_t;

/**
 * @brief States of the lidar's weird sampling rigmarole.
 */
typedef enum
{
    NOT_SAMPLING = 0,   ///< In this state when the enable pin is pulled low and the lidar is off
    ROLLING_OUT_ZEROS,  ///< In this state after first turning on the lidar, and trying to get rid of 0 measurements
    WARMUP_WAITING,     ///< In this state after getting rid of the zero measurements, but waiting for the full warmup period as
                        ///< recommended in the LIDAR datasheets. No measurements are done during this time, to save power.
    SAMPLING,           ///< In this state until we get a successful non-0 reading after deliberately trying to roll the 0s out. and
                        ///< waiting the warmup period.
                        ///< If we transition back to ROLLING_OUT_ZEROS in order to start taking the next measurement while still
                        ///< In this phase, that should cause some flag that the most recent lidar measurement failed.
    SAMPLE_ERROR        ///< Indicates there's been an error in sampling.
}lidar_sampling_states_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

void lidar_event_dispatch(uint16_t distance, lidar_event_type_t event);
static void lidar_process_distance(void);
static ret_code_t set_register_value(uint8_t reg, uint8_t val);
static ret_code_t write_register(uint8_t reg);
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);
void twi_reset(void);
/* Processes timer timeout */
static void on_timer_event(void* evt_data, uint16_t evt_size);

static void timer_event_handler(void * p_context);

/**********************************************************
                        VARIABLES
**********************************************************/

/* Whether or not to utilize on-board LEDs for debugging. */
static bool led_debug;

static uint8_t sample_readings = 0;
static lidar_error_code_type_t error_code;

static uint16_t sample_buffer[MEDIAN_FILTER_SIZE];
static uint16_t median_filtered_distance;
static uint8_t  median_filter_index;
static bool     median_filter_warmup;
static bool     distance_changed_warning_flag;

static lidar_evt_handler_t lidar_evt_handlers[MAX_EVT_HANDLERS];

// How many times a 2ms timer must elapse before we trigger a reading.
static const uint16_t rate_to_2ms_periods_conversion[LIDAR_SAMPLE_RATE_COUNT] = {25,50,100,125,250,500};
static uint16_t periods_between_sample_acquisition;

/* State control variables for sampling. */
static uint16_t                 timer_iteration_count = 0;
static lidar_twi_states_t       lidar_twi_state = READING_DONE;
static lidar_sampling_states_t  lidar_sampling_state = NOT_SAMPLING;

/* Indicates if operation on TWI has ended. */
static volatile bool xfer_done = true;
static uint8_t       rx_buffer[BUFFER_LENGTH];

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
/* TWI config */
static const nrf_drv_twi_config_t twi_lidar_config = {
            .scl                = LIDAR_TWI_SCL_PIN,
            .sda                = LIDAR_TWI_SDA_PIN,
            .frequency          = NRF_TWI_FREQ_400K,
            .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
            .clear_bus_init     = false
            };

/* Timer instance. */
APP_TIMER_DEF(m_lidar_timer);


/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * @brief Responsible for progressing the state of reading information off the lidar.
 */
static void lidar_update_process(void* evt_data, uint16_t evt_size)
{
    ret_code_t err_code = NRF_SUCCESS;
    if(xfer_done)
    {
        switch(lidar_twi_state)
        {
            case READING_START_FLAG:
                // Write to the LIDAR the register we want to read data off of.
                lidar_twi_state = WAITING_FOR_WRITE;
                err_code = set_register_value( MEASUREMENT_TYPE_REGISTER, RECEIVER_BIAS_CORRECTION );
                if(err_code == NRF_ERROR_BUSY)
                {
                    /* The TWI driver is busy. Let's miss this measurement and try again next time. */
                    err_code = NRF_SUCCESS; // Excuse this particular error.
                    twi_reset();
                }
                APP_ERROR_CHECK(err_code);
                break;
            case WAITING_FOR_WRITE:
                // Write to the LIDAR what register we want to do a reading from.
                xfer_done = false;
                lidar_twi_state = INITIATE_DISTANCE_READ;
                err_code = write_register(DISTANCE_VALUE_REGISTER);
                if(err_code == NRF_ERROR_BUSY)
                {
                    /* The TWI driver is busy. Let's miss this measurement and try again next time. */
                    err_code = NRF_SUCCESS; // Excuse this particular error.
                    twi_reset();
                }
                APP_ERROR_CHECK(err_code);
                break;
            case INITIATE_DISTANCE_READ:
                // Initiate a reading off the LIDAR.
                xfer_done = false;
                lidar_twi_state = WAITING_FOR_READ;
                err_code = nrf_drv_twi_rx(&m_twi, LIDAR_ADDR, rx_buffer, sizeof(rx_buffer));
                if(err_code == NRF_ERROR_BUSY)
                {
                    /* The TWI driver is busy. Let's miss this measurement and try again next time. */
                    err_code = NRF_SUCCESS; // Excuse this particular error.
                    twi_reset();
                }
                APP_ERROR_CHECK(err_code);
                break;
            case WAITING_FOR_READ:
                // The data is now all safely tucked away in rx_buffer, so let's do something with it.
                lidar_process_distance();
                lidar_twi_state = READING_DONE;
                sample_readings++;
                break;
            case READING_DONE:
                // Nothing to do!
                break;
            default:
                break;
        };
    }
}

/**
 * @brief Reset the TWI instance, this should be done after
 * hitting a BUSY error in the twi interface.
 */
void twi_reset(void)
{
    ret_code_t err_code;
    lidar_twi_state = READING_DONE;
    lidar_sampling_state = SAMPLE_ERROR;
    nrf_drv_twi_uninit(&m_twi);
    err_code = nrf_drv_twi_init(&m_twi, &twi_lidar_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

/**
 * @brief UART initialization.
 */
void twi_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_twi_init(&m_twi, &twi_lidar_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

/**
 * @brief Set the sample rate! Must choose from one of the values in the lidar_sample_rate_type_t enum.
 */
void lidar_set_sample_rate(lidar_sample_rate_type_t rate)
{
    periods_between_sample_acquisition = rate_to_2ms_periods_conversion[rate];
}

/**
 * @brief Processes distance from rx_buffer and broadcasts it over ANT.
 */
static void lidar_process_distance(void)
{
    // Shift high byte and add to low byte
    uint16_t distance = (rx_buffer[0] << 8) + rx_buffer[1];
    uint16_t new_filtered_distance;

    if(lidar_sampling_state == ROLLING_OUT_ZEROS)
    {
        // If the distance is greater than 0, we're done rolling out the zeros.
        if (distance > 0)
        {
            lidar_sampling_state = WARMUP_WAITING;
        }
    }

    if(lidar_sampling_state == SAMPLING)
    {
        // If the distance is exactly zero even after rolling out the zeros
        // The lidar may be reading a very close distance or a very far distance,
        // but it's more likely that the reading is not correct. To mitigate, just
        // continue to output the last filtered distance.
        if (distance == 0)
        {
            distance = median_filtered_distance;
        }
        // Add the measurement to the median filter sample buffer
        sample_buffer[median_filter_index] = distance;
        median_filter_index++;
        if(median_filter_index >= MEDIAN_FILTER_SIZE)
        {
            median_filter_index = 0;
            // Only if we're just starting and our filters haven't filled out yet, assign the median of the buffer to
            // median_filtered_distance right away.
            if(!median_filter_warmup)
            {
                median_filtered_distance = MEDIAN(sample_buffer[0], sample_buffer[1], sample_buffer[2]);
                median_filter_warmup = true;
            }
        }

        if(median_filter_warmup)
        {
            // If the filter has been warmed up, check and see how different the last filtered sample is to the most recent filtered sample.
            // If there's been a significant enough difference, flag that!
            new_filtered_distance = MEDIAN(sample_buffer[0], sample_buffer[1], sample_buffer[2]); // Ok to be honest i'm not even sure why i bothered making everything else dynamic because in the end it's just gonna be a median of 3 samples anyway??? smh.
            if(abs(new_filtered_distance - median_filtered_distance) > MEDIAN_FILTER_DIFF_THRES)
            {
                if(led_debug)
                {
                    bsp_board_led_on(1);
                }
                if(distance_changed_warning_flag != true)
                {
                    // The distance has suddenly changed when it was stable.
                    lidar_event_dispatch(new_filtered_distance, LIDAR_EVENT_CODE_MEASUREMENT_CHANGE);
                }
                distance_changed_warning_flag = true;
            }
            else
            {
                if(led_debug)
                {
                    bsp_board_led_off(1);
                }
                if(distance_changed_warning_flag != false)
                {
                    // The distance has now become stable.
                    lidar_event_dispatch(new_filtered_distance, LIDAR_EVENT_CODE_MEASUREMENT_SETTLE);
                }
                distance_changed_warning_flag = false;
            }
            median_filtered_distance = new_filtered_distance;

            #if(ANT_BROADCAST_DEBUG)
                // Queue the distance and broadcast over ANT.
                mm_ant_payload_t payload;
                memset(&payload.data, 0, sizeof(payload));
                memcpy(&payload.data, &distance, sizeof(median_filtered_distance));
                mm_ant_set_payload(&payload);
            #endif
        }

        lidar_sampling_state = NOT_SAMPLING;
        #if(USE_POWER_CYCLING)
            nrf_drv_gpiote_out_clear(LIDAR_ENABLE_PIN); // Pull the power enable pin low to save power
        #endif
        error_code = LIDAR_ERROR_NONE;
    }
}

/**
 * @brief Tell the listeners that there's recently been a change in the distance,
 *        and whether the distance is just becoming unstable or settling.
 */
void lidar_event_dispatch(uint16_t distance, lidar_event_type_t event)
{
    lidar_evt_t lidar_event;
    lidar_event.distance = distance;
    lidar_event.event = event;
    // Forward LIDAR event to listeners
    for (uint32_t i = 0; i < MAX_EVT_HANDLERS; i++)
    {
        if (lidar_evt_handlers[i] != NULL)
        {
            lidar_evt_handlers[i](&lidar_event);
        }
        else
        {
            break;
        }
    }
}

/**
 * @brief Add a new lidar event listener!
 *
 * @param[in] lidar_evt_handler The event handler to add as a listener.
 */
void lidar_evt_handler_register(lidar_evt_handler_t lidar_evt_handler)
{
    uint32_t i;

    for (i = 0; i < MAX_EVT_HANDLERS; i++)
    {
        if (lidar_evt_handlers[i] == NULL)
        {
            lidar_evt_handlers[i] = lidar_evt_handler;
            break;
        }
    }

    APP_ERROR_CHECK(i == MAX_EVT_HANDLERS);
}

/**
 * @brief Check if there's been an alarming change in distance recently
 *
 */
bool lidar_get_distance_change_flag(void)
{
    return distance_changed_warning_flag;
}

/**
 * @brief Get the most recent median-filtered sample from the lidar.
 *
 */
uint16_t lidar_get_filtered_distance(void)
{
    return median_filtered_distance;
}

static ret_code_t set_register_value(uint8_t reg, uint8_t val)
{
    ret_code_t err_code;
    xfer_done = false;
    uint8_t reg_val[2] = {reg, val};
    err_code = nrf_drv_twi_tx(&m_twi, LIDAR_ADDR, (uint8_t*)&reg_val, 2, true);

    return err_code;
}

static ret_code_t write_register(uint8_t reg)
{
   xfer_done = false;
   ret_code_t err_code;
   err_code = nrf_drv_twi_tx(&m_twi, LIDAR_ADDR, (uint8_t*)&reg, 1, true);

   return err_code;
}

/**
 * @brief Starts periodic distance readings.
 */
static void timed_reading_init(void)
{
    ret_code_t err_code;
    err_code = app_timer_create(&m_lidar_timer, APP_TIMER_MODE_REPEATED, timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_lidar_timer, APP_TIMER_TICKS(CONTROL_TIMER_PERIOD), NULL);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Initializes LIDAR sensor.
 * @param[in] use_led_debug   Whether or not to use the on-board LEDs for debugging.
 */
void lidar_init(bool use_led_debug)
{
    led_debug = use_led_debug;
    timer_iteration_count = 0;
    lidar_twi_state = READING_DONE;
    lidar_sampling_state = NOT_SAMPLING;
    memset(sample_buffer, 0, sizeof(sample_buffer));
    median_filtered_distance = 0;
    median_filter_index = 0;
    median_filter_warmup = false;
    ret_code_t err_code;
    periods_between_sample_acquisition = rate_to_2ms_periods_conversion[LIDAR_SAMPLE_RATE_10Hz];
    twi_init();
    timed_reading_init();


    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

    err_code = nrf_drv_gpiote_out_init(LIDAR_ENABLE_PIN, &out_config);
    APP_ERROR_CHECK(err_code);
    #if(!(USE_POWER_CYCLING))
        nrf_drv_gpiote_out_set(LIDAR_ENABLE_PIN); // Pull the power enable pin low
    #endif

    xfer_done = true;
}

/**
 * @brief Deinitializes the LIDAR sensor. NOTE - Has not been tested yet!
 */
void lidar_deinit(void)
{
    app_timer_stop(m_lidar_timer);
    nrf_drv_twi_disable(&m_twi);
}

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    uint32_t err_code;
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            xfer_done = true;
            /* The transfer is done, so schedule an update for processing. */
            err_code = app_sched_event_put(NULL, 0, lidar_update_process);
            APP_ERROR_CHECK(err_code);
            break;
        default:
            break;
    }
}

/**
 * @brief Handler for timer events.
 */
static void timer_event_handler(void * p_context)
{
    uint32_t err_code;

    /* Kick timer event to main, since there's a bunch of little steps to do on timer events. */
    err_code = app_sched_event_put(NULL, 0, on_timer_event);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief The bulk of processing that needs to be done on timer events.
 */
static void on_timer_event(void* evt_data, uint16_t evt_size)
{
    timer_iteration_count++;
    if(timer_iteration_count >= periods_between_sample_acquisition)
    {
        if(led_debug)
        {
            bsp_board_led_invert(2); // Flashy flashy every time we start a new sample acquisition sequence.
        }
        timer_iteration_count = 0;
        if(lidar_sampling_state != NOT_SAMPLING)
        {
            error_code = LIDAR_ERROR_SAMPLING_FAIL;
            if(led_debug)
            {
                bsp_board_led_on(3);
            }
        }
        else
        {
            // The error has been resolved.
            error_code = LIDAR_ERROR_NONE;
            if(led_debug)
            {
                bsp_board_led_off(3);
            }
        }

        #if(USE_POWER_CYCLING)
            lidar_sampling_state = ROLLING_OUT_ZEROS;
        #else
            lidar_sampling_state = SAMPLING;
        #endif
        sample_readings = 0;
        #if(USE_POWER_CYCLING)
            nrf_drv_gpiote_out_set(LIDAR_ENABLE_PIN); // Stop pulling the power enable pin low
        #endif
    }
    switch(lidar_sampling_state)
    {
        case NOT_SAMPLING:
        case SAMPLE_ERROR:
            // do nothing.
            break;

        case ROLLING_OUT_ZEROS:
            // Take measurements until we stop getting 0's.
            xfer_done = true;
            lidar_twi_state = READING_START_FLAG;
            if(sample_readings >= ZEROS_TO_ROLL_OUT)
            {
                // Stop rolling out the zeros.
                lidar_sampling_state = SAMPLING;
            }
            lidar_update_process(NULL, 0);
            break;

        case WARMUP_WAITING:
            // Waiting the full 22ms warmup as recommended in the datasheets. No measurements are done, to save power.
            if(timer_iteration_count >= 10) // by now, 22ms. Should be enough time for the lidar to warm up.
            {

                lidar_sampling_state = SAMPLING;
            }
            break;

        case SAMPLING:
            xfer_done = true;
            lidar_twi_state = READING_START_FLAG;
            lidar_update_process(NULL, 0);
            // Just make sure we're starting another reading if we're reaching this update
            // It means that the most recent reading was a 0.
            break;
    }
}
