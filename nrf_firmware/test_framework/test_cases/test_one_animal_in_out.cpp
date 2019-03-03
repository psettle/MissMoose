/**
file: tests_one_animal_constant_speed.cpp
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

/* Test an animal barely entering the network from each side, then exiting right away. */
static void test_case_in_out_left_side(TestOutput& oracle);
static void test_case_in_out_right_side(TestOutput& oracle);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_one_animal_in_out_add_tests(std::vector<TestCase>& tests)
{
    ADD_TEST(test_case_in_out_left_side);
    ADD_TEST(test_case_in_out_right_side);
}

static void test_case_in_out_left_side(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start pir detection from left of network  */
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(5);

    /* Start lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 500);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(9);

    /* End lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);
    simulate_time(13);

    /* End pir detection from left of network */
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(17);

    /* Start pir detection from top-left of network  */
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 1100);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(15);

    /* End lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);
    simulate_time(12);

    /* End pir detection from top-left of network (No expected output) */
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything settle out. */
    simulate_time(MINUTES(10));
}

static void test_case_in_out_right_side(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start PIR detection from right */
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start Lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 243);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    simulate_time(4);

    /* End lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1800);
    simulate_time(15);

    /* End pir detection from right */
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(14);

    /* Start PIR detection from bottom right */
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(1);

    /* Start Lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 899);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(3);

    /* End lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1800);
    simulate_time(22);

    /* End pir detection from bottom-right */
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything settle out. */
    simulate_time(MINUTES(10));
}