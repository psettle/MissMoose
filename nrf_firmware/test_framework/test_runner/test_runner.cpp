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

extern "C" {
#include "mm_sensor_algorithm.h"
#include "mm_sensor_transmission.h"
#include "mm_monitoring_dispatch.h"
#include "mm_position_config.h"
#include "mm_led_control.h"
}

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Run a particular test
 */
static void run_test_case(test_case_cb test, std::string const & test_name, std::string const & output_destination);

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

void test_runner_init(std::vector<test_case_cb> const & tests, std::vector<std::string> test_names, std::string const & output_destination)
{
    for (int i = 0; i < tests.size(); i++)
    {
        run_test_case(tests[i], test_names[i], output_destination);
    }
}

static void run_test_case(test_case_cb test, std::string const & test_name, std::string const & output_destination)
{
    init_test_case(test_name);

    try
    {
        test();
    }
    catch (const std::exception& ex) /* Catch everything, who knows what the test code could do! */
    {
        std::cout << std::string("Test \"") + test_name + std::string("\" Failed: ") + ex.what() << std::endl;
    }
    
    deinit_test_case();
}

static void init_test_case(std::string const & test_name)
{
    
    mm_position_config_init();
    mm_led_control_init();
    mm_monitoring_dispatch_init();
    mm_sensor_transmission_init();
    mm_sensor_algorithm_init();

    /* Initialize the logger for this test. */
	init_test_output_logger(test_name);
}

static void deinit_test_case(void)
{
	/* Deinitialize the logger for the next test. */
	deinit_test_output_logger();
}