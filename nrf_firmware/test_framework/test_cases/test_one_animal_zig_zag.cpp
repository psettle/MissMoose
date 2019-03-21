/**********************************************************
					   INCLUDES
**********************************************************/

#include "tests.hpp"

/**********************************************************
					   DECLARATIONS
**********************************************************/

static void one_animal_zig_zag_left_away_slow(TestOutput& oracle);
static void one_animal_zig_zag_right_away_fast(TestOutput& oracle);

/**********************************************************
					   DEFINITIONS
**********************************************************/

void test_one_animal_zig_zag(std::vector<TestCase>& tests)
{
	ADD_TEST(one_animal_zig_zag_left_away_slow);
	ADD_TEST(one_animal_zig_zag_right_away_fast);
}

static void one_animal_zig_zag_left_away_slow(TestOutput& oracle)
{
	/*
	 * One deer brushes the network from the top left (after crossing the road),
	 * then enters the network from the top right, crosses to the top left AV, and
	 * then down to the bottom left AV before leaving the network.
	 */

	 /* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from top-left LIDAR and top-middle PIR:
	 * the animal has brushed the top of the network.
	 */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 300);
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from top-left LIDAR and top-right PIR:
	 * the animal has entered the network from the top right quadrant.
	 */
	simulate_time(3);
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1300);
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/*
	 * Start detection from top-middle LIDAR and middle-middle PIR:
	 * the animal has moved into the top-left quadrant.
	 */
	simulate_time(8);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 300);
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/* After 8 seconds, the top-left LIDAR has stopped detecting. */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);

	/* After 11 seconds (3 + 8), the top-middle PIR has stopped detecting. */
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/*
	 * Start detection from left-middle LIDAR and continue detection for middle-middle PIR:
	 * the animal has moved into the bottom-left quadrant.
	 */
	simulate_time(4);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_180, 300);

	/* After 12 seconds, the top-right PIR has stoppped detecting. */
	test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/*
	 * Start detection from left-middle PIR and bottom-left LIDAR:
	 * the animal has brushed the bottom-left edge of the network.
	 */
	simulate_time(4);
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 300);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);

	/* After 8 seconds (4 + 4), the top-middle LIDAR has stopped detecting. */
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2100);

	/* After 4 seconds, the left-middle LIDAR has stopped detecting. */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_180, 2100);

	/*
	 * Start detection from bottom-left PIR and bottom-right LIDAR:
	 * the animal has exited the network from the bottom-left quadrant.
	 */
	simulate_time(8);
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_180, PIR_DETECTION_START);

	/* After 12 seconds (4 + 8), the middle-middle PIR has stopped detecting. */
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 8 seconds, the bottom-left LIDAR has stopped detecting. */
	test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2100);

	/* After 12 seconds (8 + 4), the left-middle PIR has stopped detecting. */
	simulate_time(4);
	test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/* After 4 seconds, the bottom-right LIDAR has stopped detecting. */
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);

	/* 30 seconds after right-side detections, the right LED decays. */
	simulate_time(10);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* After 20 seconds (4 + 10 + 6), the bottom-left PIR has stopped detecting. */
	simulate_time(6);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_180, PIR_DETECTION_END);

	/* 30 seconds after left-side detections, the middle and left LEDs decay. */
	simulate_time(10);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* 60 seconds after right-side detections, the right LED turns off. */
	simulate_time(14);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* 60 seconds after left-side detections, the middle and left LEDs decay. */
	simulate_time(16);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}

static void one_animal_zig_zag_right_away_fast(TestOutput& oracle)
{
	/*
	 * One deer enters the network from the top-left quadrant and weaves 
	 * between the middle column of nodes before exiting the network from
	 * the bottom left quadrant.
	 */

	 /* Idle */
	simulate_time(MINUTES(3));

	/*
	 * Start detection from top-left LIDAR and top-middle PIR:
	 * the animal has entered the network from the top-left quadrant.
	 */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 300);
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/*
	 * Start detection from top-middle LIDAR and middle-middle PIR:
	 * the animal has moved into the top-right quadrant.
	 */
	simulate_time(1);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 300);
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

	/*
	 * Start detection from left-middle LIDAR and right-middle PIR:
	 * the animal has moved into the bottom-right quadrant.
	 */
	simulate_time(1);
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1300);
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);

	/* After 2 seconds (1 + 1), the top-left LIDAR has stopped detecting. */
	test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2100);

	/*
	 * Continue detection from top-middle LIDAR and start detection for bottom-middle PIR:
	 * the animal has moved into the bottom-left quadrant.
	 */
	simulate_time(1);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1300);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/*
	 * Start detection from bottom-right LIDAR and bottom-left PIR:
	 * the animal has exited the network from the bottom-left quadrant.
	 */
	simulate_time(1);
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);

	/* After 2 seconds (1 + 1), the left-middle LIDAR has stopped detecting. */
	test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2100);

	/* After 2 seconds (1 + 1), the top-middle LIDAR has stopped detecting. */
	simulate_time(1);
	test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1300);

	/* After 2 seconds (1 + 1), the bottom-right LIDAR has stopped detecting. */
	simulate_time(1);
	test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2100);

	/* After 10 seconds, the top-middle PIR has stopped detecting.*/
	simulate_time(4);
	test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* After 10 seconds, the middle-middle PIR has stopped detecting.*/
	simulate_time(1);
	test_send_pir_data(0, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);

	/* After 10 seconds, the right-middle PIR has stopped detecting.*/
	simulate_time(1);
	test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);

	/* After 10 seconds, the bottom-middle PIR has stopped detecting.*/
	simulate_time(1);
	test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

	/* After 10 seconds, the bottom-left PIR has stopped detecting. */
	simulate_time(1);
	test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

	/* Approx 30 seconds after right-side detections, the right LED decays. */
	simulate_time(20);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* Approx 30 seconds after left-side detections, the middle and left LEDs decay. */
	simulate_time(1);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

	/* Approx 60 seconds after right-side detections, the right LED turns off. */
	simulate_time(29);
	oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

	/* Approx 60 seconds after left-side detections, the middle and left LEDs turn off.*/
	simulate_time(1);
	oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
	oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

	/* Idle to rest. */
	simulate_time(MINUTES(10));
}