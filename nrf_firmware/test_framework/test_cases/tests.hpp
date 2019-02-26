/**
file: tests.hpp
brief:
notes:
*/
#ifndef TESTS_HPP
#define TESTS_HPP

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_sensor_transmission.hpp"
#include "test_constants.hpp"
#include "test_output.hpp"
#include "simulate_time.hpp"

/**********************************************************
                          TYPES
**********************************************************/

typedef void(*test_case_cb)(TestOutput& oracle);

struct TestCase
{
    test_case_cb test;
    std::string  test_name;
};

/**********************************************************
                        CONSTANTS
**********************************************************/

#define ADD_TEST(test_case)   do{ tests.push_back(TestCase{ test_case, #test_case }); } while(0)

/**********************************************************
                       DECLARATIONS
**********************************************************/

void test_demo_add_tests(std::vector<TestCase>& tests);

// Adds the 4 tests to be run to the test runner
void test_basic_sensor_activity_add_tests(std::vector<TestCase>& tests);

// Adds the tests related to sensor hyperactiveness or inactiveness
void test_hyperactive_inactive_add_tests(std::vector<TestCase>& tests);

// Add the tests for one animal moving through the network at a constant speed
void test_one_animal_constant_speed_add_tests(std::vector<TestCase>& tests);

#endif /* TESTS_HPP */