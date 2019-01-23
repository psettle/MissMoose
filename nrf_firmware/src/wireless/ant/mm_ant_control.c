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
#include "nrf.h"
#include "nrf_sdm.h"
#include "nrf_mtx.h"
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
#define MAX_QUEUED_EVT ( 10 )

/**********************************************************
                       DEFINITIONS
**********************************************************/

typedef struct
{
	ant_evt_t queue[MAX_QUEUED_EVT];
	uint16_t  head;
	uint16_t  tail;
	uint16_t  count_a;
	uint16_t  count_b;
	nrf_mtx_t mtx_a;
	nrf_mtx_t mtx_b;
} ant_evt_queue_t;

/* Send ant events to listeners */
static void ant_evt_dispatch(ant_evt_t * p_ant_evt);

/* Internal callback for handling low level ant events. */
static void ant_evt_handler(ant_evt_t * p_ant_evt);

/**********************************************************
                       VARIABLES
**********************************************************/

static ant_evt_queue_t ant_evt_queue;

static mm_ant_evt_handler_t ant_evt_handlers[MAX_EVT_HANDLERS];

static bool ant_broadcast_active;

/**********************************************************
                       DECLARATIONS
**********************************************************/

void mm_softdevice_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    err_code = softdevice_ant_evt_handler_set(ant_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = softdevice_handler_init(&clock_lf_cfg, NULL, 0, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config(); // set ant resource
    APP_ERROR_CHECK(err_code);
}

void mm_ant_init(void)
{
    uint32_t err_code;

    nrf_mtx_init(&ant_evt_queue.mtx_a);
    nrf_mtx_init(&ant_evt_queue.mtx_b);

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

    err_code = ant_channel_init(&broadcast_channel_config);
    APP_ERROR_CHECK(err_code);

    //default payload
    mm_ant_payload_t payload;
    memset(&payload, 0, sizeof(payload));
    mm_ant_set_payload(&payload);

    mm_ant_resume_broadcast();
}

void mm_ant_main(void)
{
	/* Send all queued packets, only hold one mutex at a time so that
	 * the event interrupt can always find a place to write to. */

	if(nrf_mtx_trylock(&ant_evt_queue.mtx_a))
	{
	    while(ant_evt_queue.count_a)
	    {
	    	ant_evt_dispatch(&ant_evt_queue.queue[ant_evt_queue.tail]);
	    	ant_evt_queue.tail++;
	    	ant_evt_queue.tail %= MAX_QUEUED_EVT;
	    	ant_evt_queue.count_a--;
	    }

	    nrf_mtx_unlock(&ant_evt_queue.mtx_a);
	}


	if(nrf_mtx_trylock(&ant_evt_queue.mtx_b))
	{
	    while(ant_evt_queue.count_b)
	    {
	    	ant_evt_dispatch(&ant_evt_queue.queue[ant_evt_queue.tail]);
	    	ant_evt_queue.tail++;
	    	ant_evt_queue.tail %= MAX_QUEUED_EVT;
	    	ant_evt_queue.count_b--;
	    }

	    nrf_mtx_unlock(&ant_evt_queue.mtx_b);
	}
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


void mm_ant_pause_broadcast(void)
{
    uint32_t err_code;
    err_code = sd_ant_channel_close(MM_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);

    ant_broadcast_active = false;
}

void mm_ant_resume_broadcast(void)
{
    uint32_t err_code;
    err_code = sd_ant_channel_open(MM_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);

    ant_broadcast_active = true;
}

bool mm_ant_get_broadcast_state(void)
{
    return ant_broadcast_active;
}

static void ant_evt_handler(ant_evt_t * p_ant_evt)
{
	/* Grab the size, there is a race condition here where the actual size may
	 * be smaller than this, but since we are the only thread that increases it, there
	 * is no way for us to exceed the queue. */
	uint16_t ant_queue_count = ant_evt_queue.count_a + ant_evt_queue.count_b;

	if(ant_queue_count < MAX_QUEUED_EVT)
	{
		/* Enqueue the event */
		memcpy(&(ant_evt_queue.queue[ant_evt_queue.head]), p_ant_evt, sizeof(ant_evt_t));
		ant_evt_queue.head++;
		ant_evt_queue.head %= MAX_QUEUED_EVT;

		/* Try both mutexs, main can only hold one at a time,
		 * and since we cannot be interrupted, the one it holds is static within the function,
		 * so one should always be available. */
		if(nrf_mtx_trylock(&ant_evt_queue.mtx_a))
		{
			ant_evt_queue.count_a++;
			nrf_mtx_unlock(&ant_evt_queue.mtx_a);
		}
		else if(nrf_mtx_trylock(&ant_evt_queue.mtx_b))
		{
			ant_evt_queue.count_b++;
			nrf_mtx_unlock(&ant_evt_queue.mtx_b);
		}
		else
		{
			APP_ERROR_CHECK(true);
		}

	}
	else
	{
		APP_ERROR_CHECK(true);
	}
}

static void ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
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

