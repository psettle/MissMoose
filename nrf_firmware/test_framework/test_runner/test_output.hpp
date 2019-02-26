/**
file: test_output.hpp
brief:
notes:
*/
#ifndef TEST_OUTPUT_HPP
#define TEST_OUTPUT_HPP

/**********************************************************
                        INCLUDES
**********************************************************/

#include <vector>

extern "C" {
#include "mm_led_control.h"
#include "mm_position_config.h"
}


/**********************************************************
                          TYPES
**********************************************************/

struct LedUpdate {
    uint32_t        time_s;
    uint16_t        targetNodeIdM;
    led_function_t  ledFunctionM;
    led_colours_t   ledColourM;
};

class TestOutput {
public:

    /**
     * Setup a test output as a blank oracle, and return the set of output leds.
     */
    void initOracle(std::vector<mm_node_position_t const *>& outputNodes);

    /**
     * Add an led update to the output. Must be logged in chronological order.
     */
    void logLedUpdate(LedUpdate const & update);

    /**
     * Log an led update at the current simulated time for the node in position (x, y)
     */
    void logLedUpdate
        (
            int8_t x,
            int8_t y,
            led_function_t  ledFunctionM,
            led_colours_t   ledColourM = LED_COLOURS_RED
        );

    /**
     * Calculate to what degree result matches oracle (0 to 1 score)
     */
    static float getMatchScore(TestOutput const & result, TestOutput const & oracle);
private:

    /**
     * The set of updates that define this output.
     */
    std::vector<LedUpdate> ledUpdatesM;
};

#endif /* TEST_OUTPUT_HPP */