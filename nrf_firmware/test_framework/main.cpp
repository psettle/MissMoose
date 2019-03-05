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
#include "test_parameters_utils.hpp"

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

int main(int argc, char* argv[])
{
    std::vector<TestCase> tests;

    if (argc > 1)
    {
        test_one_animal_constant_speed_add_tests(tests);
        test_one_animal_in_out_add_tests(tests);
        test_one_animal_with_one_stop_add_tests(tests);
        // We're getting some arguments from the genetic algorithm :D
        mm_sensor_algorithm_config_t parameters;
        test_demo_parse_parameters(&parameters, std::string(argv[0]), std::string(argv[1]));
        float score = test_runner_init(tests, &parameters);
        test_demo_write_score(score, std::string(argv[0]), std::string(argv[1]));
    }
    else
    {
        test_basic_sensor_activity_add_tests(tests);
        test_hyperactive_inactive_add_tests(tests);
        test_one_animal_constant_speed_add_tests(tests);
        test_one_animal_in_out_add_tests(tests);
        test_one_animal_with_one_stop_add_tests(tests);
        test_runner_init(tests, &sensor_algorithm_config_default);
    }

    return 0;
}