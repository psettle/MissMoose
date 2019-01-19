/*
 * sensor_manager.c
 *
 *  Created on: Jan 16, 2019
 *      Author: nataliewong
 */

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_sensor_manager.h"

#include <string.h>

#include "mm_switch_config.h"
#include "mm_sensor_transmission.h"
#include "pir_st_00081_pub.h"
#include "lidar_pub.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void process_lidar_evt(lidar_evt_t * evt);

static void process_pir_evt(pir_evt_t * evt);

/**********************************************************
                        VARIABLES
**********************************************************/

static hardware_config_t hardware_config = HARDWARE_CONFIG_UNKNOWN;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_sensor_manager_init( bool led_debug_enabled )
{
	hardware_config = read_hardware_config();

    switch(hardware_config)
    {
        case HARDWARE_CONFIG_PIR_PIR:
            pir_st_00081_init(2, led_debug_enabled);

            /* Set ourselves up as a listener for the pir sensors */
            pir_evt_handler_register(process_pir_evt);
            break;

        case HARDWARE_CONFIG_PIR_LIDAR:
            pir_st_00081_init(1, led_debug_enabled);
            lidar_init(led_debug_enabled);

            /* Set ourselves up as a listener for the pir sensors */
            pir_evt_handler_register(process_pir_evt);

            /* Set ourselves up as a listener for the lidar */
            lidar_evt_handler_register(process_lidar_evt);
            break;

        case HARDWARE_CONFIG_PIR_LIDAR_LED:
            pir_st_00081_init(1, led_debug_enabled);
            lidar_init(true);

            /* Set ourselves up as a listener for the lidar */
            lidar_evt_handler_register(process_lidar_evt);

            /* Set ourselves up as a listener for the pir sensors */
            pir_evt_handler_register(process_pir_evt);
            break;

        default:
        	APP_ERROR_CHECK(true);
    }
}

void mm_sensor_manager_main( void )
{
    switch(hardware_config)
    {
        case HARDWARE_CONFIG_PIR_PIR:
        	pir_update_main();
            break;

        case HARDWARE_CONFIG_PIR_LIDAR:
        case HARDWARE_CONFIG_PIR_LIDAR_LED:
        	lidar_update_main();
        	pir_update_main();
            break;

        default:
        	APP_ERROR_CHECK(true);
    }
}

static void process_pir_evt(pir_evt_t * evt)
{
	sensor_rotation_t sensor_rotation = SENSOR_ROTATION_0;

	switch (evt->sensor_index)
	{
		case 0:
			if ( hardware_config == HARDWARE_CONFIG_PIR_PIR)
			{
				sensor_rotation = SENSOR_ROTATION_270;
			}
			else
			{
				sensor_rotation = SENSOR_ROTATION_90;
			}
			break;
		case 1:
			sensor_rotation = SENSOR_ROTATION_0;
			break;
		default:
			APP_ERROR_CHECK(true);
			break;

	}

	switch (evt->event)
    {
        case PIR_EVENT_CODE_DETECTION:
        	mm_send_pir_data( sensor_rotation, true );
            break;
        case PIR_EVENT_CODE_NO_DETECTION:
        	mm_send_pir_data( sensor_rotation, false );
        	break;
        default:
            break;
    }
}

static void process_lidar_evt(lidar_evt_t * evt)
{
    switch (evt->event)
    {
        case LIDAR_EVENT_CODE_MEASUREMENT_SETTLE:
        	mm_send_lidar_data( SENSOR_ROTATION_0, evt->distance );
            break;

        default:
            break;
    }
}
