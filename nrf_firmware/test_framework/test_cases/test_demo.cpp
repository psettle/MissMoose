/**
file: test_demo.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "tests.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
extern "C" {
#include "mm_sensor_error_check.h"
}

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
/* Test that all the sensors will be not flagged as hyperactive after normal activity.*/
static void test_case_test_sensor_normal_activity(void);
/* Test that all the sensors will be flagged as hyperactive */
static void test_case_test_sensor_hyperactivity(void);
/* Make sure that hyperactive sensors wouldn't be called hyperactive anymore after a long cooldown period. */
static void test_case_test_sensor_hyperactivity_cooldown(void);
/* Create big list of pir sensor events so that we can use a function to check if a sensor is hyperactive. */
static void create_all_pir_sensor_evts(std::vector<sensor_evt_t>& evts, bool detection);
/* Create big list of lidar sensor events so that we can use a function to check if a sensor is hyperactive. */
static void create_all_lidar_sensor_evts(std::vector<sensor_evt_t>& evts, uint16_t distance);
/* Helper function for making events so we can call is_sensor_hyperactive */
static sensor_evt_t create_lidar_sensor_evt
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    uint16_t distance_measured
);

/* Helper function for making events so we can call is_sensor_hyperactive */
static sensor_evt_t create_pir_sensor_evt
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    bool detection
);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_demo_add_tests(std::vector<test_case_cb>& tests, std::vector<std::string>& test_names)
{
    ADD_TEST(test_case_3_days_idle);
    ADD_TEST(test_case_test_sending_sensor_data);
    ADD_TEST(test_case_test_sensor_normal_activity);
    ADD_TEST(test_case_test_sensor_hyperactivity);
    ADD_TEST(test_case_test_sensor_hyperactivity_cooldown);
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

static void test_case_test_sensor_normal_activity(void)
{
    std::vector<sensor_evt_t> pir_evts;
    std::vector<sensor_evt_t> lidar_evts;
    create_all_pir_sensor_evts(pir_evts, true);
    create_all_lidar_sensor_evts(lidar_evts, 100);
    // Test sending data at a reasonable frequency - one every 65 seconds
    for (int i = 0; i < 120; i++)
    {
        for (auto evt : pir_evts)
        {
            test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, true);
        }
        for (auto evt : lidar_evts)
        {
            test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
        }
        simulate_time(65);
    }

    // If any sensors are listed as hyperactive, that's a fail.
    for (auto evt : pir_evts)
    {
        if (mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is hyperactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto evt : lidar_evts)
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
    create_all_pir_sensor_evts(pir_evts, true);
    create_all_lidar_sensor_evts(lidar_evts, 100);
    // Test sending data at a high frequency - one every 55 seconds
    for (int i = 0; i < 120; i++)
    {
        for (auto evt : pir_evts)
        {
            test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, true);
        }
        for (auto evt : lidar_evts)
        {
            test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
        }
        simulate_time(55);
    }

    // If any sensors are listed as not hyperactive, that's a fail.
    for (auto evt : pir_evts)
    {
        if (!mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is not hyperactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto evt : lidar_evts)
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
    // Test sending data at a high frequency - one every 55 seconds
    for (int i = 0; i < 120; i++)
    {
        for (auto evt : pir_evts)
        {
            test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, true);
        }
        for (auto evt : lidar_evts)
        {
            test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
        }
        simulate_time(55);
    }

    /* While not the purpose of this test, if any sensors are listed as not hyperactive, that's also a fail
       and prevents us from running the rest of the test. */
    for (auto evt : pir_evts)
    {
        if (!mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is not hyperactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto evt : lidar_evts)
    {
        if (!mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "LIDAR sensor on node ID " << evt.lidar_data.node_id << " at rotation " << evt.lidar_data.sensor_rotation << " is not hyperactive when it should be.";
            throw std::runtime_error(ss.str());
        }
    }
    // Wait long enough to clear out any hyperactivity flags, then send one more event.
    simulate_time(MINUTES(121));
    for (auto evt : pir_evts)
    {
        test_send_pir_data(evt.pir_data.node_id, evt.pir_data.sensor_rotation, true);
    }
    for (auto evt : lidar_evts)
    {
        test_send_lidar_data(evt.lidar_data.node_id, evt.lidar_data.sensor_rotation, 100);
    }
    simulate_time(MINUTES(1));

    // If any sensors are listed as hyperactive, that's a fail.
    for (auto evt : pir_evts)
    {
        if (mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "PIR sensor on node ID " << evt.pir_data.node_id << " at rotation " << evt.pir_data.sensor_rotation << " is hyperactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }
    for (auto evt : lidar_evts)
    {
        if (mm_sensor_error_is_sensor_hyperactive(&evt))
        {
            std::stringstream ss;
            ss << "LIDAR sensor on node ID " << evt.lidar_data.node_id << " at rotation " << evt.lidar_data.sensor_rotation << " is hyperactive when it shouldn't be.";
            throw std::runtime_error(ss.str());
        }
    }
}

static void create_all_pir_sensor_evts(std::vector<sensor_evt_t>& evts, bool detection)
{
    evts.push_back(create_pir_sensor_evt(1, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(2, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(3, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(4, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(5, SENSOR_ROTATION_0, detection));
    evts.push_back(create_pir_sensor_evt(5, SENSOR_ROTATION_270, detection));
    evts.push_back(create_pir_sensor_evt(6, SENSOR_ROTATION_0, detection));
    evts.push_back(create_pir_sensor_evt(6, SENSOR_ROTATION_270, detection));
    evts.push_back(create_pir_sensor_evt(7, SENSOR_ROTATION_90, detection));
    evts.push_back(create_pir_sensor_evt(8, SENSOR_ROTATION_0, detection));
    evts.push_back(create_pir_sensor_evt(8, SENSOR_ROTATION_270, detection));
    evts.push_back(create_pir_sensor_evt(9, SENSOR_ROTATION_90, detection));
}

static void create_all_lidar_sensor_evts(std::vector<sensor_evt_t>& evts, uint16_t distance)
{
    evts.push_back(create_lidar_sensor_evt(1, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(2, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(3, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(4, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(7, SENSOR_ROTATION_0, distance));
    evts.push_back(create_lidar_sensor_evt(9, SENSOR_ROTATION_0, distance));
}

/* Helper function for making events so we can call is_sensor_hyperactive */
static sensor_evt_t create_lidar_sensor_evt
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    uint16_t distance_measured
)
{
    //Create a sensor_event_t with SENSOR_TYPE_LIDAR
    sensor_evt_t sensor_evt;
    memset(&sensor_evt, 0, sizeof(sensor_evt));

    sensor_evt.sensor_type = SENSOR_TYPE_LIDAR;
    sensor_evt.lidar_data.node_id = node_id;
    sensor_evt.lidar_data.sensor_rotation = sensor_rotation;
    sensor_evt.lidar_data.distance_measured = distance_measured;

    return sensor_evt;
}

/* Helper function for making events so we can call is_sensor_hyperactive */
static sensor_evt_t create_pir_sensor_evt
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    bool detection
)
{
    // Create a sensor_event_t with SENSOR_TYPE_PIR
    sensor_evt_t sensor_evt;
    memset(&sensor_evt, 0, sizeof(sensor_evt));

    sensor_evt.sensor_type = SENSOR_TYPE_PIR;
    sensor_evt.pir_data.node_id = node_id;
    sensor_evt.pir_data.sensor_rotation = sensor_rotation;
    sensor_evt.pir_data.detection = detection;

    return sensor_evt;
}