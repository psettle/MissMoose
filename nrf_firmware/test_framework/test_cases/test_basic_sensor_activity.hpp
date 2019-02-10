/**
file: test_basic_sensor_activity.hpp
brief: Testing basic sensor features - Testing detections from all PIR sensors, all Lidar sensor, and
allowing activity variables to drain after messages are sent
notes:

Author: Elijah Pennoyer
*/

#ifndef TEST_BASIC_SENSOR_ACTIVITY_HPP
#define TEST_BASIC_SENSOR_ACTIVITY_HPP

/**********************************************************
						INCLUDES
**********************************************************/
#include "tests.hpp"

/**********************************************************
					   DECLARATIONS
**********************************************************/
// Adds the 4 tests to be run to the test runner
void test_basic_sensor_activity_add_tests(std::vector<test_case_cb>& tests, std::vector<std::string>& test_names);

#endif // TEST_BASIC_SENSOR_ACTIVITY_HPP