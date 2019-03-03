/**
file: test_demo.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "tests.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
extern "C" {
#include "mm_sensor_error_check.h"
#include "mm_sensor_algorithm_config.h"
}

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/* Tests the algorithm running idle for 3 days. */
static void test_case_3_days_idle(TestOutput& oracle);
/* Test sending PIR and Lidar data */
static void test_case_test_sending_sensor_data(TestOutput& oracle);
/* Test providing a desired output. */
static void test_case_with_oracle(TestOutput& oracle);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_demo_add_tests(std::vector<TestCase>& tests)
{
    ADD_TEST(test_case_3_days_idle);
    ADD_TEST(test_case_test_sending_sensor_data);
    ADD_TEST(test_case_with_oracle);
}

static void test_case_3_days_idle(TestOutput& oracle)
{
    simulate_time(DAYS(3));
}

static void test_case_test_sending_sensor_data(TestOutput& oracle)
{
    // Test sending PIR and lidar data
    test_send_pir_data(2, SENSOR_ROTATION_90, PIR_DETECTION_START);
    test_send_lidar_data(3, SENSOR_ROTATION_0, 100);
    // Simulate time passing
    simulate_time(MINUTES(5));
}

static void test_case_with_oracle(TestOutput& oracle)
{
    test_send_pir_data(2, SENSOR_ROTATION_90, PIR_DETECTION_START);
    test_send_lidar_data(3, SENSOR_ROTATION_0, 100);

    /* Those two detections could put all the leds to on forever
       (This isn't a very serious test) */
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    // Simulate time passing
    simulate_time(MINUTES(5));
}
