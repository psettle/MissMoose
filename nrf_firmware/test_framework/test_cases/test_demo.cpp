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
#define LIDAR_DETECT_DISTANCE_350CM         (350) //350 cm = 3.5 meters
#define LIDAR_DETECT_DISTANCE_OUTOFRANGE    (30000) //30 meters - Outside of the range lidars are used to detect
#define PIR_DETECTION_START                 (true) //Used to indicate that a PIR has started detecting
#define PIR_DETECTION_END                   (false) //Used to indicate that a PIR has stopped detecting

/**********************************************************
                       DECLARATIONS
**********************************************************/

/* Tests the algorithm running idle for 3 days. */
static void test_case_3_days_idle(void);
/* Test sending PIR and Lidar data */
static void test_case_test_sending_sensor_data(void);
// Test sending sensor data from every PIR sensor in the network.
static void test_case_detect_from_all_pirs(void);
// Test sending sensor data from every lidar sensor in the network.
static void test_case_detect_from_all_lidars(void);
// Tests letting a PIR charge up an activity variable then let the AV drain again
static void test_case_pir_on_let_drain_to_1(void);
// Tests letting a PIR charge up an activity variable then let the AV drain again
static void test_case_lidar_on_let_drain_to_1(void);


/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_demo_add_tests(std::vector<test_case_cb>& tests, std::vector<std::string>& test_names)
{
    ADD_TEST(test_case_3_days_idle);
    ADD_TEST(test_case_test_sending_sensor_data);

    //Basic tests
    ADD_TEST(test_case_detect_from_all_pirs);
    ADD_TEST(test_case_detect_from_all_lidars);
    ADD_TEST(test_case_pir_on_let_drain_to_1);
    ADD_TEST(test_case_lidar_on_let_drain_to_1);
}

static void test_case_3_days_idle(void)
{
    simulate_time(DAYS(3));
}

static void test_case_test_sending_sensor_data(void)
{
    // Test sending PIR and lidar data
    test_send_pir_data(2, SENSOR_ROTATION_180, PIR_DETECTION_START);
    test_send_lidar_data(3, SENSOR_ROTATION_0, 100);
    // Simulate time passing
    simulate_time(MINUTES(5));
}

// Test sending sensor data from every PIR sensor in the network.
// Allows 1 minute between tests to make sure nothing odd happens as the result of any 1 detection.
static void test_case_detect_from_all_pirs(void)
{
    // Node 1 - 1 PIR, sensor rotation 90 degrees
    test_send_pir_data(1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(MINUTES(1));

    // Node 2 - 1 PIR, sensor rotation 90 degrees
    test_send_pir_data(2, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(MINUTES(1));

    // Node 3 - 1 PIR, sensor rotation 90 degrees
    test_send_pir_data(3, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(MINUTES(1));

    // Node 4 - 1 PIR, sensor rotation 90 degrees
    test_send_pir_data(4, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(MINUTES(1));

    // Node 5 - 2 PIRs, sensor rotations of 0 and 270 degrees
    test_send_pir_data(5, SENSOR_ROTATION_0, PIR_DETECTION_START);
    test_send_pir_data(5, SENSOR_ROTATION_270, PIR_DETECTION_START);

    // Node 6 - 2 PIRs, sensor rotations of 0 and 270 degrees
    test_send_pir_data(6, SENSOR_ROTATION_0, PIR_DETECTION_START);
    simulate_time(MINUTES(1));
    test_send_pir_data(6, SENSOR_ROTATION_270, PIR_DETECTION_START);
    simulate_time(MINUTES(1));

    // Node 7 - 1 PIR, sensor rotation 90 degrees
    test_send_pir_data(7, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(MINUTES(1));

    // Node 8 - 2 PIRs, sensor rotations of 0 and 270 degrees
    test_send_pir_data(8, SENSOR_ROTATION_0, PIR_DETECTION_START);
    simulate_time(MINUTES(1));
    test_send_pir_data(8, SENSOR_ROTATION_270, PIR_DETECTION_START);
    simulate_time(MINUTES(1));

    // Node 9 - 1 PIR, sensor rotation 90 degrees
    test_send_pir_data(9, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(MINUTES(1));

}


// Test sending sensor data from every Lidar sensor in the network.
// Allows 1 minute between tests to make sure nothing odd happens as the result of any 1 detection.
static void test_case_detect_from_all_lidars(void)
{

    // Node 1 - 1 Lidar, sensor rotation 0 degrees
    test_send_lidar_data(1, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
    simulate_time(MINUTES(1));

    // Node 2 - 1 Lidar, sensor rotation 0 degrees
    test_send_lidar_data(2, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
    simulate_time(MINUTES(1));

    // Node 3 - 1 Lidar, sensor rotation 0 degrees
    test_send_lidar_data(3, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
    simulate_time(MINUTES(1));

    // Node 4 - 1 Lidar, sensor rotation 0 degrees
    test_send_lidar_data(4, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
    simulate_time(MINUTES(1));

    // Node 7 - 1 Lidar, sensor rotation 0 degrees
    test_send_lidar_data(7, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
    simulate_time(MINUTES(1));

    // Node 9 - 1 Lidar, sensor rotation 0 degrees
    test_send_lidar_data(9, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
    simulate_time(MINUTES(1));

}

// Tests letting a PIR charge up an activity variable then let the AV drain again
static void test_case_pir_on_let_drain_to_1(void)
{
    // Use node 1's PIR. 
    test_send_pir_data(1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(15); //Let 15 seconds pass so that the AV charges up
    test_send_pir_data(1, SENSOR_ROTATION_90, PIR_DETECTION_END); //No longer detecting

    simulate_time(DAYS(1)); //Let the variable drain
}


// Tests letting a PIR charge up an activity variable then let the AV drain again
static void test_case_lidar_on_let_drain_to_1(void)
{
    // Use node 1's lidar. Send that a detection occurred.
    test_send_lidar_data(1, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_350CM);
    simulate_time(15); //Let 15 seconds pass so that the AV charges up
    test_send_lidar_data(1, SENSOR_ROTATION_0, LIDAR_DETECT_DISTANCE_OUTOFRANGE);

    simulate_time(DAYS(1)); //Let the variable drain
}