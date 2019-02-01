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

void test_runner_init(std::vector<test_case_cb> const & tests, std::string const & output_destination);

#endif /* TEST_RUNNER_HPP */