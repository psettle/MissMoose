/**********************************************************
                        INCLUDES
**********************************************************/

#include "lidar_pub.h"

#include <stdio.h>
#include <string.h>
#include "boards.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "app_util_platform.h"
#include "app_timer.h"
#include "mm_ant_control.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/* Register definitions for the LIDAR. */
#define DISTANCE_VALUE_REGISTER     (0x8FU)
#define MEASUREMENT_TYPE_REGISTER   (0x00U)
#define RECEIVER_BIS_CORRECTION     (0x04U)

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

#define LIDAR_ADDR                  (0x62U)
#define BUFFER_LENGTH                   (2)
#define BASE_FRAMERATE                (100)

/**
 * @brief States of simple timer state machine.
 */
typedef enum
{
    READING_START_FLAG = 0, ///< When this flag is set, time to start a reading!
    WAITING_FOR_WRITE,      ///< We have told the LIDAR to take a reading, wait for transfer complete
    INITIATE_DISTANCE_READ, ///< We initiated a reading off the LIDAR's distance register, wait for transfer complete
    WAITING_FOR_READ,       ///< We have started reading data, waiting for transfer complete
    READING_DONE            ///< We're finished the reading.
}lidar_reading_states_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void lidar_process_distance(void);
static ret_code_t set_register_value(uint8_t reg, uint8_t val);
static ret_code_t write_register(uint8_t reg);
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);

static void timer_event_handler(void * p_context);

/* Indicates if operation on TWI has ended. */
static volatile bool xfer_done = true;
static uint8_t       rx_buffer[BUFFER_LENGTH];

static uint8_t       samples_per_second = 10;
static uint8_t       timer_iteration_count = 0;
static lidar_reading_states_t lidar_reading_state = READING_DONE;

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

/* Timer instance. */
APP_TIMER_DEF(m_lidar_timer);


/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * @brief Call for updating the data read off the LIDAR.
 */
void lidar_update_main(void)
{
    ret_code_t err_code;
    if(xfer_done)
    {
        switch(lidar_reading_state)
        {
            case READING_START_FLAG:
                // Write to the LIDAR the register we want to read data off of.
                err_code = set_register_value( MEASUREMENT_TYPE_REGISTER, RECEIVER_BIS_CORRECTION );
                APP_ERROR_CHECK(err_code);
                lidar_reading_state = WAITING_FOR_WRITE;
                break;
            case WAITING_FOR_WRITE:
                // Initiate a reading off the LIDAR.
                xfer_done = false;
                err_code = write_register(DISTANCE_VALUE_REGISTER);
                APP_ERROR_CHECK(err_code);
                lidar_reading_state = INITIATE_DISTANCE_READ;
                break;
            case INITIATE_DISTANCE_READ:
                // Initiate a reading off the LIDAR.
                xfer_done = false;
                err_code = nrf_drv_twi_rx(&m_twi, LIDAR_ADDR, rx_buffer, sizeof(rx_buffer));
                APP_ERROR_CHECK(err_code);
                lidar_reading_state = WAITING_FOR_READ;
                break;
            case WAITING_FOR_READ:
                // The data is now all safely tucked away in rx_buffer, so let's do something with it.
                lidar_process_distance();
                lidar_reading_state = READING_DONE;
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
 * @brief UART initialization.
 */
void twi_init(void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_lidar_config = {
       .scl                = 30,
       .sda                = 29,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_lidar_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

/**
 * @brief Processes distance from rx_buffer and broadcasts it over ANT.
 */
static void lidar_process_distance(void)
{
    bsp_board_led_off(2);

    // Shift high byte and add to low byte
    uint32_t distance = (rx_buffer[0] << 8) + rx_buffer[1];

    // Queue the distance and broadcast over ANT.
    mm_ant_payload_t payload;
    memset(&payload.data, 0, sizeof(payload));
    memcpy(&payload.data, &distance, sizeof(distance));

    mm_ant_set_payload(&payload);

    // If the distance is like, greater than 30 cm, LED on
    if (distance > 30)
    {
    	bsp_board_led_on(2);
    }
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

    err_code = app_timer_start(m_lidar_timer, APP_TIMER_TICKS(BASE_FRAMERATE), NULL);
}

/**
 * @brief Initializes LIDAR sensor.
 */
void lidar_init(void)
{
    twi_init();
    timed_reading_init();
}

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            xfer_done = true;
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
    timer_iteration_count++;
    if(timer_iteration_count >= (11 - samples_per_second))
    {
        // do a reading!
        lidar_reading_state = READING_START_FLAG;
        timer_iteration_count = 0;
        bsp_board_led_invert(0); // Flashy flashy every time we read a distance value.
    }
}
