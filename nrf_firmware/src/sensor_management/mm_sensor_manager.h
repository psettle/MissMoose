/*
 * sensor_manager.h
 *
 *  Created on: Jan 16, 2019
 *      Author: nataliewong
 */

#ifndef SRC_SENSOR_MANAGEMENT_SENSOR_MANAGER_H_
#define SRC_SENSOR_MANAGEMENT_SENSOR_MANAGER_H_

/**********************************************************
                        INCLUDES
**********************************************************/

#include "nrf_soc.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/* @brief Initializes the sensors associated with this node's
 * hardware configuration and begins listening to and
 * processing sensor data events.*/
void mm_sensor_manager_init( bool led_debug_enabled );

#endif /* SRC_SENSOR_MANAGEMENT_SENSOR_MANAGER_H_ */
