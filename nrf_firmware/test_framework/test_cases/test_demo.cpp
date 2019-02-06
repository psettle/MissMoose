/**
file: test_demo.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "tests.hpp"


/**********************************************************
						CONSTANTS
**********************************************************/

#define ADD_TEST(test_case)   tests.push_back(test_case); test_names.push_back(#test_case);

/**********************************************************
                       DECLARATIONS
**********************************************************/

/* Tests the algorithm running idle for 3 days. */
static void test_case_3_days_idle(void);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_demo_add_tests(std::vector<test_case_cb>& tests, std::vector<const char*>& test_names)
{
	ADD_TEST(test_case_3_days_idle)
}

static void test_case_3_days_idle(void)
{
    simulate_time(DAYS(3));
}