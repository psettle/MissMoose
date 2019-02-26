/**
file: mm_av_transmission.c
brief: Methods for transmitting activity variable data to the monitoring application over ANT
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

#include "mm_av_transmission.h"
#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_NUM_ACTIVITY_VARIABLES              ( 4 )
#define MESSAGE_ACKNOWLEDGEMENT_PAGE_NUM        ( 0x20 )
#define REGION_ACTIVITY_VARIABLE_PAGE_NUM       ( 0x23 )
#define CONCURRENT_PAGE_COUNT                   ( 1 ) 

#define PAGE_NUM_INDEX                          ( 0 )
#define MESSAGE_ID_INDEX                        ( 1 )
#define X_Y_COORD_INDEX                         ( 2 )
#define ACTIVITY_VARIABLE_INDEX                 ( 3 )
#define AV_STATUS_INDEX                         ( 7 )

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

// If no AV has been assigned to the av_page_broadcast_t, then  broadcast_state is NO_ELEMENT_ASSIGNED
// When not broadcasting, state NOT_BROADCASTING. When broadcasting, state BROADCASTING
typedef struct {
    mm_ant_payload_t activity_variable_page_payload; // The payload data
    current_broadcast_state_t broadcast_state; //If this page is being broadcast
} av_page_broadcast_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Gets an av_page_broadcast_t element used to store information about an activity variable broadcast,
 * based on the x and y positions of that activity variable
 */
static av_page_broadcast_t* get_av_broadcast(uint8_t av_position_x, uint8_t av_position_y);

/**
 * Clear all av_page_broadcasts, start av_page_broadcasts where broadcast_state set to BROADCAST
 */
static void broadcast_av_pages(void);

/**
 * Interrupt callback for ant events, kicks to on_message_acknowledge
 */
static void process_ant_evt(ant_evt_t * evt);

/**
 * Handles a broadcast acknowledgement. If it is an ack for a av_page_broadcast, set that AV's
 * broadcast state to NOT_BROADCASTING and clear all finished broadcasts.
 */
static void on_message_acknowledge(void* evt_data, uint16_t evt_size);

/**********************************************************
                       VARIABLES
**********************************************************/

static av_page_broadcast_t av_page_broadcasts[MAX_NUM_ACTIVITY_VARIABLES];
static uint8_t message_id = 0;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Initialize activity variable state transmission over ANT.
*/
void mm_av_transmission_init(void) 
{
    //Initialize av_page_broadcasts array. This sets all elements of the struct to 0,
    //Including broadcast_state, where 0 represents NO_ELEMENT_ASSIGNED
    memset(&av_page_broadcasts[0], 0, sizeof(av_page_broadcasts));

    // Register to receive ANT events
    mm_ant_evt_handler_set(process_ant_evt);
}

/**
 * Broadcast all activity variable state over ANT.
 */
void mm_av_transmission_send_all_avs(void)
{
    for (uint8_t x = 0; x < MAX_AV_SIZE_X; ++x)
    {
        for (uint8_t y = 0; y < MAX_AV_SIZE_Y; ++y)
        {
            /* Get the region status for AV transmission... */
            activity_variable_state_t av_status = mm_get_status_for_av(&AV(x, y));

            /* Broadcast raw AV values to monitoring application over ANT. */
            mm_av_transmission_send_av_update(x, y, AV(x, y), av_status);
        }
    }
}

/**
    Broadcast latest activity variable state over ANT.
*/
void mm_av_transmission_send_av_update
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    mm_activity_variable_t av_value,
    activity_variable_state_t av_status
    ) 
{
    //Get a pointer to the relevant activity variable broadcast struct
    av_page_broadcast_t * activity_variable_broadcast = get_av_broadcast(av_position_x, av_position_x); 
    uint8_t* payload = &(activity_variable_broadcast->activity_variable_page_payload.data[0]);

    memset(&(activity_variable_broadcast->activity_variable_page_payload), 0, sizeof(mm_ant_payload_t));

    payload[PAGE_NUM_INDEX] = REGION_ACTIVITY_VARIABLE_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    //next - X and Y coordinates. 1 byte, x coordinate is the first half, y coordinate is the second half
    //Start with the y coordinate, then bitshift it 4 points to the left, then OR in the x coordinate.
    payload[X_Y_COORD_INDEX] = av_position_y;
    payload[X_Y_COORD_INDEX]  <<= 4;
    payload[X_Y_COORD_INDEX]  |= av_position_x;
    //Copy AV into payload
    memcpy(&payload[ACTIVITY_VARIABLE_INDEX], &av_value, sizeof(mm_activity_variable_t));
    payload[AV_STATUS_INDEX] = av_status;

    message_id++;

    //Set page to be broadcast
    activity_variable_broadcast->broadcast_state = BROADCASTING;
    //Update pages being broadcast
    broadcast_av_pages();
}

static av_page_broadcast_t* get_av_broadcast(uint8_t av_position_x, uint8_t av_position_y)
{
    uint8_t x_y_coord = av_position_y;
    x_y_coord  <<= 4;
    x_y_coord  |= av_position_x;

    for(int i = 0; i < MAX_NUM_ACTIVITY_VARIABLES; i++)
    {
        //Check if the broadcast is unassigned. If we reach this point, then there was no previous broadcast. Return this value.
        if(av_page_broadcasts[i].broadcast_state == NO_ELEMENT_ASSIGNED){
            return &av_page_broadcasts[i];
        }
        //If not unassigned, check for the input x_y_coord. If found, return that.
        else if(av_page_broadcasts[i].activity_variable_page_payload.data[X_Y_COORD_INDEX] == x_y_coord)
        {
            return &av_page_broadcasts[i];
            break;
        }
    }

    //If neither of the above is true, throw an error - There should be space for the new broadcast to be stored
    APP_ERROR_CHECK(true);
    return NULL;
}


static void broadcast_av_pages(void)
{
    //Clear all broadcasts with REGION_ACTIVITY_VARIABLE_PAGE_NUM
    mm_ant_page_manager_remove_all_pages(REGION_ACTIVITY_VARIABLE_PAGE_NUM);

    //Broadcast all pages where broadcast_state == BROADCASTING
    for(int i = 0; i < MAX_NUM_ACTIVITY_VARIABLES; i++)
    {
        if(av_page_broadcasts[i].broadcast_state == BROADCASTING)
        {
            mm_ant_page_manager_add_page
                (
                    REGION_ACTIVITY_VARIABLE_PAGE_NUM, 
                    &(av_page_broadcasts[i].activity_variable_page_payload),
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
    if(payload[ACKED_PAGE_NUM_INDEX] == REGION_ACTIVITY_VARIABLE_PAGE_NUM)
    {
        // Get the message ID and search for the relevant broadcast.
        for(int i = 0; i < MAX_NUM_ACTIVITY_VARIABLES; i++){
            uint8_t payload_msg_id = payload[MESSAGE_ID_INDEX];
            uint8_t broadcast_msg_id = av_page_broadcasts[i].activity_variable_page_payload.data[MESSAGE_ID_INDEX];
            if(payload_msg_id == broadcast_msg_id)
            {
                //Indexes match, the message has been ACKed. Disable this broadcast.
                av_page_broadcasts[i].broadcast_state = NOT_BROADCASTING;
                break;
            }
        }
        // Resend any remaining broadcasts:
        broadcast_av_pages();
    }

    // If it's an ACK for a different type of message, do nothing.
    return;
}