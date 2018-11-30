/**
file: mm_node_config.c
brief: Handles the node configuration procedure over ANT
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_node_config.h"

#include <string.h>

#include "mm_ant_control.h"
#include "mm_blaze_control.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                        ENUMS
**********************************************************/
typedef enum
{
    IDLE,
    BLAZE_INIT_PENDING,
} state_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/
/* Processes an ANT event */
static void process_ant_evt(ant_evt_t * evt);

/* Encodes node status data page. */
static void encode_node_status_page(mm_ant_payload_t * status_page);

/**********************************************************
                       VARIABLES
**********************************************************/
static uint8_t node_config_status;
static uint16_t node_id;
static uint16_t network_id;

static state_t state;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_node_config_init(void)
{
    // Node is unconfigured at initialization
    node_config_status = CONFIG_STATUS_UNCONFIGURED;

    // Register to receive ANT events
    mm_ant_evt_handler_set(&process_ant_evt);

    mm_ant_payload_t payload;
    encode_node_status_page(&payload);
    mm_ant_set_payload(&payload);
}

void mm_node_config_main(void)
{
    switch (state)
    {
        case IDLE:
            // Do nothing
            break;
        case BLAZE_INIT_PENDING:
            // Start BLAZE initialization
            mm_blaze_init(node_id, network_id);
            state = IDLE;
            break;
        default:
            // Do nothing
            break;
    }
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
                if (p_message->ANT_MESSAGE_aucPayload[0] == CONFIG_COMMAND_PAGE_NUM)
                {
                    // Only accept node configuration if this node
                    // does not already have a node_id and network_id
                    if (node_id == 0 && network_id == 0)
                    {
                        // Set the node ID and network ID
                        memcpy(&node_id, &p_message->ANT_MESSAGE_aucPayload[1], sizeof(node_id));
                        memcpy(&network_id, &p_message->ANT_MESSAGE_aucPayload[3], sizeof(network_id));

                        // Start BLAZE init now that node ID and network ID are known
                        state = BLAZE_INIT_PENDING;

                        // Node has been configured, so update the status broadcast
                        node_config_status = CONFIG_STATUS_CONFIGURED;

                        mm_ant_payload_t payload;
                        encode_node_status_page(&payload);
                        mm_ant_set_payload(&payload);
                    }
                }
            }
            break;

        default:
            break;
    }
}

static void encode_node_status_page(mm_ant_payload_t * payload)
{
    payload->data[0] = NODE_STATUS_PAGE_NUM;
    payload->data[1] = node_config_status;
    memset(&payload->data[2], 0xFF, 6);
}
