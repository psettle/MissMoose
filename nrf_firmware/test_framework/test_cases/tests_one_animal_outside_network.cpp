/**
file: tests_one_animal_outside_network.cpp
brief: Tests where an animal is close enough to the network to trigger PIR 
    sensors but never enters the network, meaning there are no LIADR detections
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

/* 
 * Test one animal passing just outside the network being 
 * detected only by the PIR sensors
 */
static void test_case_pass_outside_network_roadside_left(TestOutput& oracle);
static void test_case_pass_outside_network_roadside_right(TestOutput& oracle);
static void test_case_pass_outside_network_nonroadside_left(TestOutput& oracle);
static void test_case_pass_outside_network_nonroadside_right(TestOutput& oracle);
static void test_case_pass_outside_network_left_roadside(TestOutput& oracle);
static void test_case_pass_outside_network_left_nonroadside(TestOutput& oracle);
static void test_case_pass_outside_network_right_roadside(TestOutput& oracle);
static void test_case_pass_outside_network_right_nonroadside(TestOutput& oracle);

/*
 * Test one animal stopping just outside the network being 
 * detected only by the PIR sensors
 */
static void test_case_stop_outside_network_roadside(TestOutput& oracle);
static void test_case_stop_outside_network_nonroadside(TestOutput& oracle);

/**********************************************************
					   DEFINITIONS
**********************************************************/

void test_one_animal_outside_network_add_tests(std::vector<TestCase>& tests)
{
	ADD_TEST(test_case_pass_outside_network_roadside_left);
	ADD_TEST(test_case_pass_outside_network_roadside_right);
	ADD_TEST(test_case_pass_outside_network_nonroadside_left);
	ADD_TEST(test_case_pass_outside_network_nonroadside_right);
	ADD_TEST(test_case_pass_outside_network_left_roadside);
	ADD_TEST(test_case_pass_outside_network_left_nonroadside);
	ADD_TEST(test_case_pass_outside_network_right_roadside);
	ADD_TEST(test_case_pass_outside_network_right_nonroadside);
	ADD_TEST(test_case_stop_outside_network_roadside);
	ADD_TEST(test_case_stop_outside_network_nonroadside);
}

static void test_case_pass_outside_network_roadside_left(TestOutput& oracle)
{
	/*
	 * Deer walks past the outside of the network and is detected by only by top-centre PIR sensor
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

    /* Start pir detection from top-centre of network  */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(2);

    /* End pir detection from top-centre of network */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);

    /* After about 30 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_pass_outside_network_roadside_right(TestOutput& oracle)
{
	/*
	 * Deer walks past the outside of the network and is detected by only by top-right PIR sensor
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from top-right of network  */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* End pir detection from top-right of network */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);

	/* After about 30 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_pass_outside_network_nonroadside_left(TestOutput& oracle)
{
	/*
	 * Deer walks past the outside of the network and is detected by only by bottom-left PIR sensor
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-left of network  */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(2);

	/* End pir detection from bottom-left of network */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(1);

	/* After about 30 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_pass_outside_network_nonroadside_right(TestOutput& oracle)
{
	/*
	 * Deer walks past the outside of the network and is detected by only by bottom-centre PIR sensor
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-centre of network  */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(2);

	/* End pir detection from bottom-centre of network */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(1);

	/* After about 30 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_pass_outside_network_left_roadside(TestOutput& oracle)
{
	/*
	 * Deer walks past the outside of the network and is detected by only by top-left PIR sensor
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from top-left of network  */
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(2);

	/* End pir detection from top-left of network */
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);
	simulate_time(1);

	/* After about 30 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_pass_outside_network_left_nonroadside(TestOutput& oracle)
{
	/*
	 * Deer walks past the outside of the network and is detected by only by centre-left PIR sensor
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from centre-left of network  */
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(2);

	/* End pir detection from centre-left of network */
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);
	simulate_time(1);

	/* After about 30 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_pass_outside_network_right_roadside(TestOutput& oracle)
{
	/*
	 * Deer walks past the outside of the network and is detected by only by centre-right PIR sensor
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from centre-right of network  */
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* End pir detection from centre-right of network */
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);
	simulate_time(1);

	/* After about 30 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_pass_outside_network_right_nonroadside(TestOutput& oracle)
{
	/*
	 * Deer walks past the outside of the network and is detected by only by bottom-right PIR sensor
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-right of network  */
	test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(2);

    /* End pir detection from bottom-right of network */
	test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);
	simulate_time(1);

	/* After about 30 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_stop_outside_network_roadside(TestOutput& oracle)
{
    /*
	 * Deer walks stops for at least 5 minutes outside of network on nonroadside side
	 */

    /* Start idle for a bit. */
	simulate_time(MINUTES(2));

    /* Start pir detection from top-centre of network  */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

    /* Deer stops for 5 minutes still being detected by the PIR sensor */
	simulate_time(MINUTES(5));
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    /* End pir detection from top-centre of network */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);

    /* After about 60 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
	simulate_time(MINUTES(10));

    /* Start pir detection from top-right of network  */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

    /* Deer stops for 5 minutes still being detected by the PIR sensor */
	simulate_time(MINUTES(5));
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    /* End pir detection from top-right of network */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);

    /* After about 60 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
	simulate_time(MINUTES(10));
}

static void test_case_stop_outside_network_nonroadside(TestOutput& oracle)
{
    /*
	 * Deer walks stops for at least 5 minutes outside of network on nonroadside side
	 */

    /* Start idle for a bit. */
	simulate_time(MINUTES(2));

    /* Start pir detection from bottom-left of network  */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

    /* Deer stops for 5 minutes still being detected by the PIR sensor */
	simulate_time(MINUTES(5));
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    /* Deer moves away from the network */

    /* End pir detection from bottom-left of network */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(1);

    /* After about 60 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
	simulate_time(MINUTES(10));

    /* Start pir detection from bottom-centre of network  */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

    /* Deer stops for 5 minutes still being detected by the PIR sensor */
	simulate_time(MINUTES(5));
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    /* Deer moves away from the network */

    /* End pir detection from bottom-centre of network */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(1);

    /* After about 60 seconds leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    /* Let everything cool off */
	simulate_time(MINUTES(10));
}
