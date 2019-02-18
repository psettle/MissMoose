/**********************************************************
                        INCLUDES
**********************************************************/

#include "test_output_logger.hpp"

extern "C" {
#include "mm_led_control.h"
}

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Writes LED output state information to the opened log file.
 * For example, the output would look like:
 *
 * [ AV OUPUT EVENT]
 * [001:02:07:24] Target Node ID: 1
 * [001:02:07:24] LED Function: 0
 * [001:02:07:24] LED Colour: 0
 */
static void log_led_ouput
(
	uint16_t target_node_id,
	led_function_t led_function,
	led_colours_t led_colour
);

/**********************************************************
                       DEFINITIONS
**********************************************************/

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
) 
{
    /* Log the event details. */
    log_led_ouput(target_node_id, led_function, led_colour);
}

/**
 * Writes LED output state information to the opened log file.
 * For example, the output would look like:
 *
 * <timestamp>, LEDOUTPUT EVENT, 1, 0, 0
 * [001:02:07:24] Target Node ID: 1
 * [001:02:07:24] LED Function: 0
 * [001:02:07:24] LED Colour: 0
 */
static void log_led_ouput
(
    uint16_t target_node_id,
    led_function_t led_function,
    led_colours_t led_colour
)
{
    /* Output would be like the above example. */
    log_message("LED OUTPUT EVENT,node," + std::to_string(target_node_id) + ",func," + std::to_string(led_function) + ",colour," + std::to_string(led_colour));
}