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

#include "bsp.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"

#include "app_timer.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define CONTROL_PIN (BSP_BUTTON_1)
#define TIMEOUT_PERIOD_S		( 60 )
#define TIMEOUT_PERIOD_MS		( TIMEOUT_PERIOD_S * 1000 )
#define TIMER_TICKS APP_TIMER_TICKS(TIMEOUT_PERIOD_MS)

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

/* Responds to button input. */
static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

/* Timer handler */
static void timer_event(void * p_context);

/**********************************************************
                       VARIABLES
**********************************************************/
static uint8_t node_config_status;
static uint16_t node_id;
static uint16_t network_id;

static state_t state;

APP_TIMER_DEF(m_timer_id);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_node_config_init(void)
{
    // Node is unconfigured at initialization
    node_config_status = CONFIG_STATUS_UNCONFIGURED;

    // Register to receive ANT events
    mm_ant_evt_handler_set(&process_ant_evt);

    mm_ant_pause_broadcast();

    mm_ant_payload_t payload;
    encode_node_status_page(&payload);
    mm_ant_set_payload(&payload);

    //configure control button
    uint32_t err_code;

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(CONTROL_PIN, &in_config, control_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(CONTROL_PIN, true);

    //configure button timeout timer
	err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timer_event);
	APP_ERROR_CHECK(err_code);
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

static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if(nrf_drv_gpiote_in_is_set(CONTROL_PIN))
    {
    	if(mm_ant_get_broadcast_state())
    	{
    		mm_ant_pause_broadcast();
    	}
    	else
    	{
    		mm_ant_resume_broadcast();

    		//launch timeout timer
    		uint32_t err_code;
    		err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
    		APP_ERROR_CHECK(err_code);
    	}
    }
}

static void timer_event(void * p_context)
{
	if(mm_ant_get_broadcast_state())
	{
		mm_ant_pause_broadcast();
	}
}

