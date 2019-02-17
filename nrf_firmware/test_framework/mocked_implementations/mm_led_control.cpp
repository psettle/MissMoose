extern "C" {
#include "mm_led_control.h"
}

/**
 * Initialize led control over blaze.
 */
void mm_led_control_init(void) {}

/**
 * Set led state for target node.
 */
void mm_led_control_update_node_leds
(
    uint16_t target_node_id,
    led_function_t led_function,
    led_colours_t led_colour
) {}