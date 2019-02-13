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

#include "test_runner.hpp"
#include "mm_sensor_transmission.hpp"

/**********************************************************
                          TYPES
**********************************************************/

typedef void(*test_case_cb)(void);

/**********************************************************
                        CONSTANTS
**********************************************************/

#define ADD_TEST(test_case)  do{ tests.push_back(test_case); test_names.push_back(#test_case); } while(0) 

/**********************************************************
                       DECLARATIONS
**********************************************************/

void test_demo_add_tests(std::vector<test_case_cb>& tests, std::vector<std::string>& test_names);

#endif /* TESTS_HPP */