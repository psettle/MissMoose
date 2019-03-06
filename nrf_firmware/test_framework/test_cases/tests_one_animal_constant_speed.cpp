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

/* Test one animal running parallel to the road quickly. */
static void test_case_roadside_parallel_fast(TestOutput& oracle);
static void test_case_nonroadside_parallel_fast(TestOutput& oracle);

/* Test one animal running towards the road quickly. */
static void test_case_left_towards_road_fast(TestOutput& oracle);
static void test_case_right_towards_road_fast(TestOutput& oracle);

/* Test one animal walking parallel to the road slowly. */
static void test_case_roadside_parallel_slow(TestOutput& oracle);
static void test_case_nonroadside_parallel_slow(TestOutput& oracle);

/* Test one animal walking away from the road slowly. */
static void test_case_left_away_road_slow(TestOutput& oracle);
static void test_case_right_away_road_slow(TestOutput& oracle);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_one_animal_constant_speed_add_tests(std::vector<TestCase>& tests)
{
    ADD_TEST(test_case_roadside_parallel_fast);
    ADD_TEST(test_case_nonroadside_parallel_fast);
    ADD_TEST(test_case_left_towards_road_fast);
    ADD_TEST(test_case_right_towards_road_fast);
    ADD_TEST(test_case_roadside_parallel_slow);
    ADD_TEST(test_case_nonroadside_parallel_slow);
    ADD_TEST(test_case_left_away_road_slow);
    ADD_TEST(test_case_right_away_road_slow);
}

static void test_case_roadside_parallel_fast(TestOutput& oracle)
{
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

    /* End pir detection from top-left of network */
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from middle of network */
    test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from right */
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);
    simulate_time(1);

    /* After about a minute leds should turn off */
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}

static void test_case_nonroadside_parallel_fast(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start lidar detection from left of network  */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 423);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start pir detection from left */
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start PIR detection from bottom */
    test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    simulate_time(1);

    /* Start lidar detection from top-middle */
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 913);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(1);

    /* Start PIR detection from right */
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    simulate_time(1);

    /* Start Lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1501);
    simulate_time(1);

    /* End lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);
    simulate_time(3);

    /* End lidar detection from top-middle */
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2000);
    simulate_time(2);

    /* PIR's take a little more time to cool off */
    simulate_time(9);

    /* End pir detection from top-left of network */
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);
    simulate_time(1);

    /* End lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1800);

    /* End pir detection from bottom of network */
    test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from right */
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);
    simulate_time(1);

    /* After about a minute leds should turn off */
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}

static void test_case_left_towards_road_fast(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start pir detection from bottom-left of network  */
    test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(7);

    /* Start lidar detection from bottom-right */
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1053);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(3);

    /* Start PIR detection from middle */
    test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(5);

    /* Start lidar detection from left */
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 113);
    simulate_time(1);

    /* Start PIR detection from top-middle */
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    simulate_time(1);

    /* Start Lidar detection from top-left */
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 721);
    simulate_time(4);

    /* End lidar detection from bottom-right */
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);
    simulate_time(1);

    /* End lidar detection from left */
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1700);
    simulate_time(1);

    /* End lidar detection from top-left */
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1800);

    /* PIR's take a little more time to cool off */
    simulate_time(13);

    /* End pir detection from bottom-left of network */
    test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from middle of network */
    test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from top-middle */
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);
    simulate_time(1);

    /* After about a minute leds should turn off */
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}

static void test_case_right_towards_road_fast(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start pir detection from bottom of network  */
    test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(5);

    /* Start lidar detection from bottom-right */
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 642);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(3);

    /* Start PIR detection from right */
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(5);

    /* Start lidar detection from left */
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1550);
    simulate_time(1);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    /* Start PIR detection from top-right */
    test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    simulate_time(1);

    /* Start Lidar detection from top-left */
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1501);
    simulate_time(4);

    /* End lidar detection from bottom-right */
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);
    simulate_time(1);

    /* End lidar detection from left */
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1700);
    simulate_time(1);

    /* End lidar detection from top-left */
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1800);

    /* PIR's take a little more time to cool off */
    simulate_time(13);

    /* End pir detection from bottom of network */
    test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from right of network */
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
    simulate_time(1);

    /* End pir detection from top-right */
    test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);
    simulate_time(1);

    /* After about a minute leds should turn off */
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}

static void test_case_roadside_parallel_slow(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));


    /* Start PIR detection from right */
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(7);

    /* Start Lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 243);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    simulate_time(6);

    /* End lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1800);
    simulate_time(17);

    /* End pir detection from right */
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(12);

    /* Start PIR detection from middle */
    test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start lidar detection from top-middle */
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 413);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    simulate_time(10);

    /* End lidar detection from top-middle */
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2000);
    simulate_time(22);

    /* End pir detection from middle of network */
    test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(22);

    /* Start pir detection from top-left of network  */
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(4);

    /* Start lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 1100);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(12);

    /* End lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);
    simulate_time(20);

    /* End pir detection from top-left of network */
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}

static void test_case_nonroadside_parallel_slow(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start PIR detection from right */
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(3);

    /* Start Lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 899);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(22);

    /* End lidar detection from top-right */
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1800);
    simulate_time(5);

    /* End pir detection from right */
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(14);

    /* Start PIR detection from middle */
    test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    simulate_time(8);

    /* Start lidar detection from top-middle */
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1009);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(4);

    /* End lidar detection from top-middle */
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2000);
    simulate_time(17);

    /* End pir detection from middle of network */
    test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(25);

    /* Start pir detection from left of network  */
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(3);

    /* Start lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 500);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(7);

    /* End lidar detection from bottom-left */
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);
    simulate_time(23);

    /* End pir detection from left of network */
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}

static void test_case_left_away_road_slow(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start PIR detection from top-middle */
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start Lidar detection from top-left */
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 432);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(13);

    /* End lidar detection from top-left */
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1800);
    simulate_time(17);

    /* End pir detection from top-middle */
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(12);

    /* Start PIR detection from middle */
    test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(3);

    /* Start lidar detection from left */
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 243);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(4);

    /* End lidar detection from left */
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 3500);
    simulate_time(14);

    /* End pir detection from middle of network */
    test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(22);

    /* Start pir detection from bottom-left of network  */
    test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(5);

    /* Start lidar detection from bottom-right */
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1263);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(7);

    /* End lidar detection from bottom-right */
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);
    simulate_time(21);

    /* End pir detection from bottom-left of network */
    test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}

static void test_case_right_away_road_slow(TestOutput& oracle)
{
    /* Start idle for a bit. */
    simulate_time(MINUTES(2));

    /* Start PIR detection from top-right */
    test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(4);

    /* Start Lidar detection from top-left */
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1387);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    simulate_time(11);

    /* End lidar detection from top-left */
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 3000);
    simulate_time(19);

    /* End pir detection from top-right */
    test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(8);

    /* Start PIR detection from right */
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(5);

    /* Start lidar detection from left */
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 897);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    simulate_time(3);

    /* End lidar detection from left */
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2700);
    simulate_time(18);

    /* End pir detection from right of network */
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(14);

    /* Start pir detection from bottom of network  */
    test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(2);

    /* Start lidar detection from bottom-right */
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 394);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(11);

    /* End lidar detection from bottom-right */
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1800);
    simulate_time(16);

    /* End pir detection from bottom of network */
    test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
    simulate_time(MINUTES(10));
}