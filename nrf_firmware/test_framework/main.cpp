/**
 file: main.cpp
 brief:
 notes:
 */

/**********************************************************
                        INCLUDES
**********************************************************/

#include "test_runner.hpp"
#include "tests.hpp"

/**********************************************************
                        VARIABLES
**********************************************************/

/**
	Default sensor algorithm configuration constants.
*/
static mm_sensor_algorithm_config_t const sensor_algorithm_config_default =
{
	1.0f,   // activity_variable_min
	12.0f,  // activity_variable_max

	1.0f, // common_sensor_weight_factor
	3.0f, // base_sensor_weight_factor_pir
	3.5f, // base_sensor_weight_factor_lidar
	1.4f, // road_proximity_factor_0
	1.2f, // road_proximity_factor_1
	1.0f, // road_proximity_factor_2

	1.0f,       // common_sensor_trickle_factor
	1.003f,     // base_sensor_trickle_factor_pir
	1.0035f,    // base_sensor_trickle_factor_lidar
	1.004f,     // road_trickle_proximity_factor_0
	1.002f,     // road_trickle_proximity_factor_1
	1.0f,       // road_trickle_proximity_factor_2

	0.99f, // activity_variable_decay_factor
	1000,  // activity_decay_period_ms

	3.0f, // possible_detection_threshold_rs
	4.0f, // possible_detection_threshold_nrs

	6.0f, // detection_threshold_rs
	7.0f, // detection_threshold_nrs

	30, // minimum_concern_signal_duration_s
	60 // minimum_alarm_signal_duration_s
};


/**********************************************************
                       DEFINITIONS
**********************************************************/

int main()
{
    std::vector<test_case_cb> tests;
    std::vector<std::string> test_names;

	test_demo_add_tests(tests, test_names);
    test_basic_sensor_activity_add_tests(tests, test_names);
    test_hyperactive_inactive_add_tests(tests, test_names);

    test_runner_init(tests, test_names, "", &sensor_algorithm_config_default);

    return 0;
}