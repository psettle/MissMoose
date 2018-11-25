/**
file: mm_ant_control.c
brief: High Level ANT interface for the missmoose project
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <string.h>

#include "mm_ant_control.h"
#include "mm_ant_static_config.h"

#include "nrf.h"
#include "nrf_sdm.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_stack_config.h"
#include "ant_channel_config.h"
#include "ant_key_manager.h"
#include "softdevice_handler.h"
#include "boards.h"

// Hack
#include "grideye_pub.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/* Internal callback for handling low level ant events. */
static void ant_evt_dispatch(ant_evt_t * p_ant_evt);

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
        .channel_period    = MM_CHAN_PERIOD/2,
        .network_number    = MM_ANT_NETWORK_NUMBER,
    };

    err_code = ant_channel_init(&broadcast_channel_config);
    APP_ERROR_CHECK(err_code);

    //default payload
    mm_ant_payload_t payload;
    memset(&payload, 0, sizeof(payload));
    mm_ant_set_payload(&payload);

    // Open channel.
    err_code = sd_ant_channel_open(MM_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);
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
    grideye_ant_event_handler(p_ant_evt);
}
