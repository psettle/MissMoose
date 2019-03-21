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

extern "C" {
#include "mm_sensor_algorithm_config.h"
}


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

// Add the tests for one animal barely entering and quickly exiting the network
void test_one_animal_in_out_add_tests(std::vector<TestCase>& tests);

// Add the tests for one animal moving through the network and stopping once
void test_one_animal_with_one_stop_add_tests(std::vector<TestCase>& tests);

// Add the tests for animals starting slowly then suddenly changing direction and running
void test_slow_to_fast_running_animals(std::vector<TestCase>& tests);

// Add the tests for two animals moving through the network
void test_two_animals_through_network(std::vector<TestCase>& tests);

// Add the tests for animals being detected but not entering the network
void test_one_animal_outside_network_add_tests(std::vector<TestCase>& tests);

#endif /* TESTS_HPP */