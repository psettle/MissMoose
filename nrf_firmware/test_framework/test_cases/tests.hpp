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

void test_demo_add_tests(std::vector<test_case_cb>& tests);

#endif /* TESTS_HPP */