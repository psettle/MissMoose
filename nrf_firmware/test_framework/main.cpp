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
	Default sensor algorithm configuration constants. 0.51949
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

/* Best -pre-eli tests 0.6332615613937378 (0.837667 now)*/
static mm_sensor_algorithm_config_t const sensor_algorithm_config_1551863129 =
{
    4.3985562363, /* activity_variable_min */
    260.2637593756, /* activity_variable_max */
    1.0000000000, /* common_sensor_weight_factor */
    1.2718169501, /* base_sensor_weight_factor_pir */
    6.2389932065, /* base_sensor_weight_factor_lidar */
    1.0000000000, /* road_proximity_factor_0 */
    1.0000000000, /* road_proximity_factor_1 */
    1.0000000000, /* road_proximity_factor_2 */
    1.0000000000, /* common_sensor_trickle_factor */
    1.3020024630, /* base_sensor_trickle_factor_pir */
    1.0000000000, /* base_sensor_trickle_factor_lidar */
    3.0530560759, /* road_trickle_proximity_factor_0 */
    1.0000000000, /* road_trickle_proximity_factor_1 */
    1.0000000000, /* road_trickle_proximity_factor_2 */
    0.9407985368, /* activity_variable_decay_factor */
    14.8725093912, /* activity_decay_period_ms */
    6.5727234744, /* possible_detection_threshold_rs */
    6.4876488286, /* possible_detection_threshold_nrs */
    42.1739123043, /* detection_threshold_rs */
    40.4021446771, /* detection_threshold_nrs */
    4.2276560074, /* minimum_concern_signal_duration_s */
    1.0000000000, /* minimum_alarm_signal_duration_s */
};

/* Best -pre-reinforcement 0.841142 */
static mm_sensor_algorithm_config_t const sensor_algorithm_config_1551911794 =
{
    2.1684160132, /* activity_variable_min */
    173.0773543340, /* activity_variable_max */
    1.0000000000, /* common_sensor_weight_factor */
    1.0000000000, /* base_sensor_weight_factor_pir */
    1.0000000000, /* base_sensor_weight_factor_lidar */
    5.4670607275, /* road_proximity_factor_0 */
    6.3722118535, /* road_proximity_factor_1 */
    6.7627315568, /* road_proximity_factor_2 */
    1.0000000000, /* common_sensor_trickle_factor */
    1.2259346369, /* base_sensor_trickle_factor_pir */
    1.0000000000, /* base_sensor_trickle_factor_lidar */
    1.0000000000, /* road_trickle_proximity_factor_0 */
    1.0000000000, /* road_trickle_proximity_factor_1 */
    1.0000000000, /* road_trickle_proximity_factor_2 */
    0.9535216014, /* activity_variable_decay_factor */
    1.0000000000, /* activity_decay_period_ms */
    9.7637955926, /* possible_detection_threshold_rs */
    9.7837836894, /* possible_detection_threshold_nrs */
    41.3127704940, /* detection_threshold_rs */
    40.0230364488, /* detection_threshold_nrs */
    36.9740456420, /* minimum_concern_signal_duration_s */
    1.0000000000, /* minimum_alarm_signal_duration_s */
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
        test_slow_to_fast_running_animals(tests);
        test_two_animals_through_network(tests);
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
        test_slow_to_fast_running_animals(tests);
        test_two_animals_through_network(tests);
		test_one_animal_zig_zag(tests);
		test_more_than_two_animals_through_network(tests);
		test_sensors_not_working(tests);
        test_runner_init(tests, &sensor_algorithm_config_default);
    }

    return 0;
}