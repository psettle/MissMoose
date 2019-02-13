/**
file: mm_sensor_transmission.cpp
brief: Mocked functions to simulate sensors detecting information for the test framework
notes:

Author: Elijah Pennoyer
*/

/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_sensor_transmission.hpp"
#include <vector>
#include <string.h>

/**********************************************************
                        VARIABLES
**********************************************************/
static std::vector<sensor_data_evt_handler_t> listeners;

/**********************************************************
                       DECLARATIONS
**********************************************************/
static void sensor_data_evt_message_dispatch(sensor_evt_t const * sensor_evt);

/**********************************************************
                       DEFINITIONS
**********************************************************/


// Initializes the sensor transmission functions
void mm_sensor_transmission_init(void) 
{
    // Clear the list of listeners - Otherwise, new listeners will be added for every test called
    // when initialization occurs.
    listeners.clear();
}

/* Registers a listener for sensor data events. 
   The test_send_pir_data and test_send_lidar_data functions will send data to these handlers. */
void mm_sensor_transmission_register_sensor_data(sensor_data_evt_handler_t sensor_data_evt_handler) 
{
    listeners.push_back(sensor_data_evt_handler);
}

void test_send_pir_data
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

    //Send message to listeners
    sensor_data_evt_message_dispatch(&sensor_evt); 

}

void test_send_lidar_data
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    uint16_t distance_measured
)
{
    //Create a sensor_event_t with SENSOR_TYPE_PIR
    sensor_evt_t sensor_evt;
    memset(&sensor_evt, 0, sizeof(sensor_evt));
    
    sensor_evt.sensor_type = SENSOR_TYPE_LIDAR;
    sensor_evt.lidar_data.node_id = node_id;
    sensor_evt.lidar_data.sensor_rotation = sensor_rotation;
    sensor_evt.lidar_data.distance_measured = distance_measured;

    //Send message to listeners
    sensor_data_evt_message_dispatch(&sensor_evt); 
}


static void sensor_data_evt_message_dispatch(sensor_evt_t const * sensor_evt)
{
    // Send the sensor_event_t to the listeners. Iterates through the vector.
    for(auto listener : listeners)
    {
        listener(sensor_evt);
    }
}