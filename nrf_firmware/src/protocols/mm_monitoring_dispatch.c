/**
file: mm_monitoring_dispatch.c
brief: Interface functions for sending ANT page information to the monitoring application.
notes:
*/

/**********************************************************
                        PROJECT INCLUDES
**********************************************************/

#include "mm_ant_page_manager.h"
#include "mm_ant_control.h"
#include "mm_monitoring_dispatch.h"
#include "app_error.h"
#include <string.h>

/**********************************************************
                        CONSTANTS
**********************************************************/

#define ANT_PAYLOAD_QUEUE_SIZE                  50
#define MONITORING_ACKNOWLEDGEMENT_PAGE_NUM     0x20
#define MONITORING_LIDAR_DATA_PAGE_NUM          0x21
#define MONITORING_PIR_DATA_PAGE_NUM            0x22
#define CONCURRENT_PAGE_COUNT                   2

#define PAGE_NUM_INDEX                          0
#define MESSAGE_ID_INDEX                        1
#define NODE_ID_INDEX                           2
#define SENSOR_ROTATION_INDEX                   4
#define LIDAR_DISTANCE_MEASURED_INDEX           5
#define PIR_DETECTION_INDEX                     5

#define TEST_MODE                               false


/**********************************************************
                       DECLARATIONS
**********************************************************/

typedef enum
{
    IDLE,
    BROADCAST_NEW_PAGE,
} state_t;

static void ant_payload_queue_pop(void);
static void ant_payload_queue_push(mm_ant_payload_t const * ant_payload);
static mm_ant_payload_t* ant_payload_queue_peak_tail(void);
static void process_ant_evt(ant_evt_t * evt);
static void broadcast_new_page(void);

/**********************************************************
                       VARIABLES
**********************************************************/
static mm_ant_payload_t ant_payload_queue[ANT_PAYLOAD_QUEUE_SIZE];
static uint32_t ant_payload_queue_count = 0;
static uint32_t ant_payload_queue_head = 0;
static uint32_t ant_payload_queue_tail = 0;
static state_t state = IDLE;
static uint8_t message_id = 0;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_monitoring_dispatch_main(void)
{
    switch (state)
    {
        case IDLE:
            // Do nothing
#if TEST_MODE
            if(ant_payload_queue_count < ANT_PAYLOAD_QUEUE_SIZE){
                if(message_id & 1){
                    mm_monitoring_dispatch_send_pir_data(0x01, message_id % 8, message_id & 2);
                }
                else{
                    mm_monitoring_dispatch_send_lidar_data(0x02, message_id % 8, 500);
                }
            }
#endif //TEST_MODE
            break;
        case BROADCAST_NEW_PAGE:
            broadcast_new_page();
            state = IDLE;
            break;
        default:
            // Do nothing
            break;
    }
}

void mm_monitoring_dispatch_init(void)
{
    // Register to receive ANT events
    mm_ant_evt_handler_set(process_ant_evt);
}

void mm_monitoring_dispatch_send_lidar_data
    (
    uint16_t node_id, 
    sensor_rotation_t sensor_rotation, 
    uint16_t distance_measured
    )
{
    mm_ant_payload_t lidar_page_payload;
    memset(&lidar_page_payload, 0x00, sizeof(mm_ant_payload_t));
    lidar_page_payload.data[PAGE_NUM_INDEX] = MONITORING_LIDAR_DATA_PAGE_NUM;
    lidar_page_payload.data[MESSAGE_ID_INDEX] = message_id;
    message_id++;

    memcpy(&(lidar_page_payload.data[NODE_ID_INDEX]), &node_id, sizeof(uint16_t));
    lidar_page_payload.data[SENSOR_ROTATION_INDEX] = sensor_rotation;

    memcpy(&(lidar_page_payload.data[LIDAR_DISTANCE_MEASURED_INDEX]), &distance_measured, sizeof(uint16_t));

    ant_payload_queue_push(&lidar_page_payload);
}

void mm_monitoring_dispatch_send_pir_data
    (
    uint16_t node_id, 
    sensor_rotation_t sensor_rotation, 
    bool detection
    )
{
    mm_ant_payload_t pir_page_payload;
    memset(&pir_page_payload, 0x00, sizeof(mm_ant_payload_t));
    pir_page_payload.data[PAGE_NUM_INDEX] = MONITORING_PIR_DATA_PAGE_NUM;
    pir_page_payload.data[MESSAGE_ID_INDEX] = message_id;
    message_id++;

    memcpy(&(pir_page_payload.data[NODE_ID_INDEX]), &node_id, sizeof(uint16_t));
    pir_page_payload.data[SENSOR_ROTATION_INDEX] = sensor_rotation;

    pir_page_payload.data[PIR_DETECTION_INDEX] = detection ? 1 : 0;

    ant_payload_queue_push(&pir_page_payload);
}

static void ant_payload_queue_pop(void)
{
    if(ant_payload_queue_count == 0)
    {
        //Nothing to pop
        return;
    }

    ant_payload_queue_tail++;
    ant_payload_queue_tail %= ANT_PAYLOAD_QUEUE_SIZE;
    ant_payload_queue_count--;
}

static void ant_payload_queue_push(mm_ant_payload_t const * ant_payload)
{
    APP_ERROR_CHECK(ant_payload == NULL);

    //Ensure state is clean before modifying the queue
    if(state == BROADCAST_NEW_PAGE){
        broadcast_new_page();
        state = IDLE;
    }
    

    bool start_broadcast = false;

    if(ant_payload_queue_count == 0 || ant_payload_queue_count == ANT_PAYLOAD_QUEUE_SIZE){
        //If there was nothing being broadcast, or the oldest broadcast has been overwritten
        start_broadcast = true;
    }

    memcpy(&ant_payload_queue[ant_payload_queue_head], ant_payload, sizeof(mm_ant_payload_t));
    ant_payload_queue_head++;
    ant_payload_queue_head %= ANT_PAYLOAD_QUEUE_SIZE;
    if(ant_payload_queue_count >= ANT_PAYLOAD_QUEUE_SIZE){
        //At capacity: Incremenet the tail as well
        ant_payload_queue_tail++;
        ant_payload_queue_tail %= ANT_PAYLOAD_QUEUE_SIZE;
    }
    else{
        //If not at capactiy, increment count
        ant_payload_queue_count++;
    }

    if(start_broadcast){
        //Broadcast a new message
        mm_ant_payload_t const * tail = ant_payload_queue_peak_tail();
        mm_ant_page_manager_add_page(tail->data[0], tail, CONCURRENT_PAGE_COUNT);
    }
    
}

static mm_ant_payload_t* ant_payload_queue_peak_tail(void)
{
    if(ant_payload_queue_count == 0)
    {
        return NULL;
    }

    return &ant_payload_queue[ant_payload_queue_tail];

}

static void process_ant_evt(ant_evt_t * evt)
{
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;

    switch (evt->event)
    {
        // If this is a "received message" event, take a closer look
        case EVENT_RX:

            if (p_message->ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID
                || p_message->ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID)
            {
                if (p_message->ANT_MESSAGE_aucPayload[PAGE_NUM_INDEX] == MONITORING_ACKNOWLEDGEMENT_PAGE_NUM)
                {
                    // Check if the message being acknowledged is the message being broadcast.
                    mm_ant_payload_t const * tail = ant_payload_queue_peak_tail();
                    if(tail->data[MESSAGE_ID_INDEX] == p_message->ANT_MESSAGE_aucPayload[MESSAGE_ID_INDEX])
                    {
                        //If true, then broadcast the next message
                        state = BROADCAST_NEW_PAGE;
                    }
                }
            }
            break;

        default:
            break;
    }
}


static void broadcast_new_page(void)
{
    // Pop last element, peek the new last element and broadcast it.
    // Removes all pages of the type that was being broadcast 
    mm_ant_payload_t const * tail = ant_payload_queue_peak_tail();
    if(tail == NULL){
        APP_ERROR_CHECK(true);
    }
    mm_ant_page_manager_remove_all_pages(tail->data[0]);

    // Pop the page that was being broadcast 
    ant_payload_queue_pop();

    // Add the new page to be broadcast
    tail = ant_payload_queue_peak_tail();
    if(tail != NULL){
        mm_ant_page_manager_add_page(tail->data[0], tail, CONCURRENT_PAGE_COUNT);
    }
}
