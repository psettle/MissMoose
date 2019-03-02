/**
file: test_basic_sensor_activity.cpp
brief: Testing basic sensor features - Testing detections from all PIR sensors, all Lidar sensor, and
allowing activity variables to drain after messages are sent
notes:

Author: Elijah Pennoyer
*/


/**********************************************************
						INCLUDES
**********************************************************/
#include "tests.hpp"

/**********************************************************
					   DECLARATIONS
**********************************************************/

// Test sending sensor data from every PIR sensor in the network.
static void test_case_detect_from_all_pirs(TestOutput& oracle);
// Test sending sensor data from every lidar sensor in the network.
static void test_case_detect_from_all_lidars(TestOutput& oracle);
// Tests letting a PIR charge up an activity variable then let the AV drain again
static void test_case_pir_on_let_drain_to_1(TestOutput& oracle);
// Tests letting a PIR charge up an activity variable then let the AV drain again
static void test_case_lidar_on_let_drain_to_1(TestOutput& oracle);

/**********************************************************
					   DEFINITIONS
**********************************************************/

void test_basic_sensor_activity_add_tests(std::vector<TestCase>& tests)
{
	//Basic tests
	ADD_TEST(test_case_detect_from_all_pirs);
	ADD_TEST(test_case_detect_from_all_lidars);
	ADD_TEST(test_case_pir_on_let_drain_to_1);
	ADD_TEST(test_case_lidar_on_let_drain_to_1);
}


// Test sending sensor data from every PIR sensor in the network.
// Allows 1 minute between tests to make sure nothing odd happens as the result of any 1 detection.
static void test_case_detect_from_all_pirs(TestOutput& oracle)
{
	// Node 1 - 1 PIR, sensor rotation 90 degrees
	test_send_pir_data(1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	simulate_time(MINUTES(1));

	// Node 2 - 1 PIR, sensor rotation 90 degrees
	test_send_pir_data(2, SENSOR_ROTATION_90, PIR_DETECTION_START);
	simulate_time(MINUTES(1));

	// Node 3 - 1 PIR, sensor rotation 90 degrees
	test_send_pir_data(3, SENSOR_ROTATION_90, PIR_DETECTION_START);
	simulate_time(MINUTES(1));

	// Node 4 - 1 PIR, sensor rotation 90 degrees
	test_send_pir_data(4, SENSOR_ROTATION_90, PIR_DETECTION_START);
	simulate_time(MINUTES(1));

	// Node 5 - 2 PIRs, sensor rotations of 0 and 270 degrees
	test_send_pir_data(5, SENSOR_ROTATION_0, PIR_DETECTION_START);
	test_send_pir_data(5, SENSOR_ROTATION_270, PIR_DETECTION_START);

	// Node 6 - 2 PIRs, sensor rotations of 0 and 270 degrees
	test_send_pir_data(6, SENSOR_ROTATION_0, PIR_DETECTION_START);
	simulate_time(MINUTES(1));
	test_send_pir_data(6, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(MINUTES(1));

	// Node 7 - 1 PIR, sensor rotation 90 degrees
	test_send_pir_data(7, SENSOR_ROTATION_90, PIR_DETECTION_START);
	simulate_time(MINUTES(1));

	// Node 8 - 2 PIRs, sensor rotations of 0 and 270 degrees
	test_send_pir_data(8, SENSOR_ROTATION_0, PIR_DETECTION_START);
	simulate_time(MINUTES(1));
	test_send_pir_data(8, SENSOR_ROTATION_270, PIR_DETECTION_START);
	simulate_time(MINUTES(1));

	// Node 9 - 1 PIR, sensor rotation 90 degrees
	test_send_pir_data(9, SENSOR_ROTATION_90, PIR_DETECTION_START);
	simulate_time(MINUTES(1));

}


// Test sending sensor data from every Lidar sensor in the network.
// Allows 1 minute between tests to make sure nothing odd happens as the result of any 1 detection.
static void test_case_detect_from_all_lidars(TestOutput& oracle)
{

	// Node 1 - 1 Lidar, sensor rotation 0 degrees
	test_send_lidar_data(1, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
	simulate_time(MINUTES(1));

	// Node 2 - 1 Lidar, sensor rotation 0 degrees
	test_send_lidar_data(2, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
	simulate_time(MINUTES(1));

	// Node 3 - 1 Lidar, sensor rotation 0 degrees
	test_send_lidar_data(3, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
	simulate_time(MINUTES(1));

	// Node 4 - 1 Lidar, sensor rotation 0 degrees
	test_send_lidar_data(4, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
	simulate_time(MINUTES(1));

	// Node 7 - 1 Lidar, sensor rotation 0 degrees
	test_send_lidar_data(7, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
	simulate_time(MINUTES(1));

	// Node 9 - 1 Lidar, sensor rotation 0 degrees
	test_send_lidar_data(9, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
	simulate_time(MINUTES(1));

}

// Tests letting a PIR charge up an activity variable then let the AV drain again
static void test_case_pir_on_let_drain_to_1(TestOutput& oracle)
{
	// Use node 1's PIR. 
	test_send_pir_data(1, SENSOR_ROTATION_90, PIR_DETECTION_START);
	simulate_time(15); //Let 15 seconds pass so that the AV charges up
	test_send_pir_data(1, SENSOR_ROTATION_90, PIR_DETECTION_END); //No longer detecting

	simulate_time(DAYS(1)); //Let the variable drain
}


// Tests letting a PIR charge up an activity variable then let the AV drain again
static void test_case_lidar_on_let_drain_to_1(TestOutput& oracle)
{
	// Use node 1's lidar. Send that a detection occurred.
	test_send_lidar_data(1, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
	simulate_time(15); //Let 15 seconds pass so that the AV charges up
	test_send_lidar_data(1, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_OUTOFRANGE);

	simulate_time(DAYS(1)); //Let the variable drain
}