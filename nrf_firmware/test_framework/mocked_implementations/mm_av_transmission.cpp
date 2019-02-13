extern "C" {
#include "mm_av_transmission.h"
}

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
) {}
