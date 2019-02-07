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

/**********************************************************
                       DECLARATIONS
**********************************************************/

/* Tests the algorithm running idle for 3 days. */
static void test_case_3_days_idle(void);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_demo_add_tests(std::vector<test_case_cb>& tests, std::vector<std::string>& test_names)
{
	ADD_TEST(test_case_3_days_idle)
}

static void test_case_3_days_idle(void)
{
    simulate_time(DAYS(3));
}