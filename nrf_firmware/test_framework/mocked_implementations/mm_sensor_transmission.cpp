extern "C" {
    #include "mm_sensor_transmission.h"
}

void mm_sensor_transmission_init(void) {}

/* Registers a listener to sensor data events. */
void mm_sensor_transmission_register_sensor_data(sensor_data_evt_handler_t sensor_data_evt_handler) {}

/* Sends a PIR data event to the gateway node for processing. */
void mm_send_pir_data
(
    sensor_rotation_t sensor_rotation,
    bool detection
) {}

/* Sends a LIDAR data event to the gateway node for processing. */
void mm_send_lidar_data
(
    sensor_rotation_t sensor_rotation,
    uint16_t distance_measured
) {}