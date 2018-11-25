/**@file
 * @defgroup grideye Sensor
 * @{
 * @ingroup grideye
 *
 * @brief More information about the grideye can be found from this page:
 * https://www.sparkfun.com/products/14607
 *
 * This is a low-resolution 8x8 infrared camera sensor. It communicates data through I2C (or TWI)
 *
 * A diagram of how to wire up this sensor should be made available on google drive.
 *
 */

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdio.h>
#include <string.h>
#include "grideye_pub.h"
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

#define GRIDEYE_ADDR                  (0x69U)
#define BUFFER_LENGTH                 (128)
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
}grideye_reading_states_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void grideye_process_frame(void);
static ret_code_t set_register_value(uint8_t reg, uint8_t val);
static ret_code_t write_register(uint8_t reg);
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);

static void timer_event_handler(void * p_context);

// Function for compression:
void insert_into_bit_array(uint8_t* arr, int32_t* current_byte, int8_t* current_bit, uint8_t value, uint8_t value_bits);

/* Indicates if operation on TWI has ended. */
static volatile bool xfer_done = true;
static uint8_t       rx_buffer[BUFFER_LENGTH];
static float         temperature_array[64];
static uint8_t       temperature_array_u7[64]; // Stores the data for each pixel in a compressed format.
                                        // Each pixel value is increased by 20, then constrained between 0 and 63 degrees.
                                        // Then, add 0.25 (for rounding), and double the number.
                                        // The result is a number than can be halved and reduced by 20 to get the original temperature
                                        // to a precision of 0.5 degrees, as long as it was within -20 and +43
static uint8_t       frames_per_second = 10;
static grideye_framerate_t grideye_framerate = GRIDEYE_10FPS; // Framerate setting in grideye format.
static uint8_t       timer_iteration_count = 0;
static bool          pending_framerate_change = false;
static uint8_t       current_grid_transfer_row_index = 0;
static grideye_reading_states_t grideye_reading_state = READING_DONE;

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

/* Timer instance. */
APP_TIMER_DEF(m_grideye_timer);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void grideye_update_main(void)
{
    ret_code_t err_code;
    if(xfer_done)
    {
        switch(grideye_reading_state)
        {
            case READING_START_FLAG:
                // Write to the grideye the register we want to read data off of.
                err_code = write_register(TEMPERATURE_REGISTER_START);
                APP_ERROR_CHECK(err_code);
                grideye_reading_state = WAITING_FOR_WRITE;
                break;
            case WAITING_FOR_WRITE:
                // Initiate a reading off the grid-eye.
                xfer_done = false;
                err_code = nrf_drv_twi_rx(&m_twi, GRIDEYE_ADDR, rx_buffer, sizeof(rx_buffer));
                APP_ERROR_CHECK(err_code);
                grideye_reading_state = WAITING_FOR_READ;
                break;
            case WAITING_FOR_READ:
                // The data is now all safely tucked away in rx_buffer, so let's do something with it.
                grideye_process_frame();
                grideye_reading_state = READING_DONE;
                break;
            case READING_DONE:
                // Nothing to do!
                break;
            default:
                break;
        };
    }

    if(pending_framerate_change)
    {
        if(xfer_done)
        {
            err_code = set_register_value(FRAMERATE_REGISTER, grideye_framerate);
            APP_ERROR_CHECK(err_code);
        }
    }
}

/**
 * @brief
 */
void grideye_set_framerate(grideye_framerate_t new_framerate)
{
    // Check for invalid argument
    if(new_framerate > GRIDEYE_1FPS)
    {
        return;
    }
    pending_framerate_change = true;
    grideye_framerate = new_framerate;
    frames_per_second = grideye_framerate == GRIDEYE_10FPS ? 10 : 1;
}

/**
 * @brief UART initialization.
 */
void twi_init(void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_grideye_config = {
       .scl                = 30,
       .sda                = 29,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_grideye_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

static void grideye_process_frame(void)
{
    int8_t      tmp;
    uint8_t        i;
    uint32_t    raw = 0;
    float       result;
    bsp_board_led_off(2);
    // loop through all pixel temperature data
    for(i = 0; i < BUFFER_LENGTH; i++)
    {
        // put temperature data into register variable
        tmp = rx_buffer[i];
        // if we are processing the upper byte
        if (i % 2)
        {
            // convert the upper byte of the 12 bit signed value into proper 16 bit
            // signed format, if value is negative
            if (tmp & 0b00001000) tmp |= 0b11111000;
            // add the upper byte into 16 bit register by shifting it 8 bits up
            raw |= tmp << 8;
            // calculate actual pixel temperature in °C
            result = raw * 0.25;
            temperature_array[i/2] = result;


            // We want to cram the value of each pixel into 7 bits, so we can cram
            // the value of 8 pixels into 7 bytes of an ant message, and transfer at 8Hz
            // To transfer a complete frame every second.
            // Assume that all temperatures read are between -20 and 43 degrees.
            // Set pixels outside that range to bet limited by those values.
            float compressed_value = temperature_array[i/2];
            // Add 20.
            compressed_value += 20;

            // Constrain between 0 and 63
            compressed_value = compressed_value > 63.0 ? 63.0 : compressed_value;
            compressed_value = compressed_value < 0.0  ? 0.0  : compressed_value;


            // Add 0.25, then double the pixel values, and convert to a uint8_t
            compressed_value = (compressed_value + 0.25) * 2;

            temperature_array_u7[i/2] = (uint8_t)compressed_value;

            if(temperature_array[i/2] > 26.0)
            {
                bsp_board_led_on(2);
            }
        }
        // if the lower byte is processed
        else
        {
            // erase raw value by setting it to the lower byte value
            raw = 0xFF & tmp;
        }
    }

}

static ret_code_t set_register_value(uint8_t reg, uint8_t val)
{
    ret_code_t err_code;
    xfer_done = false;
    uint8_t reg_val[2] = {reg, val};
    err_code = nrf_drv_twi_tx(&m_twi, GRIDEYE_ADDR, (uint8_t*)&reg_val, 2, true);

    // while (xfer_done == false);
    return err_code;
}

static ret_code_t write_register(uint8_t reg)
{
    xfer_done = false;
    ret_code_t err_code;
    err_code = nrf_drv_twi_tx(&m_twi, GRIDEYE_ADDR, (uint8_t*)&reg, 1, true);

    // while (xfer_done == false);
    return err_code;
}

static void timed_reading_init(void)
{
    ret_code_t err_code;
    err_code = app_timer_create(&m_grideye_timer, APP_TIMER_MODE_REPEATED, timer_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_grideye_timer, APP_TIMER_TICKS(BASE_FRAMERATE), NULL);
}

void grideye_init(void)
{
    uint8_t idx;
    twi_init();
    timed_reading_init();
    // Make sure the temperature_array is all zero
    for(idx = 0; idx < 8; idx++)
    {
        temperature_array[idx] = 0;
    }
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
        grideye_reading_state = READING_START_FLAG;
        timer_iteration_count = 0;
        bsp_board_led_invert(0); // Flashy flashy every time we take a frame.
    }
}

void grideye_ant_event_handler(ant_evt_t * p_ant_evt)
{
    int32_t current_byte = 1;
    int8_t current_bit = 7;

    uint8_t transfer_count;
    uint8_t current_pixel;
    mm_ant_payload_t payload;

    bsp_board_led_invert(1);

    switch (p_ant_evt->event)
    {
        case EVENT_TX:
            // Time to send the next row!
            bsp_board_led_invert(3);

            memset(&payload, 0, sizeof(payload));
            // Roll over the index of the row we're transferring, if necessary.
            if(current_grid_transfer_row_index > 7)
            {
                current_grid_transfer_row_index = 0;
            }
            payload.data[0] = current_grid_transfer_row_index;

            for(transfer_count = 0; transfer_count < 8; transfer_count++)
            {
                current_pixel = temperature_array_u7[current_grid_transfer_row_index*8 + transfer_count];

                insert_into_bit_array(payload.data, &current_byte, &current_bit, current_pixel, 7);

            }

            current_grid_transfer_row_index++;

            mm_ant_set_payload(&payload);
            break;
    }

}


/**
 * This is a specialized function which inserts the first "value_bits" of "value" into an array of bytes "arr"
 * starting at the "current_bit" of the "current_byte".
 *
 * EXAMPLE: if we passed a value, say 9, into this function, specifying we wanted to write value_bits=4 of it into arr,
 *             at current_bit = 5 (little-endian style) of current_byte = 4, that would look like this:
 *
 *                                          Start writing here.
 *                                          v
 *     arr:         ..[X|X|X|X|X|X|X|X]_[X|X|1|0|0|1|X|X]_[X|X|X|X|X|X|X|X]_[X|X|X|X|X|X|X|X]_[X|X|X|X|X|X|X|X]_...
 *     bitIndex:      [7|6|5|4|3|2|1|0]_[7|6|5|4|3|2|1|0]_[7|6|5|4|3|2|1|0]_[7|6|5|4|3|2|1|0]_[7|6|5|4|3|2|1|0]_
 *     byteIndex:             [3]               [4]                 [5]               [6]                 [7]
 *
 *     As a result of any operations, current_byte and current_bit are set to the next available place to write to the array arr.
 *     in the above example, after execution current_byte = 4 and current_bit = 1.
 *
 *     It should also be possible for an input to spread out over the end of one byte and the start of the next. for example,
 *     if in the previous example current_bit = 1, the result would look like this:
 *
 *                                                   Start writing here.
 *                                                   v
 *     arr:         ..[X|X|X|X|X|X|X|X]_[X|X|X|X|X|X|1|0]_[0|1|X|X|X|X|X|X]_[X|X|X|X|X|X|X|X]_[X|X|X|X|X|X|X|X]_...
 *     bitIndex:      [7|6|5|4|3|2|1|0]_[7|6|5|4|3|2|1|0]_[7|6|5|4|3|2|1|0]_[7|6|5|4|3|2|1|0]_[7|6|5|4|3|2|1|0]_
 *     byteIndex:             [3]               [4]                 [5]               [6]                 [7]
 */
void insert_into_bit_array(uint8_t* arr, int32_t* current_byte, int8_t* current_bit, uint8_t value, uint8_t value_bits)
{
    // the PLAN: change value into a short
    uint16_t bigValue = value;

    // shift the short value until it's overlapping with the space it needs to occupy
    bigValue = bigValue << (8 + *current_bit - (value_bits-1));

    // Write the High part into the current_byte, and the Low part into the next byte.
    // Note that since we're bitwise OR-ing the values in, this function relies on
    // the array values being initialized to 0.
    arr[*current_byte] |= (uint8_t) (bigValue >> 8);
    arr[*current_byte+1] |= (uint8_t) (bigValue & 0x00FF);

    // increment the current_byte and change current_bit appropriately.
    if(*current_bit - value_bits < 0){
        *current_byte += 1;
    }
    *current_bit = ((*current_bit - value_bits) % 8);
    // The % operator in C does not work how one might expect...
    if (*current_bit < 0)
    {
        *current_bit += 8;
    }
}

/**
 *@}
 **/
