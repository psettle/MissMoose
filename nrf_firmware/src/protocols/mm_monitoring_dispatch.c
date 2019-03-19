/**
file: mm_monitoring_dispatch.c
brief: Interface functions for sending ANT page information to the monitoring application.
notes:
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"
#include "app_scheduler.h"

#include "mm_ant_control.h"
#include "mm_ant_page_manager.h"
#include "mm_monitoring_dispatch.h"
#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define SENSOR_STATE_QUEUE_SIZE                  ( MAX_SENSOR_COUNT )
#define MONITORING_ACKNOWLEDGEMENT_PAGE_NUM     ( 0x20 )
#define MONITORING_LIDAR_DATA_PAGE_NUM          ( 0x21 )
#define MONITORING_PIR_DATA_PAGE_NUM            ( 0x22 )
#define CONCURRENT_PAGE_COUNT                   ( 2 )

#define PAGE_NUM_INDEX                          ( 0 )
#define MESSAGE_ID_INDEX                        ( 1 )
#define NODE_ID_INDEX                           ( 2 )
#define SENSOR_ROTATION_INDEX                   ( 4 )
#define LIDAR_DISTANCE_MEASURED_INDEX           ( 5 )
#define LIDAR_REGION_DETECTED_INDEX             ( 7 )
#define PIR_DETECTION_INDEX                     ( 5 )

#define ACKED_PAGE_NUM_INDEX                    ( 2 )

/**********************************************************
                        MACROS
**********************************************************/

#define QUEUE_EMPTY()   ( state_queue.count == 0 )
#define QUEUE_FULL()    ( state_queue.count == SENSOR_STATE_QUEUE_SIZE )

/**********************************************************
                       TYPES
**********************************************************/

typedef struct {
    bool                valid;
    uint16_t            node_id;
    sensor_type_t       sensor_type;
    sensor_rotation_t   sensor_rotation;
    mm_ant_payload_t    message;
} sensor_state_t;

typedef struct {
    sensor_state_t queue[SENSOR_STATE_QUEUE_SIZE];
    uint16_t         count;
    uint16_t         current;
} sensor_state_queue_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Interrupt callback for ant events, kicks to on_monitoring_data_acknowledge
 */
static void process_ant_evt(ant_evt_t * evt);

/**
 * Main callback for ant events, kicked to by process_ant_evt
 */
static void on_monitoring_data_acknowledge(void* evt_data, uint16_t evt_size);

/**
 * Pop the payload queue, then broadcast the current (if the queue is not empty)
 */
static void broadcast_new_page(void);

/**
 * Pop the payload queue
 */
static void sensor_state_queue_pop(void);

/**
 * Push to the payload queue.
 * 
 * If queue was empty, start a new broadcast.
 * If queue was full, broadcast_new_page() before pushing.
 */
static void sensor_state_queue_push(sensor_state_t const * sensor_state);

/**
 * Find a queue slot that matches the provided sensor state (if one exists) or is empty.
 * 
 * Returns the index of the found slot.
 */
static uint8_t find_queue_slot(sensor_state_t const * sensor_state);

/**
 * Look at the current broadcast of the queue.
 */
static sensor_state_t* sensor_state_queue_current(void);

/**
 * Returns the array address of the next queue element after index.
 */
static uint16_t queue_next(uint16_t index);

/**
 * Returns the page id of the current broadcast.
 */
static uint8_t current_page_id(void);

/**
 * Returns the message id of the current broadcast.
 */
static uint8_t current_msg_id(void);

/**********************************************************
                       VARIABLES
**********************************************************/

static sensor_state_queue_t state_queue;
static uint8_t             message_id = 0;

/**********************************************************
                       DEFINITIONS
**********************************************************/
void mm_monitoring_dispatch_init(void)
{
    /* Init queue */
    memset(&state_queue, 0, sizeof(state_queue));

    /* Register to receive ANT events */
    mm_ant_evt_handler_set(process_ant_evt);
}

void mm_monitoring_dispatch_send_lidar_data
    (
    uint16_t node_id, 
    sensor_rotation_t sensor_rotation, 
    uint16_t distance_measured,
    lidar_region_t region
    )
{
    /* Encode payload */
    sensor_state_t      sensor_state;
    memset(&sensor_state, 0, sizeof(sensor_state));

    sensor_state.valid = true;
    sensor_state.sensor_type = SENSOR_TYPE_LIDAR;
    sensor_state.sensor_rotation = sensor_rotation;
    sensor_state.node_id = node_id;
 
    uint8_t* payload = &sensor_state.message.data[0];

    payload[PAGE_NUM_INDEX] = MONITORING_LIDAR_DATA_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    memcpy(&payload[NODE_ID_INDEX], &node_id, sizeof(uint16_t));
    payload[SENSOR_ROTATION_INDEX] = sensor_rotation;
    memcpy(&payload[LIDAR_DISTANCE_MEASURED_INDEX], &distance_measured, sizeof(uint16_t));
    payload[LIDAR_REGION_DETECTED_INDEX] = region;
    message_id++;
    /* Push to message queue. */
    sensor_state_queue_push(&sensor_state);
}

void mm_monitoring_dispatch_send_pir_data
    (
    uint16_t node_id, 
    sensor_rotation_t sensor_rotation, 
    bool detection
    )
{
    /* Encode payload */
    sensor_state_t      sensor_state;
    memset(&sensor_state, 0, sizeof(sensor_state));

    sensor_state.valid = true;
    sensor_state.sensor_type = SENSOR_TYPE_PIR;
    sensor_state.sensor_rotation = sensor_rotation;
    sensor_state.node_id = node_id;

    uint8_t* payload = &sensor_state.message.data[0];

    payload[PAGE_NUM_INDEX] = MONITORING_PIR_DATA_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    memcpy(&payload[NODE_ID_INDEX], &node_id, sizeof(uint16_t));
    payload[SENSOR_ROTATION_INDEX] = sensor_rotation;
    payload[PIR_DETECTION_INDEX] = detection ? 1 : 0;
    message_id++;
    /* Push to message queue. */
    sensor_state_queue_push(&sensor_state);
}

static void sensor_state_queue_pop(void)
{
    if(QUEUE_EMPTY() || !sensor_state_queue_current()->valid)
    {
        /* (Popping an empty queue isn't allowed, it is protected against here
            by checking !QUEUE_EMPTY() or QUEUE_FULL() as guards before calling.) */
        APP_ERROR_CHECK(true);
    }

    sensor_state_queue_current()->valid = false;
    state_queue.count--;

    if(!QUEUE_EMPTY())
    {
        state_queue.current = queue_next(state_queue.current);
    }
}

static void sensor_state_queue_push(sensor_state_t const * sensor_state)
{
    APP_ERROR_CHECK(sensor_state == NULL);

    bool queue_empty = QUEUE_EMPTY();

    uint8_t index = find_queue_slot(sensor_state);
    sensor_state_t* dest = &state_queue.queue[index];

    /* If the destination is not valid, it means we are adding to the queue. */
    if(!dest->valid)
    {
        state_queue.count++;
    }

    /* Write the page into the next available slot. */
    memcpy(dest, sensor_state, sizeof(sensor_state_t));

    /* If the queue was empty previously, start a new broadcast. */
    if(queue_empty)
    {
        /* Broadcast a new message */
        mm_ant_page_manager_add_page
            (
            current_page_id(),
            &sensor_state_queue_current()->message,
            CONCURRENT_PAGE_COUNT
            );
    }
    /* Otherwise if the current broadcast was updated, refresh it.
       (This will overwrite the previous message id, which will prevent an ack from
        being misinterpreted) */
    else if(index == state_queue.current)
    {
        mm_ant_page_manager_remove_all_pages
            (
            current_page_id()
            );
        mm_ant_page_manager_add_page
            (
            current_page_id(),
            &sensor_state_queue_current()->message,
            CONCURRENT_PAGE_COUNT
            );
    }
}

static sensor_state_t* sensor_state_queue_current(void)
{
    if(QUEUE_EMPTY())
    {
        return NULL;
    }

    return &state_queue.queue[state_queue.current];
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
                if (p_message->ANT_MESSAGE_aucPayload[PAGE_NUM_INDEX] == MONITORING_ACKNOWLEDGEMENT_PAGE_NUM)
                {
                    /* This might acknowledge the current broadcast, kick it to main to check. */
                    uint32_t err_code;
                    err_code = app_sched_event_put(evt, sizeof(ant_evt_t), on_monitoring_data_acknowledge);
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}

static void on_monitoring_data_acknowledge(void* evt_data, uint16_t evt_size)
{
    ant_evt_t const * evt = (ant_evt_t const *)evt_data;
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;
    uint8_t const * payload = &p_message->ANT_MESSAGE_aucPayload[0];

    if(QUEUE_EMPTY())
    {
        /* Nothing to acknowledge,
         likely received an extra ack, or ack for a different message */
        return;
    }
    //If the ack is for the message type that was being broadcast, then it's an ack for the right message.
    if(payload[ACKED_PAGE_NUM_INDEX] == current_page_id())
    {
        if(payload[MESSAGE_ID_INDEX] == current_msg_id())
        {
            /* If the message is acknowledging the current broadcast, mark it as invalid and switch to the next page. */
            broadcast_new_page();
        }
    }
}

static void broadcast_new_page(void)
{
    /* Remove the page that is being dropped. */
    mm_ant_page_manager_remove_all_pages(current_page_id());

    /* Pop the page that was being broadcast */
    sensor_state_queue_pop();

    /* Add the new page to be broadcast */
    if(!QUEUE_EMPTY())
    {
        mm_ant_page_manager_add_page
            (
            current_page_id(),
            &sensor_state_queue_current()->message,
            CONCURRENT_PAGE_COUNT
            );
    }
}

static uint16_t queue_next(uint16_t index)
{
    /* We assume an element exists, so the queue can't be empty. */
    APP_ERROR_CHECK(QUEUE_EMPTY());

    /* Find the next valid index. */
    do {
        index++;
        index %= SENSOR_STATE_QUEUE_SIZE;
    } while(!state_queue.queue[index].valid);
    
    return index;
}

static uint8_t current_page_id(void)
{
    return sensor_state_queue_current()->message.data[PAGE_NUM_INDEX];
}

static uint8_t current_msg_id(void)
{
    return sensor_state_queue_current()->message.data[MESSAGE_ID_INDEX];
}

static uint8_t find_queue_slot(sensor_state_t const * sensor_state)
{
    /* See if there is already a queue slot for the provided state: */
    for(uint8_t i = 0; i < SENSOR_STATE_QUEUE_SIZE; ++i)
    {
        sensor_state_t const * element = &state_queue.queue[i];

        if(!element->valid)
        {
            continue;
        }

        if(element->node_id != sensor_state->node_id)
        {
            continue;
        }

        if(element->sensor_type != sensor_state->sensor_type)
        {
            continue;
        }

        if(element->sensor_rotation != sensor_state->sensor_rotation)
        {
            continue;
        }

        return i;
    }

    /* Find an invalid slot, since there isn't one allocated. */
    for(uint8_t i = 0; i < SENSOR_STATE_QUEUE_SIZE; ++i)
    {
        sensor_state_t const * element = &state_queue.queue[i];

        if(!element->valid)
        {
            /* If the queue is empty, current must be reset to the provided index. */
            if(QUEUE_EMPTY())
            {
                state_queue.current = i;
            }
            return i;
        }
    }

    /* There should either be an existing slot, or an empty slot: */
    APP_ERROR_CHECK(true);
    return 0;
}
