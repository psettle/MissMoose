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
/* Test sending PIR and Lidar data */
static void test_case_test_sending_sensor_data(void);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_demo_add_tests(std::vector<test_case_cb>& tests, std::vector<std::string>& test_names)
{
	ADD_TEST(test_case_3_days_idle);
    ADD_TEST(test_case_test_sending_sensor_data);
}

static void test_case_3_days_idle(void)
{
    simulate_time(DAYS(3));
}

static void test_case_test_sending_sensor_data(void)
{
    //Test sending PIR and lidar data
    test_send_pir_data(2, SENSOR_ROTATION_180, true);
    test_send_lidar_data(3, SENSOR_ROTATION_0, 100);
    //Simulate time passing
    simulate_time(MINUTES(5));
}