/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_location_config.h"

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

#define CONTROL_PIN 					(BSP_BUTTON_2)
#define TIMEOUT_PERIOD_S				( 60 )
#define TIMEOUT_PERIOD_MS				( TIMEOUT_PERIOD_S * 1000 )
#define TIMER_TICKS APP_TIMER_TICKS		(TIMEOUT_PERIOD_MS)

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

/* Responds to button input. */
static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

/* Timer handler */
static void timer_event(void * p_context);

/**********************************************************
                       VARIABLES
**********************************************************/
static uint16_t node_id;

static uint8_t node_type; // Determined by the node, and therefore, can be verified
static uint8_t node_rotation;

static state_t state;

APP_TIMER_DEF(m_timer_id);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_location_config_init(uint16_t node)
{
    // Get the node and network IDs
	node_id = node;

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
    uint16_t target_node_id;

    switch (evt->event)
    {
        // If this is a "received message" event, take a closer look
        case EVENT_RX:

            if (p_message->ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID
                || p_message->ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID)
            {
                if (p_message->ANT_MESSAGE_aucPayload[0] == LOCATION_CONFIG_PAGE_NUM)
                {
                	// Get the target node id from the message payload
                	memcpy(&target_node_id, &p_message->ANT_MESSAGE_aucPayload[1], sizeof(target_node_id));

                	// Only accept location configuration if this node id
                    // matches the payload node id.
                    if (target_node_id == node_id)
                    {
                    	memcpy(&node_type, &p_message->ANT_MESSAGE_aucPayload[3], sizeof(node_type));
                    	memcpy(&node_rotation, &p_message->ANT_MESSAGE_aucPayload[4], sizeof(node_rotation));

                    	// TODO: Verify node type from configuration with that from switches...
                    }
                }
            }
            break;

        default:
            break;
    }
}

static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint32_t err_code;

    /* Check that button is actually set. */
    if(nrf_drv_gpiote_in_is_set(CONTROL_PIN))
    {
        /* If broadcast is on, pause it and stop the timer. */
        if(mm_ant_get_broadcast_state())
        {
            mm_ant_pause_broadcast();

            //stop timeout timer
            err_code = app_timer_stop(m_timer_id);
            APP_ERROR_CHECK(err_code);
    	}
        /* If broadcast is off, start it and launch the timer. */
        else
        {
            mm_ant_resume_broadcast();

            //launch timeout timer
            err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
            APP_ERROR_CHECK(err_code);
    	}
    }
}

static void timer_event(void * p_context)
{
    /* When the timer times out, if the broadcast is on, pause it. */
    if(mm_ant_get_broadcast_state())
    {
        mm_ant_pause_broadcast();
    }
}

