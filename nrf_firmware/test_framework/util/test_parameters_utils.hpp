/**
file: test_parameters_utils.hpp
brief:
notes:
*/
#ifndef TEST_PARAMETERS_UTILS_HPP
#define TEST_PARAMETERS_UTILS_HPP

/**********************************************************
INCLUDES
**********************************************************/

#include <string>
#include <vector>
#include "test_runner.hpp"
#include "tests.hpp"

/**********************************************************
CONSTANTS
**********************************************************/

/**********************************************************
DECLARATIONS
**********************************************************/


/**
* Parses the parameters for the algorithm out of a binary file.
*/
void test_demo_parse_parameters(mm_sensor_algorithm_config_t* config, std::string const & executable_path, std::string const & individual_index);

/**
* Writes the score for testing to a binary file.
*/
void test_demo_write_score(float score, std::string const & executable_path, std::string const & individual_index);

#endif /* #define TEST_PARAMETERS_UTILS_HPP
*/