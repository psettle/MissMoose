/**
file: simulate_time.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdint.h>

extern "C" {
#include "mm_sensor_algorithm.h"
}

/**********************************************************
                        VARIABLES
**********************************************************/

/* Keeps track of the number of seconds elapsed in simulated test time. */
uint32_t seconds_elapsed;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void simulate_time(uint32_t seconds)
{
    /* We just run the clock as fast as possible, since there are no other events to process concurrently. */
    for (uint32_t i = 0; i < seconds; ++i)
    {
        mm_sensor_algorithm_on_second_elapsed();
		++seconds_elapsed;
    }
}

uint32_t get_simulated_time_elapsed()
{
	return seconds_elapsed;
}