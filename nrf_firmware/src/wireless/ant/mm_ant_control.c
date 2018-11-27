/**
file: mm_ant_control.c
brief: High Level ANT interface for the missmoose project
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_ant_control.h"

#include <string.h>

#include "mm_ant_static_config.h"
#include "mm_blaze_control.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_stack_config.h"
#include "ant_channel_config.h"
#include "ant_key_manager.h"
#include "softdevice_handler.h"
#include "boards.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_EVT_HANDLERS ( 10 )

/**********************************************************
                       DECLARATIONS
**********************************************************/

/* Internal callback for handling low level ant events. */
static void ant_evt_dispatch(ant_evt_t * p_ant_evt);

/* Performs internal processing of an ANT event. */
static void process_ant_evt(ant_evt_t * evt);

/* Encodes node status data page. */
static void encode_node_status_page(mm_ant_payload_t * status_page);

/**********************************************************
                       VARIABLES
**********************************************************/

static mm_ant_evt_handler_t ant_evt_handlers[MAX_EVT_HANDLERS];

static uint8_t node_config_status;
extern uint16_t node_id;
extern uint16_t network_id;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_softdevice_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    err_code = softdevice_ant_evt_handler_set(ant_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    err_code = softdevice_handler_init(&clock_lf_cfg, NULL, 0, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config(); // set ant resource
    APP_ERROR_CHECK(err_code);
}

void mm_ant_init(void)
{
    uint32_t err_code;

    ant_channel_config_t broadcast_channel_config =
    {
        .channel_number    = MM_CHANNEL_NUMBER,
        .channel_type      = MM_CHANNEL_TYPE,
        .ext_assign        = MM_EXT_ASSIGN,
        .rf_freq           = MM_RF_FREQ,
        .transmission_type = MM_CHAN_ID_TRANS_TYPE,
        .device_type       = MM_CHAN_ID_DEV_TYPE,
        .device_number     = MM_CHAN_ID_DEV_NUM,
        .channel_period    = MM_CHAN_PERIOD,
        .network_number    = MM_ANT_NETWORK_NUMBER,
    };

    node_config_status  = CONFIG_STATUS_UNCONFIGURED;

    err_code = ant_channel_init(&broadcast_channel_config);
    APP_ERROR_CHECK(err_code);

    mm_ant_payload_t payload;
    encode_node_status_page(&payload);
    mm_ant_set_payload(&payload);

    // Open channel.
    err_code = sd_ant_channel_open(MM_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);
}

void mm_ant_evt_handler_set(mm_ant_evt_handler_t mm_ant_evt_handler)
{
	uint32_t i;

	for (i = 0; i < MAX_EVT_HANDLERS; i++)
	{
		if (ant_evt_handlers[i] == NULL)
		{
			ant_evt_handlers[i] = mm_ant_evt_handler;
			break;
		}
	}

	APP_ERROR_CHECK(i == MAX_EVT_HANDLERS);
}

void mm_ant_set_payload(mm_ant_payload_t const * payload)
{
    uint32_t err_code;
    mm_ant_payload_t local_payload;
    memcpy(&local_payload, payload, sizeof(local_payload));

    // Broadcast the data.
    err_code = sd_ant_broadcast_message_tx(MM_CHANNEL_NUMBER,
    											ANT_STANDARD_DATA_PAYLOAD_SIZE,
												local_payload.data);
    APP_ERROR_CHECK(err_code);
}

static void ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
    // Perform internal processing of ANT event
    process_ant_evt(p_ant_evt);

    // Forward ANT event to listeners
	for (uint32_t i = 0; i < MAX_EVT_HANDLERS; i++)
	{
		if (ant_evt_handlers[i] != NULL)
		{
			ant_evt_handlers[i](p_ant_evt);
		}
		else
		{
			break;
		}
	}
}

static void process_ant_evt(ant_evt_t * evt)
{
    // Process the event if it occurred on the configuration app channel
    if (evt->channel == MM_CHANNEL_NUMBER)
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

                            // Initialize BLAZE now that node ID and network ID are set
                            mm_blaze_init();

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
}

static void encode_node_status_page(mm_ant_payload_t * payload)
{
    payload->data[0] = NODE_STATUS_PAGE_NUM;
    payload->data[1] = node_config_status;
    memcpy(&payload->data[2], 0xFF, 6);
}
