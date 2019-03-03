/**
file: mm_sensor_transmission.c
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"
#include "app_scheduler.h"

#include "mm_sensor_transmission.h"
#include "mm_blaze_control.h"
#include "mm_blaze_static_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_NUMBER_LISTENERS             ( 4 )
#define BLAZE_LIDAR_DATA_PAGE_NUMBER     ( 0x01 )
#define BLAZE_PIR_DATA_PAGE_NUMBER       ( 0x02 )

/**********************************************************
                        VARIABLES
**********************************************************/

#ifdef MM_BLAZE_GATEWAY
static sensor_data_evt_handler_t listeners[MAX_NUMBER_LISTENERS];
#endif

/**********************************************************
                       DECLARATIONS
**********************************************************/

#ifdef MM_BLAZE_GATEWAY
static void blaze_rx_handler(ant_blaze_message_t msg);
static void on_rx_sensor_data(void* p_data, uint16_t size);
static void sensor_data_evt_message_dispatch(sensor_evt_t const * sensor_evt);
#endif

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_sensor_transmission_init()
{
#ifdef MM_BLAZE_GATEWAY
    mm_blaze_register_message_listener(blaze_rx_handler);
    memset(&listeners[0], 0, sizeof(listeners));
#endif
}

#ifdef MM_BLAZE_GATEWAY
void mm_sensor_transmission_register_sensor_data( sensor_data_evt_handler_t sensor_data_evt_handler )
{
    for(uint16_t i = 0; i < MAX_NUMBER_LISTENERS; i++)
    {
        if( listeners[i] == NULL)
        {
            listeners[i] = sensor_data_evt_handler;
            return;
        }
    }

    APP_ERROR_CHECK(true);
}
#endif

void mm_send_pir_data(sensor_rotation_t sensor_rotation, bool detection)
{
#ifdef MM_BLAZE_GATEWAY
    // Gateway nodes don't broadcast their own sensor data events
    // over blaze: they simply notify listeners...
    sensor_evt_t sensor_evt;
    memset(&sensor_evt, 0, sizeof(sensor_evt));

    sensor_evt.sensor_type = SENSOR_TYPE_PIR;
    sensor_evt.pir_data.node_id = MM_GATEWAY_ID;
    sensor_evt.pir_data.sensor_rotation = sensor_rotation;
    sensor_evt.pir_data.detection = detection;

    sensor_data_evt_message_dispatch(&sensor_evt);
#else
    // Non-gateway nodes encode their sensor data events to
    // broadcast to the gateway node over blaze...
    ant_blaze_message_t blaze_message;
    memset(&blaze_message, 0, sizeof(blaze_message));

    blaze_message.address = MM_GATEWAY_ID;
    blaze_message.index = 0;
    blaze_message.length = 5;

    uint8_t payload [5];
    memset(&payload[0], 0, sizeof(payload));

    payload[0] = BLAZE_PIR_DATA_PAGE_NUMBER;
    payload[1] = sensor_rotation;
    payload[2] = detection ? 1 : 0;

    memset( &payload[3], 0xFF, 2);

    blaze_message.p_data = &payload[0];

    mm_blaze_send_message(&blaze_message);
#endif
}

void mm_send_lidar_data(sensor_rotation_t sensor_rotation, uint16_t distance_measured)
{
#ifdef MM_BLAZE_GATEWAY
    // Gateway nodes don't broadcast their own sensor data events
    // over blaze: they simply notify listeners...
    sensor_evt_t sensor_evt;
    memset(&sensor_evt, 0, sizeof(sensor_evt));

    sensor_evt.sensor_type = SENSOR_TYPE_LIDAR;
    sensor_evt.lidar_data.node_id = MM_GATEWAY_ID;
    sensor_evt.lidar_data.sensor_rotation = sensor_rotation;
    sensor_evt.lidar_data.distance_measured = distance_measured;

    sensor_data_evt_message_dispatch(&sensor_evt);
#else
    // Non-gateway nodes encode their sensor data events to
    // broadcast to the gateway node over blaze...
    ant_blaze_message_t blaze_message;
    memset(&blaze_message, 0, sizeof(blaze_message));

    blaze_message.address = MM_GATEWAY_ID;
    blaze_message.index = 0;
    blaze_message.length = 5;

    uint8_t payload [5];
    memset(&payload[0], 0, sizeof(payload));

    payload[0] = BLAZE_LIDAR_DATA_PAGE_NUMBER;
    payload[1] = sensor_rotation;
    memcpy(&payload[2], &distance_measured, sizeof(distance_measured));

    memset(&payload[4], 0xFF, 1);

    blaze_message.p_data = &payload[0];

    mm_blaze_send_message(&blaze_message);
#endif
}

#ifdef MM_BLAZE_GATEWAY
/* The gateway node processes incoming sensor data events from
 * non-gateway nodes and notifies listeners.*/
static void blaze_rx_handler(ant_blaze_message_t msg)
{

    /* Filter message for types we care about. */
    switch(msg.p_data[0])
    {
        case BLAZE_LIDAR_DATA_PAGE_NUMBER:
        case BLAZE_PIR_DATA_PAGE_NUMBER:
            break;
        default:
            return;
    }

    /* Pack message to serializable format. */
    mm_blaze_message_serialized_t evt;
    mm_blaze_pack_message(&msg, &evt);

    /* Kick event to main */
    uint32_t err_code;
    err_code = app_sched_event_put(&evt, sizeof(evt), on_rx_sensor_data);
    APP_ERROR_CHECK(err_code);
}

static void on_rx_sensor_data(void* p_data, uint16_t size)
{

    /* Process packet  */
    mm_blaze_message_serialized_t const * msg = (mm_blaze_message_serialized_t const *)p_data;
    uint8_t const * payload = &msg->data[0];

    sensor_evt_t sensor_evt;
    memset( &sensor_evt, 0, sizeof( sensor_evt ) );

    switch( payload[0] )
    {
        case BLAZE_LIDAR_DATA_PAGE_NUMBER:
            sensor_evt.sensor_type = SENSOR_TYPE_LIDAR;
            sensor_evt.lidar_data.node_id = msg->message.address;
            sensor_evt.lidar_data.sensor_rotation = payload[1];
            memcpy(&sensor_evt.lidar_data.distance_measured, &payload[2], 2);
            break;
        case BLAZE_PIR_DATA_PAGE_NUMBER:
            sensor_evt.sensor_type = SENSOR_TYPE_PIR;
            sensor_evt.pir_data.node_id = msg->message.address;
            sensor_evt.pir_data.sensor_rotation = payload[1];
            sensor_evt.pir_data.detection = payload[2];
            break;
        default:
            /* Message type should have been filtered already. */
            APP_ERROR_CHECK(true);
            return;
    }

    sensor_data_evt_message_dispatch( &sensor_evt );

}

/* Notifies sensor data event listeners of processed sensor
 * data events.*/
static void sensor_data_evt_message_dispatch(sensor_evt_t const * sensor_evt)
{
    for(uint16_t i = 0; i < MAX_NUMBER_LISTENERS; i++)
    {
        if(listeners[i] != NULL)
        {
            listeners[i](sensor_evt);
        }
    }
}
#endif



