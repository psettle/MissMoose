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

#include "simulate_time.hpp"

#include <vector>
#include <string>

extern "C" {
#include "mm_sensor_algorithm_config.h"
}

/**********************************************************
                          TYPES
**********************************************************/

typedef void(*test_case_cb)(void);

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

void test_runner_init(std::vector<test_case_cb> const & tests, std::vector<std::string> test_names, std::string const & output_destination, mm_sensor_algorithm_config_t const * config);

#endif /* TEST_RUNNER_HPP */