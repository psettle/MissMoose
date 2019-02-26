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

/**********************************************************
CONSTANTS
**********************************************************/

/**********************************************************
DECLARATIONS
**********************************************************/


/**
* Parses the parameters for the algorithm out of a binary file.
*/
void test_demo_parse_parameters(std::vector<float> & parameters, const std::string executable_path, std::string individual_index);

/**
* Writes the score for testing to a binary file.
*/
void test_demo_write_score(float score, const std::string executable_path, std::string individual_index);

#endif /* #define TEST_PARAMETERS_UTILS_HPP
*/