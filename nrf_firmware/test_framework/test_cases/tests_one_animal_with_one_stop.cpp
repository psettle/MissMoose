/**
file: tests_one_animal_with_one_stop.cpp
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

/* 
 * Test one animal stopping once outside of sensor sight
 * while walking straight through the network
 */
static void test_case_stop_outside_sight_roadside_left(TestOutput& oracle);
static void test_case_stop_outside_sight_roadside_right(TestOutput& oracle);
static void test_case_stop_outside_sight_nonroadside_left(TestOutput& oracle);
static void test_case_stop_outside_sight_nonroadside_right(TestOutput& oracle);

/*
 * Test one animal stopping once within sensor sight
 * while walking straight through the network
 */
static void test_case_stop_within_sight_roadside_left(TestOutput& oracle);
static void test_case_stop_within_sight_roadside_right(TestOutput& oracle);
static void test_case_stop_within_sight_top_left(TestOutput& oracle);
static void test_case_stop_within_sight_top_middle(TestOutput& oracle);
static void test_case_stop_within_sight_top_right(TestOutput& oracle);
static void test_case_stop_within_sight_middle_left(TestOutput& oracle);
static void test_case_stop_within_sight_middle_right(TestOutput& oracle);
static void test_case_stop_within_sight_bottom_left(TestOutput& oracle);
static void test_case_stop_within_sight_bottom_middle(TestOutput& oracle);
static void test_case_stop_within_sight_bottom_right(TestOutput& oracle);
static void test_case_stop_within_sight_nonroadside_left(TestOutput& oracle);
static void test_case_stop_within_sight_nonroadside_right(TestOutput& oracle);

/**********************************************************
					   DEFINITIONS
**********************************************************/

void test_one_animal_with_one_stop_add_tests(std::vector<TestCase>& tests)
{
	ADD_TEST(test_case_stop_outside_sight_roadside_left);
	ADD_TEST(test_case_stop_outside_sight_roadside_right);
	ADD_TEST(test_case_stop_outside_sight_nonroadside_left);
	ADD_TEST(test_case_stop_outside_sight_nonroadside_right);
	ADD_TEST(test_case_stop_within_sight_roadside_left);
	ADD_TEST(test_case_stop_within_sight_roadside_right);
	//ADD_TEST(test_case_stop_within_sight_top_left);
	//ADD_TEST(test_case_stop_within_sight_top_middle);
	//ADD_TEST(test_case_stop_within_sight_top_right);
	ADD_TEST(test_case_stop_within_sight_middle_left);
	ADD_TEST(test_case_stop_within_sight_middle_right);
	//ADD_TEST(test_case_stop_within_sight_bottom_left);
	//ADD_TEST(test_case_stop_within_sight_bottom_middle);
	//ADD_TEST(test_case_stop_within_sight_bottom_right);
	ADD_TEST(test_case_stop_within_sight_nonroadside_left);
	ADD_TEST(test_case_stop_within_sight_nonroadside_right);
}

static void test_case_stop_outside_sight_roadside_left(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom left to top left quadrants,
	 * stopping for at least 2 minutes in the top left before exiting
	 * through the top left towards the road
	 */

	/* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-left of network  */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(7);	

	/* End lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);
	simulate_time(1);

	/* Start PIR detection from middle */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 405);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(4);

	/* End pir detection from bottom-left of network */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(1);

	/* End lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(8);

	/* End pir detection from middle of network */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* Deer stops in top-left quadrant for at least 2 minutes */

	/* After about a minute from the most recent detection leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

	/* Wait for the rest of the deer's 2 minutes */
	simulate_time(MINUTES(1));

	/* Start pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 413);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(4);

	/* End lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(12);

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

static void test_case_stop_outside_sight_roadside_right(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom right to top right quadrants,
	 * stopping for at least 2 minutes in the top right before exiting
	 * through the top right towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-middle of network  */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 425);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(7);

	/* End lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2200);
	simulate_time(1);

	/* Start PIR detection from middle-right */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1300);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	simulate_time(4);

	/* End pir detection from bottom-middle of network */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(1);

	/* End lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(8);

	/* End pir detection from middle-right of network */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* Deer stops in top-right quadrant for at least 2 minutes */

	/* After about a minute from the most recent detection leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Wait for the rest of the deer's 2 minutes */
	simulate_time(MINUTES(1));

	/* Start pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1200);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	simulate_time(4);

	/* End lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(12);

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

static void test_case_stop_outside_sight_nonroadside_left(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom left to top left quadrants,
	 * stopping for at least 2 minutes in the bottom left before exiting
	 * through the top left towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-left of network  */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(7);

	/* End lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);
	simulate_time(4);

	/* End pir detection from bottom-left of network */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* Deer stops in bottom-left quadrant for at least 2 minutes */

	/* After about a minute from the most recent detection leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

	/* Wait for the rest of the deer's 2 minutes */
	simulate_time(MINUTES(1));

	/* Start pir detection from middle */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 405);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(5);

	/* End lidar detection from middle-left */
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_90, 2100);
	simulate_time(2);

	/* Start pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(2);	

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 413);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(4);

	/* End pir detection from middle of network */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);

	/* End lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(10);

	/* End pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

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

static void test_case_stop_outside_sight_nonroadside_right(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom right to top right quadrants,
	 * stopping for at least 2 minutes in the bottom right before exiting
	 * through the top right towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-middle of network  */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 425);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(7);

	/* End lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2200);
	simulate_time(4);

	/* End pir detection from bottom-middle of network */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* Deer stops in bottom-right quadrant for at least 2 minutes */

	/* After about a minute from the most recent detection leds should turn off */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

	/* Wait for the rest of the deer's 2 minutes */
	simulate_time(MINUTES(1));

	/* Start pir detection from middle-right */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1300);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	simulate_time(4);	

	/* End lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(3);

	/* Start pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1200);
	simulate_time(4);

	/* End pir detection from middle-right of network */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);	
	simulate_time(1);

	/* End lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(9);

	/* End pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

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

static void test_case_stop_within_sight_roadside_left(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom left to top left quadrants,
	 * stopping for at least 2 minutes between the roadside left and middle
	 * node before exiting through the top left towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-left of network  */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(7);

	/* End lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);
	simulate_time(1);

	/* Start PIR detection from middle */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 405);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(4);

	/* End pir detection from bottom-left of network */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(2);

	/* End lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(1);

	/* Start pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 413);
	simulate_time(4);	

	/* End pir detection from middle of network */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* Deer waits between roadside left and middle nodes for at least 2 minutes */
	simulate_time(MINUTES(2));

	/* End lidar detection from top-left */
	simulate_time(5);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(9);

	/* End pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

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

static void test_case_stop_within_sight_roadside_right(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom right to top right quadrants,
	 * stopping for at least 2 minutes between the roadside right and middle
	 * nodes before exiting through the top right towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-middle of network  */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(1);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 425);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(6);

	/* End lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2200);
	simulate_time(1);

	/* Start pir detection from middle-right */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1300);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	simulate_time(5);		

	/* End pir detection from bottom-middle of network */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(1);

	/* End lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(1);

	/* Start pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1200);
	simulate_time(4);	

	/* End pir detection from middle-right of network */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);	

	/* Deer stops between roadside right and middle nodes for at least 2 minutes */
	simulate_time(MINUTES(2));

	/* End lidar detection from top-left */
	simulate_time(6);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(12);

	/* End pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

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

static void test_case_stop_within_sight_top_left(TestOutput& oracle)
{
	
}

static void test_case_stop_within_sight_top_middle(TestOutput& oracle)
{
	
}

static void test_case_stop_within_sight_top_right(TestOutput& oracle)
{
	
}

static void test_case_stop_within_sight_middle_left(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom left to top left quadrants,
	 * stopping for at least 2 minutes between the middle-left and middle
	 * nodes before exiting through the top left towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-left of network  */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(5);

	/* End lidar detection from bottom-right */	
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);
	simulate_time(3);

	/* Start pir detection from middle */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 405);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(4);

	/* End pir detection from bottom-left of network */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* Deer waits between middle-left and middle nodes for at least 2 minutes */
	simulate_time(MINUTES(2));	

	/* End lidar detection from middle-left */
	simulate_time(6);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(2);

	/* Start pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 413);
	simulate_time(4);

	/* End pir detection from middle of network */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);

	/* End lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(9);

	/* End pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

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

static void test_case_stop_within_sight_middle_right(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom right to top right quadrants,
	 * stopping for at least 2 minutes between the middle-right and middle
	 * nodes before exiting through the top right towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-middle of network  */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(1);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 425);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(6);

	/* End lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2200);
	simulate_time(1);

	/* Start pir detection from middle-right */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1300);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	simulate_time(5);

	/* End pir detection from bottom-middle of network */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* Deer stops between middle-right and middle nodes for at least 2 minutes */
	simulate_time(MINUTES(2));

	/* End lidar detection from middle-left */
	simulate_time(5);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(3);

	/* Start pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1200);
	simulate_time(5);

	/* End pir detection from middle-right of network */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);	

	/* End lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(10);

	/* End pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

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

static void test_case_stop_within_sight_bottom_left(TestOutput& oracle)
{
	
}

static void test_case_stop_within_sight_bottom_middle(TestOutput& oracle)
{
	
}

static void test_case_stop_within_sight_bottom_right(TestOutput& oracle)
{
	
}

static void test_case_stop_within_sight_nonroadside_left(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom left to top left quadrants,
	 * stopping for at least 2 minutes between the nonroadside left and middle
	 * nodes before exiting through the top left towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-left of network  */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* Deer waits between nonroadside left and middle nodes for at least 2 minutes */
	simulate_time(MINUTES(2));

	/* End lidar detection from bottom-right */
	simulate_time(5);
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);
	simulate_time(3);

	/* Start pir detection from middle */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 405);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(4);

	/* End pir detection from bottom-left of network */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(2);

	/* End lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(1);

	/* Start pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 413);
	simulate_time(4);

	/* End pir detection from middle of network */
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);

	/* End lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(9);

	/* End pir detection from top-middle */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

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

static void test_case_stop_within_sight_nonroadside_right(TestOutput& oracle)
{
	/*
	 * Deer walks through network from bottom right to top right quadrants,
	 * stopping for at least 2 minutes between the nonroadside right and
	 * middle nodes before exiting through the top right towards the road
	 */

	 /* Start idle for a bit. */
	simulate_time(MINUTES(2));

	/* Start pir detection from bottom-middle of network  */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	simulate_time(2);

	/* Start lidar detection from bottom-right */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 425);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* Deer stops between nonroadside right and middle nodes for at least 2 minutes */
	simulate_time(MINUTES(2));

	/* End lidar detection from bottom-right */
	simulate_time(6);
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2200);
	simulate_time(2);

	/* Start pir detection from middle-right */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1300);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	simulate_time(4);

	/* End pir detection from bottom-middle of network */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);
	simulate_time(2);

	/* End lidar detection from middle-left */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);
	simulate_time(1);

	/* Start pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(1);

	/* Start lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1200);
	simulate_time(4);

	/* End pir detection from middle-right of network */
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
	simulate_time(1);

	/* End lidar detection from top-left */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);
	simulate_time(9);

	/* End pir detection from top-right */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

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
