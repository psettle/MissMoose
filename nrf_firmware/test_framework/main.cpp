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

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

int main()
{
    std::vector<test_case_cb> tests;
    std::vector<std::string> test_names;

    test_demo_add_tests(tests, test_names);

    test_runner_init(tests, test_names, "");

    return 0;
}