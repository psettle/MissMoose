/**
file: test_runner.hpp
brief:
notes:
*/
#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

/**********************************************************
                        INCLUDES
**********************************************************/

#include "tests.hpp"
#include "simulate_time.hpp"

#include <vector>
#include <string>

extern "C" {
#include "mm_sensor_algorithm_config.h"
}

/**********************************************************
                          TYPES
**********************************************************/

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

void test_runner_init(std::vector<TestCase> const & tests, mm_sensor_algorithm_config_t const * config);

#endif /* TEST_RUNNER_HPP */