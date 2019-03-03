extern "C" {
    #include "mm_monitoring_dispatch.h"
}

void mm_monitoring_dispatch_init(void) {}

void mm_monitoring_dispatch_send_lidar_data
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    uint16_t distance_measured,
    lidar_region_t region
) {}

void mm_monitoring_dispatch_send_pir_data
(
    uint16_t node_id,
    sensor_rotation_t sensor_rotation,
    bool detection
) {}