/*
 * mm_switch_config.h
 *
 *  Created on: Jan 16, 2019
 *      Author: nataliewong
 */

#ifndef MM_SWITCH_CONFIG_H_
#define MM_SWITCH_CONFIG_H_

/**********************************************************
                        INCLUDES
**********************************************************/

#include "stdint.h"
#include "stdbool.h"

/**********************************************************
                         ENUMS
**********************************************************/

typedef enum
{
	HARDWARE_CONFIG_UNKNOWN			= 0x00,
	HARDWARE_CONFIG_PIR_PIR         = 0x01,	/* Switch 0 & 1 off */
	HARDWARE_CONFIG_PIR_LIDAR 	    = 0x02, /* Switch 0 on, switch 1 off */
	HARDWARE_CONFIG_PIR_LIDAR_LED   = 0x03  /* Switch 0 off, switch 1 on */
} hardware_config_t;

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/* Initialize switches for the switch configuration */
void mm_switch_config_init(void);

/* Read the dip switches to fetch the hardware configuration */
hardware_config_t read_hardware_config(void);

#endif /* MM_SWITCH_CONFIG_H_ */
