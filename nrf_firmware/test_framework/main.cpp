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
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

int main(int argc, char* argv[])
{
    std::vector<test_case_cb> tests;
    std::vector<std::string> test_names;
    std::vector<float> parameters;
    if (argc > 1)
    {
        test_demo_parse_parameters(parameters, std::string(argv[0]), std::string(argv[1]));
    }
    test_demo_add_tests(tests, test_names);
    test_basic_sensor_activity_add_tests(tests, test_names);
    test_hyperactive_inactive_add_tests(tests, test_names);

    test_runner_init(tests, test_names, "");

    if (argc > 1)
    {
        test_demo_write_score(10.0, std::string(argv[0]), std::string(argv[1]));
    }

    return 0;
}