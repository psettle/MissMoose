/**
file: test_hyperactive_inactive.cpp
brief:
notes:
*/

/**********************************************************
INCLUDES
**********************************************************/

#include "tests.hpp"
#include "sensor_evt_utils.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
extern "C" {
#include "mm_sensor_error_check.h"
#include "mm_sensor_algorithm_config.h"
}

/**********************************************************
CONSTANTS
**********************************************************/

/**********************************************************
DECLARATIONS
**********************************************************/

/* Test that all the sensors will be not flagged as hyperactive after normal activity.*/
static void test_case_test_sensor_normal_activity(void);
/* Test that all the sensors will be flagged as hyperactive */
static void test_case_test_sensor_hyperactivity(void);
/* Make sure that hyperactive sensors wouldn't be called hyperactive anymore after a long cooldown period. */
static void test_case_test_sensor_hyperactivity_cooldown(void);
/* Test to see if the sensors are marked as inactive after an appropriately long time. */
static void test_case_test_sensor_inactivity(void);
/* Test to make sure that inactivity flags on different sensors will update as time passes. */
static void test_case_test_inactive_state_update(void);

/**********************************************************
DEFINITIONS
**********************************************************/

void test_hyperactive_inactive_add_tests(std::vector<test_case_cb>& tests, std::vector<std::string>& test_names)
{
    ADD_TEST(test_case_test_sensor_normal_activity);
    ADD_TEST(test_case_test_sensor_hyperactivity);
    ADD_TEST(test_case_test_sensor_hyperactivity_cooldown);
    ADD_TEST(test_case_test_sensor_inactivity);
    ADD_TEST(test_case_test_inactive_state_update);
}

static void test_case_test_sensor_normal_activity(void)
{
    std::vector<sensor_evt_t> pir_evts;
    std::vector<sensor_evt_t> lidar_evts;
    create_all_pir_sensor_evts(pir_evts, PIR_DETECTION_START);
    create_all_lidar_sensor_evts(lidar_evts, 100);
    // Test sending data at a reasonable frequency
    for (int i = 0; i < SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE + 2; i++)
    {
        for (auto const & evt : pir_evts)
        {
            test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, PIR_DETECTION_START);
        }
        for (auto const & evt : lidar_evts)
        {
            test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
        }
        simulate_time(MINUTES(SENSOR_HYPERACTIVITY_FREQUENCY_THRES) + 5);
    }

    // If any sensors are listed as hyperactive, that's a fail.
    for (auto const & evt : pir_evts)
    {
        if (mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is hyperactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto const & evt : lidar_evts)
    {
        if (mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "LIDAR sensor on node ID " << evt.lidar_data.node_id << " at rotation " << evt.lidar_data.sensor_rotation << " is hyperactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }
}

static void test_case_test_sensor_hyperactivity(void)
{
    std::vector<sensor_evt_t> pir_evts;
    std::vector<sensor_evt_t> lidar_evts;
    create_all_pir_sensor_evts(pir_evts, PIR_DETECTION_START);
    create_all_lidar_sensor_evts(lidar_evts, 100);
    // Test sending data at a high frequency
    for (int i = 0; i < SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE + 2; i++)
    {
        for (auto const & evt : pir_evts)
        {
            test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, PIR_DETECTION_START);
        }
        for (auto const & evt : lidar_evts)
        {
            test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
        }
        simulate_time(MINUTES(SENSOR_HYPERACTIVITY_FREQUENCY_THRES) - 5);
    }

    // If any sensors are listed as not hyperactive, that's a fail.
    for (auto const & evt : pir_evts)
    {
        if (!mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is not hyperactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto const & evt : lidar_evts)
    {
        if (!mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "LIDAR sensor on node ID " << evt.lidar_data.node_id << " at rotation " << evt.lidar_data.sensor_rotation << " is not hyperactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
}

static void test_case_test_sensor_hyperactivity_cooldown(void)
{
    std::vector<sensor_evt_t> pir_evts;
    std::vector<sensor_evt_t> lidar_evts;
    create_all_pir_sensor_evts(pir_evts, true);
    create_all_lidar_sensor_evts(lidar_evts, 100);
    // Test sending data at a high frequency
    for (int i = 0; i < SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE + 2; i++)
    {
        for (auto const & evt : pir_evts)
        {
            test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, PIR_DETECTION_START);
        }
        for (auto const & evt : lidar_evts)
        {
            test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
        }
        simulate_time(MINUTES(SENSOR_HYPERACTIVITY_FREQUENCY_THRES) - 5);
    }

    /* While not the purpose of this test, if any sensors are listed as not hyperactive, that's also a fail
    and prevents us from running the rest of the test. */
    for (auto const & evt : pir_evts)
    {
        if (!mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is not hyperactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto const & evt : lidar_evts)
    {
        if (!mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "LIDAR sensor on node ID " << evt.lidar_data.node_id << " at rotation " << evt.lidar_data.sensor_rotation << " is not hyperactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
    // Wait long enough to clear out any hyperactivity flags, then send one more event.
    simulate_time(MINUTES(SENSOR_HYPERACTIVITY_EVENT_WINDOW_SIZE / SENSOR_HYPERACTIVITY_FREQUENCY_THRES));

    for (auto const & evt : pir_evts)
    {
        test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, PIR_DETECTION_START);
    }
    for (auto const & evt : lidar_evts)
    {
        test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
    }
    simulate_time(MINUTES(1));

    // If any sensors are listed as hyperactive, that's a fail.
    for (auto const & evt : pir_evts)
    {
        if (mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is hyperactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto const & evt : lidar_evts)
    {
        if (mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "LIDAR sensor on node ID " << evt.lidar_data.node_id << " at rotation " << evt.lidar_data.sensor_rotation << " is hyperactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }
}

static void test_case_test_sensor_inactivity(void)
{

    std::vector<sensor_evt_t> pir_evts;
    std::vector<sensor_evt_t> lidar_evts;
    create_all_pir_sensor_evts(pir_evts, true);
    create_all_lidar_sensor_evts(lidar_evts, 100);
    // Start by sending some data to each of the sensors
    for (auto const & evt : pir_evts)
    {
        test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, PIR_DETECTION_START);
    }
    for (auto const & evt : lidar_evts)
    {
        test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
    }
    simulate_time(MINUTES(1));

    /* Make sure that all the sensors are not marked as inactive */
    for (auto const & evt : pir_evts)
    {
        if (mm_sensor_error_is_sensor_inactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is inactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto const & evt : lidar_evts)
    {
        if (mm_sensor_error_is_sensor_inactive(&evt))
        {
            std::stringstream ss;
            ss << "LIDAR sensor on node ID " << evt.lidar_data.node_id << " at rotation " << evt.lidar_data.sensor_rotation << " is inactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }

    // Wait long enough to cause the sensors to be marked as inactive
    simulate_time(MINUTES(SENSOR_INACTIVITY_THRESHOLD_MIN + 1));

    // If any sensors are listed as not inactive, that's a fail.
    for (auto const & evt : pir_evts)
    {
        if (!mm_sensor_error_is_sensor_inactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is not inactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto const & evt : lidar_evts)
    {
        if (!mm_sensor_error_is_sensor_inactive(&evt))
        {
            std::stringstream ss;
            ss << "LIDAR sensor on node ID " << evt.lidar_data.node_id << " at rotation " << evt.lidar_data.sensor_rotation << " is not inactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
}

static void test_case_test_inactive_state_update(void)
{
    sensor_evt_t pir_1_evt = create_pir_sensor_evt(1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    sensor_evt_t pir_2_evt = create_pir_sensor_evt(2, SENSOR_ROTATION_90, PIR_DETECTION_START);

    // Start by sending data to PIR sensor 1
    test_send_pir_data(1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(HOURS(12));

    //12 Hours later send data to pir sensor 2
    test_send_pir_data(2, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(HOURS(13));

    // By now Sensor 1 should be inactive and sensor 2 should be active
    if (!(mm_sensor_error_is_sensor_inactive(&pir_1_evt) && !mm_sensor_error_is_sensor_inactive(&pir_2_evt)))
    {
        std::stringstream ss;
        ss << "Error: PIR on node 1 should be inactive and PIR on node 2 should be active.";
        throw std::runtime_error(ss.str());
    }

    simulate_time(HOURS(12));

    // By now Sensor 1 should be inactive and sensor 2 should be inactive
    if (!(mm_sensor_error_is_sensor_inactive(&pir_1_evt) && mm_sensor_error_is_sensor_inactive(&pir_2_evt)))
    {
        std::stringstream ss;
        ss << "Error: PIR on node 1 and PIR on node 2 should be inactive.";
        throw std::runtime_error(ss.str());
    }
}