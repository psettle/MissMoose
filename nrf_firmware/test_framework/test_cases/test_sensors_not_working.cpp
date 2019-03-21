/**********************************************************
					   INCLUDES
**********************************************************/

#include "tests.hpp"

/**********************************************************
					   DECLARATIONS
**********************************************************/

static void test_case_bottom_left_lidar_not_working_one_animal(TestOutput& oracle);
static void test_case_bottom_right_lidar_not_working_one_animal(TestOutput& oracle);
static void test_case_top_middle_lidar_not_working_one_animal(TestOutput& oracle);
static void test_case_both_middle_node_sensors_not_working_one_animal(TestOutput& oracle);
static void test_case_both_top_right_node_sensors_not_working_one_animal(TestOutput& oracle);
static void test_case_both_middle_left_node_sensors_not_working_one_animal(TestOutput& oracle);
static void test_case_both_middle_node_sensors_not_working_two_animals(TestOutput& oracle);

/**********************************************************
					   DEFINITIONS
**********************************************************/

void test_sensors_not_working(std::vector<TestCase>& tests)
{
	ADD_TEST(test_case_bottom_left_lidar_not_working_one_animal);
	ADD_TEST(test_case_bottom_right_lidar_not_working_one_animal);
	ADD_TEST(test_case_top_middle_lidar_not_working_one_animal);
	ADD_TEST(test_case_both_middle_node_sensors_not_working_one_animal);
	ADD_TEST(test_case_both_top_right_node_sensors_not_working_one_animal);
	ADD_TEST(test_case_both_middle_left_node_sensors_not_working_one_animal);
	ADD_TEST(test_case_both_middle_node_sensors_not_working_two_animals);
}

static void test_case_bottom_left_lidar_not_working_one_animal(TestOutput& oracle)
{
	/*
	 * One deer enters the network from the top-right quadrant, brushes against the
	 * edges of the bottom-left quadrant, and then exits the network out of the bottom
	 * right quadrant. The bottom-left LIDAR is not working (not detecting).
	 */

	/* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from top-left PIR:
	 * the animal has entered the network from the top-left quadrant.
	 */
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from middle-left LIDAR and middle-middle PIR:
	 * the animal has moved into the bottom left quadrant.
	 */
	simulate_time(3);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 300);
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from middle-left PIR:
	 * the animal has brushed the bottom left edge of the network.
	 */
	simulate_time(2);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);

	/* After 2 seconds, middle-left LIDAR stops detecting. */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);

	/*
	 * Start detection from bottom-left PIR and bottom-right LIDAR:
	 * the animal has brushed the bottom left edge of the network.
	 */
	simulate_time(1);
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/*
	 * Start detection from top-middle LIDAR and bottom-middle PIR:
	 * the animal has moved into the bottom-right quadrant of the network.
	 */
	simulate_time(1);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1300);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/*
	 * Start detection from top-right LIDAR and bottom-right PIR:
	 * the animal has moved into the bottom-right quadrant of the network.
	 */
	simulate_time(2);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1300);
	test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/* After 4 seconds, bottom-right LIDAR has stopped detecting. */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);

	/* After 3 seconds, top-middle LIDAR has stopped detecting. */
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/* After 2 seconds, top-right LIDAR has stopped detecting. */
	simulate_time(2);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2100);

	/* After 10 seconds, top-left PIR has stopped detecting. */
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/* After 10 seconds, middle-middle PIR has stopped detecting.*/
	simulate_time(2);
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* After 10 seconds, middle-left PIR has stopped detecting.*/
	simulate_time(2);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/* After 10 seconds, bottom-left PIR has stopped detecting.*/
	simulate_time(1);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* After 10 seconds, bottom-middle PIR has stopped detecting.*/
	simulate_time(1);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 10 seconds, bottom-right PIR has stopped detecting.*/
	simulate_time(2);
	test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After about 30 seconds, the LEDs decay. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After about 60 seconds, the LEDs turn off. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}

static void test_case_bottom_right_lidar_not_working_one_animal(TestOutput& oracle)
{
	/*
	 * One deer enters the network from the top-right quadrant, brushes against the
	 * edges of the bottom-left quadrant, and then exits the network out of the bottom
	 * right quadrant. The bottom-left LIDAR is not working (not detecting).
	 */

	/* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from bottom-middle PIR:
	 * the animal has entered the network from the bottom-right quadrant.
	 */
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from (other!) bottom-middle PIR and top-middle LIDAR:
	 * the animal has moved into the bottom-left quadrant.
	 */
	simulate_time(2);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/*
	 * Start detection from bottom-left PIR:
	 * the animal has brushed the bottom edge of the bottom-left quadrant.
	 */
	simulate_time(1);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/*
	 * Start detection from bottom-left LIDAR and left-middle PIR:
	 * the animal has exited the network through the bottom-left quadrant.
	 */
	simulate_time(2);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 300);

	/* AFter 3 seconds, the top-middle LIDAR has stopped detecting.*/
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/* AFter 2 seconds, the bottom-left LIDAR has stopped detecting.*/
	simulate_time(2);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);

	/* After 10 seconds, bottom-middle PIR has stopped detecting. */
	simulate_time(3);
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* After 10 seconds, (other!) bottom-middle PIR has stopped detecting. */
	simulate_time(2);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 10 seconds, bottom-left PIR has stopped detecting. */
	simulate_time(1);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* After 10 seconds, left-middle PIR has stopped detecting. */
	simulate_time(2);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After about 30 seconds, the LEDs decay. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After about 60 seconds, the LEDs turn off. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}

static void test_case_top_middle_lidar_not_working_one_animal(TestOutput& oracle)
{
	/*
	 * One deer enters the network from the top-right quadrant, brushes against the
	 * edges of the bottom-left quadrant, and then exits the network out of the bottom
	 * right quadrant. The bottom-left LIDAR is not working (not detecting).
	 */

	 /* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from bottom-left PIR and bottom-right LIDAR:
	 * the animal has entered the network through the bottom-left quadrant.
	 */
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from bottom-middle PIR:
	 * the animal has moved into the bottom-right quadrant.
	 */
	simulate_time(2);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After 2 seconds, bottom-right LIDAR has stopped detecting. */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);

	/*
	 * Start detection from middle-left LIDAR and middle-right PIR:
	 * the animal has moved into the top-right quadrant.
	 */
	simulate_time(1);
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1300);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/*
	 * Start detection from top-left LIDAR and (other!) middle-right PIR:
	 * the animal has exited the network through the top-right quadrant.
	 */
	simulate_time(1);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 300);

	/* After 2 seconds, middle-left LIDAR has stopped detecting. */
	simulate_time(1);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);

	/* After 2 seconds, top-left LIDAR has stopped detecting. */
	simulate_time(1);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2100);
	
	/* After 10 seconds, bottom-left PIR has stopped detecting. */
	simulate_time(4);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* After 10 seconds, bottom-middle PIR has stopped detecting. */
	simulate_time(2);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 10 seconds, middle-right PIR has stopped detecting. */
	simulate_time(1);
	test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* After 10 seconds, (other!) middle-right PIR has stopped detecting. */
	simulate_time(1);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After about 30 seconds, the LEDs decay. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After about 60 seconds, the LEDs turn off. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}

static void test_case_both_middle_node_sensors_not_working_one_animal(TestOutput& oracle)
{
	/*
	 * One deer enters the network from the top-left quadrant and brushes the edge towards the
	 * bottom-left quadrant before proceeding right and exiting the network out of the top-right
	 * quadrant. Both PIR sensors of the middle node are not working (not detecting).
	 */

	 /* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from top-left PIR and bottom-left LIDAR:
	 * the animal has entered the network from the top-left quadrant.
	 */
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from middle-left LIDAR:
	 * the animal has brushed against the edge of towards the bottom-left 
	 * quadrant.
	 */
	simulate_time(1);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 300);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/*
	 * Start detection from top-middle LIDAR:
	 * the animal has moved into the top-right quadrant.
	 */
	simulate_time(3);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 300);

	/* After 4 seconds, bottom-left LIDAR stops detecting. */
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);

	/* After 3 seconds, middle-left LIDAR stops detecting. */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);

	/*
	 * Start detection from middle-right PIR and top-right LIDAR:
	 * the animal has exited the network from the top-right quadrant.
	 */
	simulate_time(2);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 300);

	/* After 2 seconds, top-middle LIDAR stops detecting. */
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/* After 2 seconds, top-right LIDAR stops detecting. */
	simulate_time(2);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2100);

	/* After 10 seconds, top-left PIR stops detecting. */
	simulate_time(2);
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/* After 10 seconds, middle-right PIR stops detecting. */
	simulate_time(6);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After about 30 seconds, the LEDs decay. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After about 60 seconds, the LEDs turn off. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}

static void test_case_both_top_right_node_sensors_not_working_one_animal(TestOutput& oracle)
{
	/*
	 * One deer enters the network from the top-right quadrant and proceeds 
	 * to move left diagonally until it exits the network from the bottom-left
	 * quadrant. Both sensors on the top-right node are not working (not detecting).
	 */

	/*
	 * Start detection from middle-right PIR: 
	 * the animal has entered the network from the top-right quadrant.
	 */
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from top-middle LIDAR and middle-middle PIR:
	 * the animal has moved into the top-left quadrant.
	 */
	simulate_time(2);
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 300);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/*
	 * Start detection from left-middle LIDAR and (other!) middle-middle PIR:
	 * the animal has moved into the bottom-left quadrant.
	 */
	simulate_time(3);
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 300);

	/* After 3 seconds, the top-middle LIDAR stops detecting. */
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/*
	 * Start detection from left-middle PIR and bottom-left LIDAR:
	 * the animal has exited the network from the bottom-left quadrant.
	 */
	simulate_time(1);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 300);

	/* After 2 seconds, the left-middle LIDAR stops detecting. */
	simulate_time(1);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);

	/* After 2 seconds, the bottom-left LIDAR stops detecting. */
	simulate_time(1);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);

	/* After 10 seconds, the middle-right PIR stops detecting. */
	simulate_time(2);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 10 seconds, the middle-middle PIR stops detecting. */
	simulate_time(2);
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 10 seconds, the (other!) middle-middle PIR stops detecting. */
	simulate_time(3);
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* After 10 seconds, the left-middle PIR stops detecting. */
	simulate_time(1);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/* After about 30 seconds, the LEDs decay. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After about 60 seconds, the LEDs turn off. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	 /* Idle */
	simulate_time(MINUTES(3));

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}

static void test_case_both_middle_left_node_sensors_not_working_one_animal(TestOutput& oracle)
{
	/*
	 * One deer enters the network from the top-right quadrant and proceeds left
	 * diagonally until exiting the network from the bottom-left quadrant. The middle-left
	 * node sensors are not working (not detecting).
	 */

	/* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from top-left LIDAR and top-right PIR:
	 * the animal has entered the network from the top-right quadrant.
	 */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1300);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from top-middle LIDAR and middle-middle PIR:
	 * the animal has moved into the top-left quadrant.
	 */
	simulate_time(2);
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 300);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/* After 2 seconds, top-left LIDAR stops detecting. */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);

	/*
	 * Start detection from (other!) middle-middle PIR:
	 * the animal has moved into the bottom-left quadrant.
	 */
	simulate_time(2);
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/* After 2 seconds, top-middle LIDAR stops detecting. */
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/*
	 * Start detection from bottom-left LIDAR:
	 * the animal has exited the network from the bottom-left quadrant.
	 */
	simulate_time(1);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 300);

	/* After 2 seconds, bottom-left LIDAR stops detecting. */
	simulate_time(2);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 300);

	/* After 10 seconds, top-right PIR stops detecting. */
	simulate_time(3);
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* After 10 seconds, middle-middle PIR stops detecting. */
	simulate_time(2);
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 10 seconds, (other!) middle-middle PIR stops detecting. */
	simulate_time(2);
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* After about 30 seconds, the LEDs decay. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After about 60 seconds, the LEDs turn off. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}

static void test_case_both_middle_node_sensors_not_working_two_animals(TestOutput& oracle)
{
	/*
	 * Two deer enter the network from the top-left quadrant of the network one-by-one.
	 * They proceed to move from left to right through the network. One exits the network
	 * from the top-right quadrant, the other exits from the bottom-right quadrant (at a 
	 * different time). Both PIR sensors of the middle node are not working (not detecting). 
	 */

	/* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from top-left PIR and bottom-left LIDAR:
	 * the first animal (A) has entered the network from the top-left quadrant.
	 */
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 1300);
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from top-left LIDAR and top-middle PIR:
	 * the second animal (B) has entered the network from the top-left quadrant.
	 */
	simulate_time(2);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 300);
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/* After 2 seconds, bottom-left LIDAR has stopped detecting. */
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);

	/*
	 * Start detection from middle-left LIDAR:
	 * animal A has moved into the bottom-left quadrant.
	 */
	simulate_time(1);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 300);

	/*
	 * Start detection from top-middle LIDAR:
	 * animal B has moved into the top-right quadrant.
	 */
	simulate_time(3);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 300);

	/* After 4 seconds, top-left LIDAR has stopped detecting. */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);

	/* After 3 seconds, middle-left LIDAR has stopped detecting. */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);

	/*
	 * Continue detection from top-middle LIDAR and start detection from bottom-middle PIR:
	 * animal A has moved into the bottom-right quadrant.
	 */
	simulate_time(2);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1300);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/*
	 * Start detection from bottom-right LIDAR and (other!) bottom-middle PIR:
	 * animal A has exited the network from the bottom-right quadrant.
	 */
	simulate_time(3);
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 300);
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);

	/* After 3 seconds, top-middle LIDAR has stopped detecting. */
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/* After 11 seconds, top-left PIR has stopped detecting */
	test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/*
	 * Start detection from top-right LIDAR and right-middle PIR:
	 * animal B has exited the network from the top-right quadrant.
	 */
	simulate_time(2);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 300);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/* After 2 seconds, bottom-right LIDAR has stopped detecting. */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);

	/* After 2 seconds, top-right LIDAR has stopped detecting. */
	simulate_time(2);
	test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2100);

	/* After 11 seconds, top-middle PIR has stopped detecting. */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* After 10 seconds, bottom-middle PIR has stopped detecting. */
	simulate_time(3);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 10 seconds, (other!) bottom-middle PIR has stopped detecting. */
	simulate_time(3);
	test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* After 10 seconds, right-middle PIR has stopped detecting. */
	simulate_time(2);
	test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After about 30 seconds, the LEDs decay. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After about 60 seconds, the LEDs turn off. */
	simulate_time(30);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}