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

#define WHITE_TAIL_DEER_KMH     ( 48 )
#define WHITE_TAIL_DEER_M_PER_S ( WHITE_TAIL_DEER_KMH * 1000 / 3600)

/**********************************************************
                       DECLARATIONS
**********************************************************/

/* Test one animal running parellel to the road quickly. */
static void test_case_roadside_parallel_fast(TestOutput& oracle);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_one_animal_constant_speed_add_tests(std::vector<TestCase>& tests)
{
    ADD_TEST(test_case_roadside_parallel_fast);
}

static void test_case_roadside_parallel_fast(TestOutput& oracle)
{
    //this is ~0.6s
    auto cell_travel_time = 8.0f / WHITE_TAIL_DEER_M_PER_S;
    
    std::vector<mm_node_position_t const *> outputNodes;
    oracle.initOracle(outputNodes);

    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start pir detection from top-left of network  */
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 1100);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start PIR detection from middle */
    test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    simulate_time(1);

    /* Start PIR detection from right */
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
    simulate_time(1);

    /* Start lidar detection from top-middle */
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 413);
    simulate_time(1);

    /* Start Lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 243);
    simulate_time(1);

    /* End lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);
    simulate_time(3);

    /* End lidar detection from top-middle */
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2000);
    simulate_time(2);

    /* End lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1800);

    /* PIR's take a little more time to cool off */
    simulate_time(12);

    /* End pir detection from top-left of network (No expected output) */
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from middle of network (No expected output) */
    test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from right (No expected output) */
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);
    simulate_time(1);

    /* After about a minute leds should turn off, slightly staggered towards the right */
    simulate_time(MINUTES(1));
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(5);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(5);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}