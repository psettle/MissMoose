/**********************************************************
                        INCLUDES
**********************************************************/

#include "test_output_logger.hpp"
#include "test_output.hpp"
#include "simulate_time.hpp"

extern "C" {
#include "mm_led_control.h"
}

static TestOutput testOutput;

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
void mm_led_control_init(void)
{
    testOutput = TestOutput();
}

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

    /* Save event details to the output struct */
    testOutput.logLedUpdate(
        LedUpdate{
            get_simulated_time_elapsed(),
            target_node_id,
            led_function,
            led_colour
        }
    );
}

/**
 * Get the output log for the current test.
 */
TestOutput const & test_led_control_get_output(void)
{
    return testOutput;
}

/**
 * Writes LED output state information to the opened log file.
 * For example, the output would look like:
 *
 * <timestamp>,LED OUTPUT EVENT,node,1,func,0,colour,0
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