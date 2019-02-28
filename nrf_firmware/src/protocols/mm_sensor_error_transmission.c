/**
file: mm_sensor_error_transmission.c
brief: Methods for transmitting sensor error data to the monitoring application over ANT
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"
#include "mm_ant_control.h"
#include "mm_ant_page_manager.h"
#include "app_scheduler.h"
#include "app_timer.h"

#include "mm_sensor_error_transmission.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_NUM_SENSORS                         ( 16 )
#define MESSAGE_ACKNOWLEDGEMENT_PAGE_NUM        ( 0x20 )
#define HYPERACTIVITY_ERROR_STATUS_PAGE_NUM     ( 0x25 )
#define INACTIVE_SENSOR_ERROR_STATUS_PAGE_NUM   ( 0x26 )
#define CONCURRENT_PAGE_COUNT                   ( 1 ) 

#define PAGE_NUM_INDEX                          ( 0 )
#define MESSAGE_ID_INDEX                        ( 1 )
#define NODE_ID_INDEX                           ( 2 )
#define SENSOR_TYPE_INDEX                       ( 4 )
#define SENSOR_ROTATION_INDEX                   ( 5 )
#define ERROR_OCCURING_INDEX                    ( 6 )
#define INDEX_7_UNUSED                          ( 7 )

#define UNUSED_PAYLOAD_BYTE_VALUE               ( 0xFF )

#define ACKED_PAGE_NUM_INDEX                    ( 2 )

/**********************************************************
                          ENUMS
**********************************************************/

//The ENUM values for error types match the error page values
typedef enum
{
    INVALID,         // No error data
    HYPERACTIVE_ERROR,   // Hyperactive error to broadcast
    INACTIVE_ERROR       // Inactive error to broadcast
} error_payload_state_t;

/**********************************************************
                       TYPES
**********************************************************/

typedef struct 
{
    mm_ant_payload_t payload;
    uint16_t node_id;
    sensor_type_t sensor_type;
    sensor_rotation_t sensor_rotation;
    error_payload_state_t payload_state;
} error_payload_t;

// A queue to hold sensor error information. 
typedef struct 
{
    error_payload_t  queue[MAX_NUM_SENSORS]; //Queue size - Max number of sensors.
    uint16_t         last_broadcast_index; //The element of the array that was last broadcast
} error_transmission_queue_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Adds a page to the error_transmission_queue.
 */
static void add_page_to_queue(error_payload_t const *error_payload);

/**
 * Helper - checks that the node_id, sensor_type, and sensor_rotation match inside of an error_payload
 */
static bool check_sensor_match(error_payload_t const *error_payload1, error_payload_t const *error_payload2);

/**
 * Broadcasts the next element of the error_transmission_queue that needs to be broadcast
 */
static void broadcast_next_page(void);

/**
 * Checks if a message ID matches the currently broadcasting message
 */
static bool id_matches_current_broadcast(uint8_t message_id);

/**
 * Interrupt callback for ant events, kicks to on_message_acknowledge
 */
static void process_ant_evt(ant_evt_t * evt);

/**
 * Handles a broadcast acknowledgement. If it is an ack for an error_page_broadcast, set the message
 * as sent and update the broadcast to the next avilable message.
 */
static void on_message_acknowledge(void* evt_data, uint16_t evt_size);


/**********************************************************
                       VARIABLES
**********************************************************/

static error_transmission_queue_t error_trans_queue;
static uint8_t message_id = 0;
// The page of the error message currently being broadcast.
static uint8_t broadcast_error_message_page_num = 0;
static bool message_being_broadcast = false;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Initialize sensor error state transmission over ANT.
*/
void mm_sensor_error_transmission_init(void)
{
    // Initialize error_trans_queue. This sets all elements of the struct to 0,
    // including flags inside of the queue element. Also sets last_broadcast_index to 0, the first element
    // of the struct
    memset(&error_trans_queue, 0, sizeof(error_trans_queue));

    // Register to receive ANT events
    mm_ant_evt_handler_set(process_ant_evt);
}

void mm_sensor_error_transmission_send_hyperactivity_update
    (
    uint16_t node_id,
    sensor_type_t sensor_type,
    sensor_rotation_t sensor_rotation,
    bool error_occuring
    )
{
    error_payload_t error_payload;
    error_payload.payload_state = HYPERACTIVE_ERROR;
    error_payload.node_id = node_id;
    error_payload.sensor_type = sensor_type;
    error_payload.sensor_rotation = sensor_rotation;

    uint8_t *payload = &(error_payload.payload.data[0]);

    memset(&(error_payload), 0, sizeof(error_payload_t));

    payload[PAGE_NUM_INDEX] = HYPERACTIVITY_ERROR_STATUS_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    memcpy(&payload[NODE_ID_INDEX], &node_id, sizeof(uint16_t));
    payload[SENSOR_TYPE_INDEX] = sensor_type;
    payload[SENSOR_ROTATION_INDEX] = sensor_rotation;
    payload[ERROR_OCCURING_INDEX] = error_occuring;
    payload[INDEX_7_UNUSED] = UNUSED_PAYLOAD_BYTE_VALUE;

    message_id++;

    //Update pages to be broadcast.
    add_page_to_queue(&error_payload);
}

void mm_sensor_error_transmission_send_inactivity_update
    (
    uint16_t node_id,
    sensor_type_t sensor_type,
    sensor_rotation_t sensor_rotation,
    bool error_occuring
    )
{
    error_payload_t error_payload;
    error_payload.payload_state = INACTIVE_ERROR;
    error_payload.node_id = node_id;
    error_payload.sensor_type = sensor_type;
    error_payload.sensor_rotation = sensor_rotation;

    uint8_t *payload = &(error_payload.payload.data[0]);

    memset(&(error_payload), 0, sizeof(error_payload_t));

    payload[PAGE_NUM_INDEX] = INACTIVE_SENSOR_ERROR_STATUS_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    memcpy(&payload[NODE_ID_INDEX], &node_id, sizeof(uint16_t));
    payload[SENSOR_TYPE_INDEX] = sensor_type;
    payload[SENSOR_ROTATION_INDEX] = sensor_rotation;
    payload[ERROR_OCCURING_INDEX] = error_occuring;
    payload[INDEX_7_UNUSED] = UNUSED_PAYLOAD_BYTE_VALUE;

    message_id++;

    //Update pages to be broadcast.
    add_page_to_queue(&error_payload);
}

static void add_page_to_queue(error_payload_t const *error_payload)
{
    //Make sure the sensor isn't already in the queue. Need to check node_id, sensor_type, and sensor_rotation
    for(uint8_t i = 0; i < MAX_NUM_SENSORS; i++)
    {
        if(check_sensor_match(error_payload, &(error_trans_queue.queue[i]) ))
        {
            // The sensor already exists in the queue. Update this error payload. 
            memcpy( &(error_trans_queue.queue[i]), error_payload, sizeof(error_payload_t));
            if(!message_being_broadcast)
            {
                broadcast_next_page();
            }
            return; //Queue has been updated, return
        }

    }
    //Sensor not found in queue. Find an invalid loction to save the sensor data.
    for(uint8_t i = 0; i < MAX_NUM_SENSORS; i++)
    {
        if(error_trans_queue.queue[i].payload_state == INVALID) //If invalid, store payload here
        {
            memcpy( &(error_trans_queue.queue[i]), &error_payload, sizeof(error_payload_t));
            if(!message_being_broadcast)
            {
                broadcast_next_page();
            }
            return; //Queue has been updated, return
        }
    }

    //Unable to find a location for the payload, error
    APP_ERROR_CHECK(true);
}

static bool check_sensor_match(error_payload_t const *error_payload1, error_payload_t const *error_payload2)
{
    if
        (
            (error_payload1->node_id == error_payload2->node_id) &&
            (error_payload1->sensor_type == error_payload2->sensor_type) &&
            (error_payload1->sensor_rotation == error_payload2->sensor_rotation)
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void broadcast_next_page(void)
{    
    // Check the next queue index after the current broadcast index. If the payload is not invalid, 
    // broadcast it. Otherwise, loop until you find the first valid payload, 
    // or until you reach the last_broadcast_index again!
    uint8_t queue_index = error_trans_queue.last_broadcast_index + 1;
    // Wrap queue index if necessary
    if(queue_index == MAX_NUM_SENSORS) 
    {
        queue_index = 0;
    }
    do 
    {
        if(error_trans_queue.queue[queue_index].payload_state != INVALID)
        {
            // Broadcast found!
            // Remove pages of the last broadcast error type.
            error_payload_state_t last_error_type = error_trans_queue.queue[error_trans_queue.last_broadcast_index].payload_state;
            switch(last_error_type)
            {
                case HYPERACTIVE_ERROR:
                    mm_ant_page_manager_remove_all_pages(HYPERACTIVITY_ERROR_STATUS_PAGE_NUM);
                    break;
                case INACTIVE_ERROR:
                    mm_ant_page_manager_remove_all_pages(INACTIVE_SENSOR_ERROR_STATUS_PAGE_NUM);
                    break;
                default:
                    // State unknown, this should not occur. 
                    APP_ERROR_CHECK(true);
            }

            // Add new broadcast. Figure out the type of error.
            if(error_trans_queue.queue[queue_index].payload_state == HYPERACTIVE_ERROR)
            {
                broadcast_error_message_page_num = HYPERACTIVITY_ERROR_STATUS_PAGE_NUM;
                mm_ant_page_manager_add_page
                    (
                        HYPERACTIVITY_ERROR_STATUS_PAGE_NUM,
                        &(error_trans_queue.queue[queue_index].payload),
                        CONCURRENT_PAGE_COUNT
                    );
            }
            else if(error_trans_queue.queue[queue_index].payload_state == INACTIVE_ERROR)
            {
                    broadcast_error_message_page_num = INACTIVE_SENSOR_ERROR_STATUS_PAGE_NUM;
                    mm_ant_page_manager_add_page
                    (
                        INACTIVE_SENSOR_ERROR_STATUS_PAGE_NUM,
                        &(error_trans_queue.queue[queue_index].payload),
                        CONCURRENT_PAGE_COUNT
                    );
            }
            //Update the queue index and message_being_broadcast variable
            error_trans_queue.last_broadcast_index = queue_index;
            message_being_broadcast = true;
            return;
        }

        // Increment the index. If past the last queue index, go back to index 0.
        queue_index++;
        if(queue_index == MAX_NUM_SENSORS) 
        {
            queue_index = 0;
        }
    }
    while(queue_index != error_trans_queue.last_broadcast_index + 1);

    // Have exited loop without finding a page to broadcast.
    message_being_broadcast = false;
}

static bool id_matches_current_broadcast(uint8_t message_id)
{
    //Check if the message ID matches the broadcast currently being sent
    error_payload_t const *broadcasting_payload = &(error_trans_queue.queue[error_trans_queue.last_broadcast_index]);
    if
        (
            (broadcasting_payload->payload_state != INVALID) &&
            (broadcasting_payload->payload.data[MESSAGE_ID_INDEX] == message_id)
        )
    {
        return true;
    }
    return false;
}

static void process_ant_evt(ant_evt_t * evt)
{
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;

    switch (evt->event)
    {
        // If this is a "received message" event, take a closer look
        case EVENT_RX:
            if( (p_message->ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID) ||
                (p_message->ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID) )
            {
                if (p_message->ANT_MESSAGE_aucPayload[PAGE_NUM_INDEX] == MESSAGE_ACKNOWLEDGEMENT_PAGE_NUM)
                {
                    /* This might acknowledge the current broadcast, kick it to main to check. */
                    uint32_t err_code;
                	err_code = app_sched_event_put(evt, sizeof(ant_evt_t), on_message_acknowledge);
                	APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}

static void on_message_acknowledge(void* evt_data, uint16_t evt_size)
{
    ant_evt_t const * evt = (ant_evt_t const *)evt_data;
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;
    uint8_t const * payload = &p_message->ANT_MESSAGE_aucPayload[0];

    if(payload[ACKED_PAGE_NUM_INDEX] == broadcast_error_message_page_num)
    {
        // Check the message ID. Should match the message being broadcast. If it does, start next broadcast.
        if(id_matches_current_broadcast(payload[MESSAGE_ID_INDEX]))
        {
            //Invalidate the current broadcast, as it has been acknowledged.
            error_trans_queue.queue[error_trans_queue.last_broadcast_index].payload_state = INVALID;
            broadcast_next_page();
        }
    }
}