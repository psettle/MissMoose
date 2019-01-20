/*
 * sensor_transmission.h
 *
 *  Created on: Jan 16, 2019
 *      Author: nataliewong
 */
#ifndef MM_SENSOR_TRANSMISSION_H
#define MM_SENSOR_TRANSMISSION_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "nrf_soc.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          ENUMS
**********************************************************/

typedef enum
{
    SENSOR_ROTATION_0,
    SENSOR_ROTATION_45,
    SENSOR_ROTATION_90,
    SENSOR_ROTATION_135,
    SENSOR_ROTATION_180,
    SENSOR_ROTATION_225,
    SENSOR_ROTATION_270,
    SENSOR_ROTATION_315
} sensor_rotation_t;

typedef enum
{
	SENSOR_TYPE_UNKNOWN,
	SENSOR_TYPE_PIR,
	SENSOR_TYPE_LIDAR,
	SENSOR_TYPE_COUNT
} sensor_type_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

typedef struct
{
	uint16_t node_id;
	sensor_rotation_t sensor_rotation;
	bool detection;
} pir_evt_data_t;

typedef struct
{
	uint16_t node_id;
	sensor_rotation_t sensor_rotation;
	uint16_t distance_measured;
} lidar_evt_data_t;

typedef struct
{
    sensor_type_t sensor_type;
    union
    {
    	pir_evt_data_t 		pir_data;
    	lidar_evt_data_t 	lidar_data;
    };
} sensor_evt_t;

/* Sensor data event callback */
typedef void (*sensor_data_evt_handler_t) ( sensor_evt_t const * evt);

/* Begins listening to blaze messages containing sensor data events. */
void mm_sensor_transmission_init( void );

/* Registers a listener to sensor data events. */
void mm_register_sensor_data( sensor_data_evt_handler_t sensor_data_evt_handler );

/* Sends a PIR data event over blaze to the gateway node for processing. */
void mm_send_pir_data
	(
	sensor_rotation_t sensor_rotation,
	bool detection
	);

/* Sends a LIDAR data event over blaze to the gateway node for processing. */
void mm_send_lidar_data
	(
	sensor_rotation_t sensor_rotation,
	uint16_t distance_measured
	);

#endif /* MM_SENSOR_TRANSMISSION_H */
