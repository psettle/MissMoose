/**
file: simulate_time.hpp
brief:
notes:
*/
#ifndef SIMULATE_TIME_HPP
#define SIMULATE_TIME_HPP

/**********************************************************
                        INCLUDES
**********************************************************/

extern "C" {
    #include "mm_sensor_transmission.h"
}

/**********************************************************
                        CONSTANTS
**********************************************************/

#define SECONDS_PER_MINUTE      ( 60 )
#define MINUTES_PER_HOUR        ( 60 )
#define HOURS_PER_DAY           ( 24 )

#define ONE_MINUTE_S            ( SECONDS_PER_MINUTE )
#define ONE_HOUR_S              ( ONE_MINUTE_S * MINUTES_PER_HOUR )
#define ONE_DAY_S               ( ONE_HOUR_S * HOURS_PER_DAY )

#define MINUTES(m)              ( (m) * ONE_MINUTE_S )
#define HOURS(h)                ( (h) * ONE_HOUR_S )
#define DAYS(d)                 ( (d) * ONE_DAY_S )

/**********************************************************
                       DECLARATIONS
**********************************************************/

void simulate_time(uint32_t seconds);

#endif /* SIMULATE_TIME_HPP */