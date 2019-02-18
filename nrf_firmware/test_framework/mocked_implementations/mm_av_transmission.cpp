/**********************************************************
                        INCLUDES
**********************************************************/

#include "test_output_logger.hpp"

extern "C" {
#include "mm_av_transmission.h"
}

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Writes AV output information to the opened log file.
 * For example, the output would look like:
 * 
 * [ AV OUPUT EVENT]
 * [001:02:07:24] AV Position X: 2
 * [001:02:07:24] AV Position Y: 2
 * [001:02:07:24] AV Value: 0
 * [001:02:07:24] AV Status X: 0
 */
static void log_av_ouput
(
    uint8_t av_position_x,
    uint8_t av_position_y,
    float av_value,
    uint8_t av_status
);

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * Initialize activity variable state transmission over ANT.
 */
void mm_av_transmission_init(void) {}

/**
 * Broadcast latest activity variable state over ANT.
 */
void mm_av_transmission_send_av_update
(
    uint8_t av_position_x,
    uint8_t av_position_y,
    float av_value,
    uint8_t av_status
) 
{
    /* Log the event details. */
    log_av_ouput(av_position_x, av_position_y, av_value, av_status);
}

/**
 * Writes AV output information to the opened log file.
 * For example, the output would look like:
 * 
 * [ AV OUPUT EVENT]
 * [001:02:07:24] AV Position X: 2
 * [001:02:07:24] AV Position Y: 2
 * [001:02:07:24] AV Value: 0
 * [001:02:07:24] AV Status X: 0
 */
static void log_av_ouput
(
    uint8_t av_position_x,
    uint8_t av_position_y,
    float av_value,
    uint8_t av_status
)
{
    /* Output would be like the above example. */
	log_heading("AV OUTPUT EVENT");
    log_message(std::string("AV Position X: ") + std::to_string(av_position_x));
    log_message(std::string("AV Position Y: ") + std::to_string(av_position_y));
    log_message(std::string("AV Value: ") + std::to_string(av_value));
    log_message(std::string("AV Status: ") + std::to_string(av_status));
}