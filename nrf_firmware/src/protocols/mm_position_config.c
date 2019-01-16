/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_position_config.h"

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

#define CONTROL_PIN 					( BSP_BUTTON_2 )
#define TIMEOUT_PERIOD_S				( 60 )
#define TIMEOUT_PERIOD_MS				( TIMEOUT_PERIOD_S * 1000 )
#define TIMER_TICKS APP_TIMER_TICKS		( TIMEOUT_PERIOD_MS )

#define NODE_TYPE_MASK					( 0x03 )
#define NODE_ROTATION_MASK				( 0x07 )

#define GRID_POSITION_X_MASK			( 0x0F )
#define GRID_POSITION_Y_MASK			( 0xF0 )

/**********************************************************
                        ENUMS
**********************************************************/

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

static uint8_t node_type;
static uint8_t node_rotation;

static uint8_t grid_position_x;
static uint8_t grid_position_y;

static uint8_t grid_offset_x;
static uint8_t grid_offset_y;

APP_TIMER_DEF(m_timer_id);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_position_config_init( void )
{
	// Register to receive ANT events
    mm_ant_evt_handler_set(&process_ant_evt);

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
                if (p_message->ANT_MESSAGE_aucPayload[0] == POSITION_CONFIG_PAGE_NUM)
                {
                	uint8_t node_type_byte;
                	uint8_t node_rotation_byte;
                	uint8_t grid_position_byte;

                	memcpy(&node_type_byte, &p_message->ANT_MESSAGE_aucPayload[3], sizeof(node_type_byte));
                	memcpy(&node_rotation_byte, &p_message->ANT_MESSAGE_aucPayload[4], sizeof(node_rotation_byte));

                	// Extract relevant bits using bitmasks...
                	node_type = node_type_byte & NODE_TYPE_MASK;
                	node_rotation = node_rotation_byte & NODE_ROTATION_MASK;

                	memcpy(&grid_position_byte, &p_message->ANT_MESSAGE_aucPayload[5], sizeof(grid_position_byte));

                	// Separate into the x and y grid position nibbles using bitmasks...
                	grid_position_x = grid_position_byte & GRID_POSITION_X_MASK;
                	grid_position_y = grid_position_byte & GRID_POSITION_Y_MASK;

                	memcpy(&grid_offset_x, &p_message->ANT_MESSAGE_aucPayload[6], sizeof(grid_offset_x));
                	memcpy(&grid_offset_y, &p_message->ANT_MESSAGE_aucPayload[7], sizeof(grid_offset_y));
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

