#include <stdio.h>
#include <time.h>
#include "nrf_drv_gpiote.h"
#include "nrf_drv_systick.h"
#include "boards.h"
#include "lidar_transmit_pub.h"
#include "app_pwm.h"
#include "nrf_delay.h"
#include "mm_ant_control.h"

#define TICKS_PER_MICROSEC 64

//static clock_t current_timestamp = 0;		// Current timestamp in clock ticks
static nrf_drv_systick_state_t current_timestamp;
static nrf_drv_gpiote_pin_t lidar_mode_pin;

/**
 * @brief Function for handling a change in the pin controlling IR transmission.
 *
 * @param[in] pin       The pin hooked up to the GPIOTE interrupt for the control pin.
 * @param[in] action    Polarity setting for the GPIOTE channel.
 */
static void lidar_mode_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if (nrf_drv_gpiote_in_is_set(pin))
	{
		nrf_drv_systick_get(&current_timestamp);
		return;
	}
	if (current_timestamp.time == 0)
	{
		nrf_drv_systick_get(&current_timestamp);
		return;
	}
	uint32_t previous_timestamp = current_timestamp.time;
	nrf_drv_systick_get(&current_timestamp);

	uint32_t distance_measured = (uint32_t)(((double)(current_timestamp.time - previous_timestamp) / (double)TICKS_PER_MICROSEC) / 10.0);

	// Queue the difference up to be broadcast over ANT.
	mm_ant_payload_t payload;
	memset(&payload.data, 0, sizeof(payload));
	memcpy(&payload.data, &distance_measured, sizeof(distance_measured));

	mm_ant_set_payload(&payload);
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
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLDOWN;

    err_code = nrf_drv_gpiote_in_init(lidar_mode_pin, &in_config, lidar_mode_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(lidar_mode_pin, true);
}
