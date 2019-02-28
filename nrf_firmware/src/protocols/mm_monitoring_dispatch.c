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
#include "app_timer.h"

#include "mm_ant_control.h"
#include "mm_ant_page_manager.h"
#include "mm_monitoring_dispatch.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define ANT_PAYLOAD_QUEUE_SIZE                  ( 50 )
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

#define TEST_MODE                               ( false )
#define TEST_MODE_PERIOD_MS                     ( 200 )
#define TEST_MODE_TICKS                         ( APP_TIMER_TICKS(TEST_MODE_PERIOD_MS) )

/**********************************************************
                        MACROS
**********************************************************/

#define QUEUE_EMPTY()   ( payload_queue.count == 0 )
#define QUEUE_FULL()    ( payload_queue.count == ANT_PAYLOAD_QUEUE_SIZE )

/**********************************************************
                       TYPES
**********************************************************/

typedef struct {
    mm_ant_payload_t queue[ANT_PAYLOAD_QUEUE_SIZE];
    uint16_t         count;
    uint16_t         head;
    uint16_t         tail;
} ant_payload_queue_t;

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
 * Pop the payload queue, then broadcast the tail (if the queue is not empty)
 */
static void broadcast_new_page(void);

/**
 * Pop the payload queue
 */
static void ant_payload_queue_pop(void);

/**
 * Push to the payload queue.
 * 
 * If queue was empty, start a new broadcast.
 * If queue was full, broadcast_new_page() before pushing.
 */
static void ant_payload_queue_push(mm_ant_payload_t const * ant_payload);

/**
 * Look at the tail of the queue.
 */
static mm_ant_payload_t const * ant_payload_queue_peek_tail(void);

/**
 * Returns the array address of the next queue element after index.
 */
static uint16_t queue_next(uint16_t index);

/**
 * Returns the page id of the tail of the queue.
 */
static uint8_t current_page_id(void);

/**
 * Returns the message id of the tail of the queue.
 */
static uint8_t current_msg_id(void);

#if TEST_MODE
    /**
     * Initialize test feature of this module.
     * 
     * The test feature pushs a random sensor update event to the payload queue
     * every TEST_MODE_PERIOD_MS ms.
     */
    static void test_mode_init(void);

    /**
     * Interrupt callback for the test mode timer.
     */
    static void test_timer_event(void * p_context);

    /**
     * Main callback for the test mode timer.
     */
    static void on_test_timer_event(void* evt_data, uint16_t evt_size);
#endif

/**********************************************************
                       VARIABLES
**********************************************************/

static ant_payload_queue_t payload_queue;
static uint8_t             message_id = 0;

#if TEST_MODE
    APP_TIMER_DEF(test_timer);
#endif

/**********************************************************
                       DEFINITIONS
**********************************************************/
void mm_monitoring_dispatch_init(void)
{
    /* Init queue */
    memset(&payload_queue, 0, sizeof(payload_queue));

    /* Register to receive ANT events */
    mm_ant_evt_handler_set(process_ant_evt);

#if TEST_MODE
    test_mode_init();
#endif
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
    mm_ant_payload_t lidar_page_payload;
    uint8_t* payload = &lidar_page_payload.data[0];

    memset(&lidar_page_payload, 0, sizeof(mm_ant_payload_t));
    
    payload[PAGE_NUM_INDEX] = MONITORING_LIDAR_DATA_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    memcpy(&payload[NODE_ID_INDEX], &node_id, sizeof(uint16_t));
    payload[SENSOR_ROTATION_INDEX] = sensor_rotation;
    memcpy(&payload[LIDAR_DISTANCE_MEASURED_INDEX], &distance_measured, sizeof(uint16_t));
    payload[LIDAR_REGION_DETECTED_INDEX] = region;
    message_id++;

    /* Push to message queue. */
    ant_payload_queue_push(&lidar_page_payload);
}

void mm_monitoring_dispatch_send_pir_data
    (
    uint16_t node_id, 
    sensor_rotation_t sensor_rotation, 
    bool detection
    )
{
    /* Encode payload */
    mm_ant_payload_t pir_page_payload;
    uint8_t* payload = &pir_page_payload.data[0];

    memset(&pir_page_payload, 0x00, sizeof(mm_ant_payload_t));

    payload[PAGE_NUM_INDEX] = MONITORING_PIR_DATA_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    memcpy(&payload[NODE_ID_INDEX], &node_id, sizeof(uint16_t));
    payload[SENSOR_ROTATION_INDEX] = sensor_rotation;
    payload[PIR_DETECTION_INDEX] = detection ? 1 : 0;

    message_id++;

    /* Push to message queue. */
    ant_payload_queue_push(&pir_page_payload);
}

static void ant_payload_queue_pop(void)
{
    if(QUEUE_EMPTY())
    {
        /* (Popping an empty queue isn't allowed, it is protected against here
            by checking !QUEUE_EMPTY() or QUEUE_FULL() as guards before calling.) */
        APP_ERROR_CHECK(true);
    }

    payload_queue.tail = queue_next(payload_queue.tail);
    payload_queue.count--;
}

static void ant_payload_queue_push(mm_ant_payload_t const * ant_payload)
{
    APP_ERROR_CHECK(ant_payload == NULL);

    /* If the queue is full, pop a page to make room. */
    if(QUEUE_FULL())
    {
        broadcast_new_page();
    }

    /* Write the page into the next available slot. */
    memcpy(&payload_queue.queue[payload_queue.head], ant_payload, sizeof(mm_ant_payload_t));
    payload_queue.head = queue_next(payload_queue.head);
    payload_queue.count++;

    /* If the queue was empty previously, start a new broadcast. */
    if(payload_queue.count == 1)
    {
        /* Broadcast a new message */
        mm_ant_page_manager_add_page
            (
            current_page_id(),
            ant_payload_queue_peek_tail(),
            CONCURRENT_PAGE_COUNT
            );
    }
    
}

static mm_ant_payload_t const * ant_payload_queue_peek_tail(void)
{
    if(QUEUE_EMPTY())
    {
        return NULL;
    }

    return &payload_queue.queue[payload_queue.tail];

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

    //If the ack is for the message type that was being broadcast, then it's an ack for the right message.
    if(payload[ACKED_PAGE_NUM_INDEX] == current_page_id())
    {
        if(QUEUE_EMPTY())
        {
            /* Nothing to acknowledge, likely
            received an extra ack. */
            return;
        }

        if(current_msg_id() == payload[MESSAGE_ID_INDEX])
        {
            /* If the message is acknowledging the current broadcast, switch to the next page. */
            broadcast_new_page();
        }
    }
}

static void broadcast_new_page(void)
{
    /* Remove the page that is being dropped. */
    mm_ant_page_manager_remove_all_pages(current_page_id());

    /* Pop the page that was being broadcast */
    ant_payload_queue_pop();

    /* Add the new page to be broadcast */
    if(!QUEUE_EMPTY())
    {
        mm_ant_page_manager_add_page
            (
            current_page_id(),
            ant_payload_queue_peek_tail(),
            CONCURRENT_PAGE_COUNT
            );
    }
}

static uint16_t queue_next(uint16_t index)
{
    /* Return the next index after index in the queue. */
    index++;
    index %= ANT_PAYLOAD_QUEUE_SIZE;
    return index;
}

static uint8_t current_page_id(void)
{
    mm_ant_payload_t const * tail = ant_payload_queue_peek_tail();
    return tail->data[PAGE_NUM_INDEX];
}

static uint8_t current_msg_id(void)
{
    mm_ant_payload_t const * tail = ant_payload_queue_peek_tail();
    return tail->data[MESSAGE_ID_INDEX];
}

#if TEST_MODE
    static void test_mode_init(void)
    {
        uint32_t err_code;

        /* Launch a timer to fill up the queue: */
        err_code = app_timer_create(&test_timer, APP_TIMER_MODE_REPEATED, test_timer_event);
        APP_ERROR_CHECK(err_code);

        err_code = app_timer_start(test_timer, TEST_MODE_TICKS, NULL);
        APP_ERROR_CHECK(err_code);
    }

    static void test_timer_event(void * p_context)
    {
        /* Kick event to main. */
        uint32_t err_code;
        err_code = app_sched_event_put(NULL, 0, on_test_timer_event);
        APP_ERROR_CHECK(err_code);
    }

    static void on_test_timer_event(void* evt_data, uint16_t evt_size)
    {
        /* Send a monitoring event. */
        switch(message_id & 0x01)
        {
        case 0:
            mm_monitoring_dispatch_send_pir_data
                (
                message_id,
                message_id % SENSOR_ROTATION_COUNT,
                message_id & 0x02  
                );
            break;
        case 1:
            mm_monitoring_dispatch_send_lidar_data
                (
                message_id,
                message_id % SENSOR_ROTATION_COUNT,
                258 /* 1 in msb, 2 in lsb */  
                );
            break;
        }
    }
#endif