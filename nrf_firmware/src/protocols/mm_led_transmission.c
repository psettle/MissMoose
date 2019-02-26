/**
file: mm_led_transmission.c
brief: Methods for transmitting led data to the monitoring application over ANT
notes:

Author: Elijah Pennoyer
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"
#include "app_scheduler.h"

#include "mm_ant_control.h"
#include "mm_ant_page_manager.h"
#include "mm_led_transmission.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_NUM_LED_NODES                       ( 3 )
#define MESSAGE_ACKNOWLEDGEMENT_PAGE_NUM        ( 0x20 )
#define LED_OUTPUT_STATUS_PAGE_NUM              ( 0x24 )
#define CONCURRENT_PAGE_COUNT                   ( 1 ) 

#define PAGE_NUM_INDEX                          ( 0 )
#define MESSAGE_ID_INDEX                        ( 1 )
#define NODE_ID_INDEX                           ( 2 )
#define CURRENT_LED_FUNCTION_INDEX              ( 4 )
#define CURRENT_LED_COLOUR_INDEX                ( 5 )
#define INDEX_6_UNUSED                          ( 6 )
#define INDEX_7_UNUSED                          ( 7 )

#define UNUSED_PAYLOAD_BYTE_VALUE               ( 0xFF )

#define ACKED_PAGE_NUM_INDEX                    ( 2 )

/**********************************************************
                          ENUMS
**********************************************************/

typedef enum
{
    NO_ELEMENT_ASSIGNED,    // Not assigned to represent anything yet
    BROADCASTING,           // This data should be getting broadcast
    NOT_BROADCASTING        // This data should not be getting broadcast 
} current_broadcast_state_t;

/**********************************************************
                       TYPES
**********************************************************/

// If no LED has been assigned to the led_page_broadcast_t, then broadcast_state is NO_ELEMENT_ASSIGNED
// When not broadcasting, state NOT_BROADCASTING. When broadcasting, state is BROADCASTING
typedef struct {
    mm_ant_payload_t led_output_page_payload; // The payload data
    current_broadcast_state_t broadcast_state; //If this page is being broadcast
} led_output_page_broadcast_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Gets an led_output_page_broadcast_t element used to store information about an LED status broadcast,
 * based on the node id 
 */
static led_output_page_broadcast_t* get_led_broadcast(uint16_t node_id);

/**
 * Clear all led_output_page_broadcasts, 
 * start led_output_page_broadcasts where broadcast_state set to BROADCAST
 */
static void broadcast_led_output_pages(void);


/**
 * Interrupt callback for ant events, kicks to on_message_acknowledge
 */
static void process_ant_evt(ant_evt_t * evt);

/**
 * Handles a broadcast acknowledgement. If it is an ack for a led_status_page_broadcast, set that LED's
 * broadcast state to NOT_BROADCASTING and clear all finished broadcasts.
 */
static void on_message_acknowledge(void* evt_data, uint16_t evt_size);

/**********************************************************
                       VARIABLES
**********************************************************/

static led_output_page_broadcast_t led_output_page_broadcasts[MAX_NUM_LED_NODES];
static uint8_t message_id = 0;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Initialize LED state transmission over ANT.
*/
void mm_led_transmission_init(void)
{
    //Initialize led_output_page_broadcasts array. This sets all elements of the struct to 0,
    //Including broadcast_state, where 0 represents NO_ELEMENT_ASSIGNED
    memset(&led_output_page_broadcasts[0], 0, sizeof(led_output_page_broadcasts));

    // Register to receive ANT events
    mm_ant_evt_handler_set(process_ant_evt);
}

/**
    Broadcast latest LED state over ANT.
*/
void mm_led_transmission_send_led_update
    (
    uint16_t node_id,
    led_function_t current_led_function,
    led_colours_t current_led_colour
    )
{
    //Get a pointer to the relevant activity variable broadcast struct
    led_output_page_broadcast_t * led_output_broadcast = get_led_broadcast(node_id); 
    uint8_t* payload = &(led_output_broadcast->led_output_page_payload.data[0]);

    memset(&(led_output_broadcast->led_output_page_payload), 0, sizeof(mm_ant_payload_t));

    payload[PAGE_NUM_INDEX] = LED_OUTPUT_STATUS_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    memcpy(&payload[NODE_ID_INDEX], &node_id, sizeof(uint16_t));
    payload[CURRENT_LED_FUNCTION_INDEX] = current_led_function;
    payload[CURRENT_LED_COLOUR_INDEX] = current_led_colour;
    //Set remaining (unused) bytes of payload to 0xFF
    payload[INDEX_6_UNUSED] = UNUSED_PAYLOAD_BYTE_VALUE;
    payload[INDEX_7_UNUSED] = UNUSED_PAYLOAD_BYTE_VALUE;

    message_id++;

    //Set page to be broadcast
    led_output_broadcast->broadcast_state = BROADCASTING;
    //Update pages being broadcast
    broadcast_led_output_pages();

}

static led_output_page_broadcast_t* get_led_broadcast(uint16_t node_id)
{
    for(unit8_t i = 0; i < MAX_NUM_LED_NODES; i++)
    {
        //Check if the broadcast is unassigned. If we reach this point, then there was no previous broadcast. Return this value.
        if(led_output_page_broadcasts[i].broadcast_state == NO_ELEMENT_ASSIGNED){
            return &led_output_page_broadcasts[i];
        }
        //If not unassigned, check for the input node_id. If found, return that.
        //Need to get the uint16_t for the node_id - Use memcpy to get two bytes out of the array
        uint16_t broadcast_node_id = 0; 
        memcpy
            (
                &broadcast_node_id,
                &(led_output_page_broadcasts[i].led_output_page_payload.data[NODE_ID_INDEX]),
                sizeof(uint16_t)
            );
        if(broadcast_node_id == node_id) //Check if the node_id matches. If it does, return the page.
        {
            return &led_output_page_broadcasts[i];
            break;
        }
    }

    //If neither of the above is true, throw an error - There should be space for the new broadcast to be stored
    APP_ERROR_CHECK(true);
    return NULL;

}

static void broadcast_led_output_pages(void)
{
    //Clear all broadcasts with LED_OUTPUT_STATUS_PAGE_NUM
    mm_ant_page_manager_remove_all_pages(LED_OUTPUT_STATUS_PAGE_NUM);

    //Broadcast all pages where broadcast_state == BROADCASTING
    for(unit8_t i = 0; i < MAX_NUM_LED_NODES; i++)
    {
        if(led_output_page_broadcasts[i].broadcast_state == BROADCASTING)
        {
            mm_ant_page_manager_add_page
                (
                    LED_OUTPUT_STATUS_PAGE_NUM, 
                    &(led_output_page_broadcasts[i].led_output_page_payload),
                    CONCURRENT_PAGE_COUNT
                );
        }
    }
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

    // Make sure that ACKED_PAGE_NUM_INDEX is correct - Should match REGION_ACTIVITY_VARIABLE_PAGE_NUM
    if(payload[ACKED_PAGE_NUM_INDEX] == MESSAGE_ACKNOWLEDGEMENT_PAGE_NUM)
    {
        // Get the message ID and search for the relevant broadcast.
        for(unit8_t i = 0; i < MAX_NUM_LED_NODES; i++){
            uint8_t payload_msg_id = payload[MESSAGE_ID_INDEX];
            uint8_t broadcast_msg_id = led_output_page_broadcasts[i].led_output_page_payload.data[MESSAGE_ID_INDEX];
            if(payload_msg_id == broadcast_msg_id)
            {
                //Indexes match, the message has been ACKed. Disable this broadcast.
                led_output_page_broadcasts[i].broadcast_state = NOT_BROADCASTING;
                break;
            }
        }
        // Resend any remaining broadcasts:
        broadcast_led_output_pages();
    }

    // If it's an ACK for a different type of message, do nothing.
    return;
}