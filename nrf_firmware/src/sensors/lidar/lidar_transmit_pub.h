/*
 * lidar_transmit_pub.h
 *
 *  Created on: Nov 23, 2018
 *      Author: nataliewong
 */

#ifndef SRC_SENSORS_LIDAR_LIDAR_TRANSMIT_PUB_H_
#define SRC_SENSORS_LIDAR_LIDAR_TRANSMIT_PUB_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function for initializing the lidar transmitter.
 */
void lidar_transmit_init(nrf_drv_gpiote_pin_t ctrl_pin);

/**
 * @brief A call for updating the lidar transmitter.
 *
 */
void lidar_transmit_update(void);
#ifdef __cplusplus
}
#endif

#endif /* SRC_SENSORS_LIDAR_LIDAR_TRANSMIT_PUB_H_ */
