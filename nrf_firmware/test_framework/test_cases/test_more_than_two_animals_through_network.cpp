/**********************************************************
					   INCLUDES
**********************************************************/

#include "tests.hpp"

/**********************************************************
					   DECLARATIONS
**********************************************************/

static void test_case_three_animals_left_to_right(TestOutput& oracle);

/**********************************************************
					   DEFINITIONS
**********************************************************/

void test_more_than_two_animals_through_network(std::vector<TestCase>& tests)
{
	ADD_TEST(test_case_three_animals_left_to_right);
}

static void test_case_three_animals_left_to_right(TestOutput& oracle)
{
	/*
	 * Three deer enter the network from the left side (2 simultaneously,
	 * one after a delay), and proceed to exit out of the right side of the network
	 * at different times. They move slowly.
	 */
	
	/* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from top-left PIR and bottom-left LIDAR:
	 * two animals (A and B) have entered the network into the top-left quadrant.
	 */
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After 8 seconds, bottom-left LIDAR stops detecting. */
	simulate_time(8);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);

	/* After 10 (8 + 2) seconds, top-left PIR stops detecting. */
	simulate_time(2);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/*
	 * Start detection from middle-left PIR and bottom left LIDAR:
	 * the third animal (C) has entered the network from the bottom-left
	 * quadrant.
	 */
	simulate_time(10);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 300);

	/*
	 * Start detection from top-middle LIDAR and middle-middle PIR:
	 * animal B has entered the top-right quadrant of the network.
	 */
	simulate_time(5);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 300);
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/* By now, the bottom-left LIDAR has stopped detecting. */
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);

	/* Middle-left PIR has stopped detecting. */
	simulate_time(2);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/*
	 * Continue detection from top-middle LIDAR and middle-middle PIR:
	 * animal A has entered the top-right quadrant of the network.
	 */
	simulate_time(3);

	/* After 4 seconds, top-middle LIDAR has stopped detecting. */
	simulate_time(4);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/* After 9 seconds (4 + 5), middle-middle PIR has stopped detecting. */
	simulate_time(5);
	test_send_lidar_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/*
	 * Start detection from top-right LIDAR and middle-right PIR:
	 * animal B has exited the network from the top-right quadrant.
	 */
	simulate_time(1);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 300);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/*
	 * Continue detection from top-right LIDAR and middle-right PIR:
	 * animal A has exited the network from the top-right quadrant.
	 */
	simulate_time(3);

	/* After 3 seconds, top-right LIDAR has stopped detecting. */
	simulate_time(3);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2100);

	/* After 9 (3 + 6) seconds, middle-right PIR has stopped detecting. */
	simulate_time(6);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/*
	 * Start detection from bottom-middle PIR and top-middle LIDAR:
	 * animal C has moved into the bottom-right quadrant of the network.
	 */
	simulate_time(2);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1300);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/*
	 * Start detection from left-middle LIDAR and right-middle PIR:
	 * animal C has moved into the top-right quadrant of the network.
	 */
	simulate_time(3);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_180, 1300);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/* By now, (3 seconds) top-middle LIDAR has stopped detecting. */
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/*
	 * Start detection from top-right LIDAR and right-middle PIR:
	 * animal C has exited the network from the top-right quadrant.
	 */
	simulate_time(4);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 300);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/* After 8 seconds (4 + 4), LIDAR has stopped detecting. */
	simulate_time(4);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_180, 2100);

	/* After 10 seconds (4 + 4 + 2), bottom-middle PIR has stopped detecting. */
	simulate_time(2);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 6 seconds (4 + 2), top-right LIDAR has stopped detecting. */
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2100);

	/* After 11 seconds (4 + 2 + 5), right-middle PIR has stopped detecting. */
	simulate_time(5);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* 30 seconds after left-side detections, the left LED decays. */
	simulate_time(12);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* 30 seconds after right-side detections, the middle and right LEDs decay. */
	simulate_time(7);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* 60 seconds after left-side detections, the left LED is off. */
	simulate_time(23);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

	/* 60 seconds after right-side detections, the middle and right LEDs are off. */
	simulate_time(7);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}