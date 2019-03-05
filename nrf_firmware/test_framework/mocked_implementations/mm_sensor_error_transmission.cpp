/**
file: mm_led_transmission.cpp
brief: Mocking out the mm_sensor_error_transmission.c file so the test program can call the relevant functions without exploding
notes:
*/


/**********************************************************
						INCLUDES
**********************************************************/

extern "C" {
#include "mm_sensor_error_transmission.h"
}

/**********************************************************
					   DEFINITIONS
**********************************************************/

void mm_sensor_error_transmission_init(void)
{
	// Do nothing
}

void mm_sensor_error_transmission_send_hyperactivity_update
(
	uint16_t node_id,
	sensor_type_t sensor_type,
	sensor_rotation_t sensor_rotation,
	bool error_occuring
)
{
	// Do nothing
}

void mm_sensor_error_transmission_send_inactivity_update
(
	uint16_t node_id,
	sensor_type_t sensor_type,
	sensor_rotation_t sensor_rotation,
	bool error_occuring
)
{
	// Do nothing
}

