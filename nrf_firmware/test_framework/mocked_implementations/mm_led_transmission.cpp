/**
file: mm_led_transmission.cpp
brief: Mocking out the mm_led_transmissions.c file so the test program can call the relevant functions without exploding
notes:
*/


/**********************************************************
						INCLUDES
**********************************************************/

extern "C" {
#include "mm_led_transmission.h"
}

/**********************************************************
					   DEFINITIONS
**********************************************************/

void mm_led_transmission_init(void)
{
	// Do nothing
}

void mm_led_transmission_send_led_update
	(
		uint16_t node_id,
		led_function_t current_led_function,
		led_colours_t current_led_colour
	)
{
	// Do nothing
}

