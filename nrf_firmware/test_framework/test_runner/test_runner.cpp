/**
file: test_runner.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <iostream>

#include "test_runner.hpp"
#include "test_output_logger.hpp"
#include "mm_led_control.hpp"

extern "C" {
#include "mm_sensor_algorithm_config.h"
#include "mm_sensor_algorithm.h"
#include "mm_sensor_transmission.h"
#include "mm_monitoring_dispatch.h"
#include "mm_position_config.h"
#include "mm_led_control.h"
#include "mm_av_transmission.h"
}


/**********************************************************
					   VARIABLES
**********************************************************/

static mm_sensor_algorithm_config_t const * sensor_algorithm_config;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Run a particular test
 */
static float run_test_case(TestCase const & test);

/**
 * Prepare for test run by initializing all components and utilities.
 */
static void init_test_case(std::string const & test_name);

/**
 * Finalize test run by saving output and shutting down utilities.
 */
static void deinit_test_case(void);

/**********************************************************
                       DEFINITIONS
**********************************************************/

float test_runner_init(std::vector<TestCase> const & tests, mm_sensor_algorithm_config_t const * config)
{
	sensor_algorithm_config = config;
    float overall_score = 0;

	for (int i = 0; i < tests.size(); i++)
    {
        overall_score += run_test_case(tests[i]);
    }
    return overall_score / tests.size();
}

static float run_test_case(TestCase const & test)
{
    float test_score = 0.0f;

    init_test_case(test.test_name);

    try
    {
        TestOutput oracle;
        oracle.initOracle();
        test.test(oracle);
        auto result = test_led_control_get_output();
        test_score = TestOutput::getMatchScore(result, oracle);
    }
    catch (const std::exception& ex) /* Catch everything, who knows what the test code could do! */
    {
        std::cout << std::string("Test \"") + test.test_name + std::string("\" Failed: ") + ex.what() << std::endl;
    }
    
    deinit_test_case();

    std::cout << "Ran " << test.test_name << " with score of " << test_score << std::endl;

    return test_score;
}

static void init_test_case(std::string const & test_name)
{
    simulate_time_init();
    mm_position_config_init();
    mm_led_control_init();
    mm_monitoring_dispatch_init();
    mm_sensor_transmission_init();
    mm_sensor_algorithm_init(sensor_algorithm_config);
    mm_av_transmission_init();

    /* Initialize the logger for this test. */
	init_test_output_logger(test_name);
}

static void deinit_test_case(void)
{
	/* Deinitialize the logger for the next test. */
	deinit_test_output_logger();
}