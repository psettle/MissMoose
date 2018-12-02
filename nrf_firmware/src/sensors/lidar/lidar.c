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

/* Register definitions for the grid-eye. */
#define POWER_CONTROL_REGISTER        (0x00U)
#define RESET_REGISTER                (0x01U)
#define FRAMERATE_REGISTER            (0x02U)
#define INT_CONTROL_REGISTER          (0x03U)
#define STATUS_REGISTER               (0x04U)
#define STATUS_CLEAR_REGISTER         (0x05U)
#define AVERAGE_REGISTER              (0x07U)
#define INT_LEVEL_REGISTER_UPPER_LSB  (0x08U)
#define INT_LEVEL_REGISTER_UPPER_MSB  (0x09U)
#define INT_LEVEL_REGISTER_LOWER_LSB  (0x0AU)
#define INT_LEVEL_REGISTER_LOWER_MSB  (0x0BU)
#define INT_LEVEL_REGISTER_HYST_LSB   (0x0CU)
#define INT_LEVEL_REGISTER_HYST_MSB   (0x0DU)
#define THERMISTOR_REGISTER_LSB       (0x0EU)
#define THERMISTOR_REGISTER_MSB       (0x0FU)
#define INT_TABLE_REGISTER_INT0       (0x10U)
#define RESERVED_AVERAGE_REGISTER     (0x1FU)
#define TEMPERATURE_REGISTER_START    (0x80U)

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
    WAITING_FOR_WRITE,      ///< We have written to the temperature register, waiting for transfer complete
    WAITING_FOR_READ,       ///< We have started reading data, waiting for transfer complete
    READING_DONE            ///< We're finished the reading.
}lidar_reading_states_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void lidar_process_distance(void);
static ret_code_t set_register_value(uint8_t reg, uint8_t val);
//static ret_code_t write_register(uint8_t reg);
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);

static void timer_event_handler(void * p_context);

/* Indicates if operation on TWI has ended. */
static volatile bool xfer_done = true;
static uint8_t       rx_buffer[BUFFER_LENGTH];

static uint8_t       frames_per_second = 1;
static lidar_framerate_t lidar_framerate = LIDAR_10FPS; // Framerate setting in lidar format.
static uint8_t       timer_iteration_count = 0;
static bool          pending_framerate_change = false;
static lidar_reading_states_t lidar_reading_state = READING_DONE;

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

/* Timer instance. */
APP_TIMER_DEF(m_lidar_timer);


/**********************************************************
                       DEFINITIONS
**********************************************************/

void lidar_update_main(void)
{
    ret_code_t err_code;
    if(xfer_done)
    {
        switch(lidar_reading_state)
        {
            case READING_START_FLAG:
                // Write to the lidar the register we want to read data off of.
                err_code = set_register_value( 0x00, 0x04 );
                APP_ERROR_CHECK(err_code);
                lidar_reading_state = WAITING_FOR_WRITE;
                break;
            case WAITING_FOR_WRITE:
                // Initiate a reading off the grid-eye.
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
 * @brief
 */
void lidar_set_framerate(lidar_framerate_t new_framerate)
{
    // Check for invalid argument
    if(new_framerate > LIDAR_1FPS)
    {
        return;
    }
    pending_framerate_change = true;
    lidar_framerate = new_framerate;
    frames_per_second = lidar_framerate == LIDAR_10FPS ? 10 : 1;
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

    // while (xfer_done == false);
    return err_code;
}

//static ret_code_t write_register(uint8_t reg)
//{
//    xfer_done = false;
//    ret_code_t err_code;
//    err_code = nrf_drv_twi_tx(&m_twi, LIDAR_ADDR, (uint8_t*)&reg, 1, true);
//
//    // while (xfer_done == false);
//    return err_code;
//}

static void timed_reading_init(void)
{
    ret_code_t err_code;
    err_code = app_timer_create(&m_lidar_timer, APP_TIMER_MODE_REPEATED, timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_lidar_timer, APP_TIMER_TICKS(BASE_FRAMERATE), NULL);
}

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
    if(timer_iteration_count >= (11 - frames_per_second))
    {
        // do a reading!
        lidar_reading_state = READING_START_FLAG;
        timer_iteration_count = 0;
        bsp_board_led_invert(0); // Flashy flashy every time we read a distance value.
    }
}
