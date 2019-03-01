/*
 * mm_switch_config.c
 *
 *  Created on: Jan 16, 2019
 *      Author: nataliewong
 */

/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_switch_config.h"

#include <string.h>

#include "mm_ant_control.h"
#include "mm_blaze_control.h"

#include "bsp.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"

#include "app_timer.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       VARIABLES
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_switch_config_init(void)
{
	//Initializes the battery board switches as inputs.
	 for (uint32_t pin = 0; pin < 32; pin++)
	 {
		 if((SWITCHES_MASK) & (1 << pin))
		 {
			 nrf_gpio_cfg_input(pin, SWITCH_PULL);
		 }
	 }
}

hardware_config_t read_hardware_config(void)
{
	hardware_config_t config = HARDWARE_CONFIG_UNKNOWN;

    config = (nrf_gpio_pin_read(SWITCH_1) << 0) |
             (nrf_gpio_pin_read(SWITCH_2) << 1);

    /* Error check config for known types. */
    switch(config)
    {
    case HARDWARE_CONFIG_PIR_PIR:
    case HARDWARE_CONFIG_PIR_LIDAR:
    case HARDWARE_CONFIG_PIR_LIDAR_LED:
    	break;
    case HARDWARE_CONFIG_UNKNOWN:
    default:
    	APP_ERROR_CHECK(true);
    }

    return HARDWARE_CONFIG_PIR_PIR;
}
