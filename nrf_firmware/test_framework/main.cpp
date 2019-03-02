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
    std::vector<TestCase> tests;

    test_basic_sensor_activity_add_tests(tests);
    test_hyperactive_inactive_add_tests(tests);

    test_runner_init(tests);

    return 0;
}