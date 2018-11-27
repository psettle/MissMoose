#include <stdio.h>
#include <time.h>
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "lidar_transmit_pub.h"
#include "app_pwm.h"
#include "nrf_delay.h"
#include "mm_ant_control.h"

#define MICROSECONDS_PER_SEC 1000000

static clock_t current_timestamp = 0;		// Current timestamp in clock ticks
static nrf_drv_gpiote_pin_t lidar_mode_pin;

/**
 * @brief Function for handling a change in the pin controlling IR transmission.
 *
 * @param[in] pin       The pin hooked up to the GPIOTE interrupt for the control pin.
 * @param[in] action    Polarity setting for the GPIOTE channel.
 */
static void lidar_mode_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if (current_timestamp == 0)
	{
		// Get the current clock time in clock ticks...
		current_timestamp = clock();
	}
	else
	{
		// Computer difference in time in microseconds...
		clock_t previous_timestamp = current_timestamp;
		current_timestamp = clock();

		if (previous_timestamp == current_timestamp)
		{
			bsp_board_led_on(0);
		}
		else
		{
			bsp_board_led_off(0);
		}

//		uint32_t time_difference =
//				(uint32_t) ((double)(current_timestamp - previous_timestamp) / CLOCKS_PER_SEC) * MICROSECONDS_PER_SEC;

		// Queue the difference up to be broadcast over ANT.
		mm_ant_payload_t payload;
		memset(&payload.data, 0, sizeof(payload));
		//memcpy(&payload.data, &time_difference, sizeof(time_difference));
		memcpy(&payload.data, &current_timestamp, sizeof(current_timestamp));
		memcpy(&(payload.data[4]), &previous_timestamp, sizeof(previous_timestamp));

		mm_ant_set_payload(&payload);
	}
}

/**
 * @brief Function for initializing the LIDAR controller, utilizing the PWM library.
 *
 * @param[in] ctrl_pin   A pin for controlling when data should be read from the Mode pin of the LIDAR sensor.
 */
void lidar_transmit_init(nrf_drv_gpiote_pin_t mode_pin)
{
    ret_code_t err_code;
    lidar_mode_pin = mode_pin;

    /* Sense changes in either direction on the control pin. Pull up the pin,
     * since we expect to use an IO board button for this. */
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_PULLDOWN;

    err_code = nrf_drv_gpiote_in_init(lidar_mode_pin, &in_config, lidar_mode_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(lidar_mode_pin, true);
}
