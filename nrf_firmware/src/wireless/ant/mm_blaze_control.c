/**
file: mm_blaze_control.c
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_blaze_control.h"

#include <string.h>

#include "mm_ant_control.h"
#include "mm_ant_static_config.h"
#include "app_error.h"
#include "app_timer.h"
#include "ant_parameters.h"
#include "ant_stack_config.h"
#include "ant_blaze_node_interface.h"
#include "ant_blaze_gateway_interface.h"
#include "ant_stack_handler_types.h"
#include "ant_interface.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define LED_DEBUG 		( true )
#if LED_DEBUG
	#include "bsp.h"
#endif

#define DEFAULT_GROUP ((uint16_t) 511)
#define TIMER_TICKS APP_TIMER_TICKS(ANT_BLAZE_TIMEOUT_INTERVAL)
APP_TIMER_DEF(m_timer_id);

#define MAX_EVT_HANDLERS	( 10 )
/**********************************************************
                       DECLARATIONS
**********************************************************/
#ifdef MM_BLAZE_NODE
	static void mm_blaze_node_init(void);
#else
	static void mm_blaze_gateway_init(void);
#endif


static void rx_message_handler(ant_blaze_message_t msg);
static void backchannel_msg_handler(ant_evt_t* p_ant_evt);
static void get_node_and_network_id(uint16_t* p_node_id, uint16_t* p_network_id);
static void timer_event(void * p_context);
static void mm_blaze_common_init(void);
static void mm_ant_evt_dispatch(ant_evt_t * p_ant_evt);

/**********************************************************
                       VARIABLES
**********************************************************/

#ifdef MM_BLAZE_NODE
	static ant_blaze_node_config_t node_config;
#else
	static ant_blaze_gateway_config_t gateway_config;
#endif

static uint16_t network_id;
static uint16_t node_id;

static mm_blaze_message_handler_t message_handlers[MAX_EVT_HANDLERS];

static uint8_t m_ant_network_key[] = {0xE8, 0xE4, 0x21, 0x3B, 0x55, 0x7A, 0x67, 0xC1}; // ANT Public network key. Replace with manufacturer specific ANT network key assigned by Dynastream.
static uint8_t m_encryption_key[] = {0x7D, 0x77, 0xBE, 0xE8, 0xD2, 0xE3, 0x2B, 0x2F, 0x41, 0x4C, 0x7C, 0x30, 0x89, 0xC1, 0x59, 0x1D};  // Encryption key to use for this network

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_blaze_init(uint16_t assigned_node_id, uint16_t assigned_network_id)
{
	node_id = assigned_node_id;
	network_id = assigned_network_id;

	#ifdef MM_BLAZE_NODE
	mm_blaze_node_init();
	#else
	mm_blaze_gateway_init();
	#endif
}

void mm_blaze_send_message(ant_blaze_message_t * message)
{
	uint32_t err_code;

#ifdef MM_BLAZE_NODE
	err_code = ant_blaze_node_send_message(message);
#else
	err_code = ant_blaze_gateway_send_message(message);
#endif
	APP_ERROR_CHECK(err_code);
}

void mm_blaze_register_message_listener(mm_blaze_message_handler_t rx_handler)
{
	uint32_t i;
	for(i = 0; i < MAX_EVT_HANDLERS; ++i)
	{
		if(message_handlers[i] == NULL)
		{
			message_handlers[i] = rx_handler;
			break;
		}
	}

	APP_ERROR_CHECK(i == MAX_EVT_HANDLERS);
}

#ifdef MM_BLAZE_NODE
	static void mm_blaze_node_init(void)
	{
		uint32_t err_code;

		// Initialize the ANT BLAZE library with Evaluation License Key.
		// If p_error_handler is not null, the system hardfaults... TODO
		err_code = ant_blaze_node_init(rx_message_handler, backchannel_msg_handler, NULL, ANT_BLAZE_EVALUATION_LICENSE_KEY);
		APP_ERROR_CHECK(err_code);

		// Configure the ANT BLAZE library.
		// This step must be performed before starting the library.
		memset(&node_config, 0, sizeof(ant_blaze_node_config_t));
		get_node_and_network_id(&(node_config.node_id), &(node_config.network_id));
		node_config.channel_period = MM_CHANNEL_PERIOD;
		node_config.radio_freqs[0] = MM_FREQ_A;
		node_config.radio_freqs[1] = MM_FREQ_B;
		node_config.radio_freqs[2] = MM_FREQ_C;
		node_config.num_channels = MM_FREQ_NUM;
		node_config.tx_power = MM_TX_POWER;
		node_config.num_group_addresses = MM_NUM_GROUP_ADDRESSES;
		node_config.encryption_enabled = MM_USE_ENCRYPTION;
		node_config.p_ant_network_key = m_ant_network_key;
		node_config.p_encryption_key = m_encryption_key;
		err_code = ant_blaze_node_config(&node_config);
		APP_ERROR_CHECK(err_code);

		// Add node to groups, if using the grouping feature
		err_code = ant_blaze_node_add_to_group(DEFAULT_GROUP);
		APP_ERROR_CHECK(err_code);

		mm_blaze_common_init();

		// Start the ANT BLAZE library
		err_code = ant_blaze_node_start();
		APP_ERROR_CHECK(err_code);
	}
#else
	static void mm_blaze_gateway_init(void)
	{
		uint32_t err_code;

		err_code = ant_blaze_gateway_init(rx_message_handler, backchannel_msg_handler, NULL, ANT_BLAZE_EVALUATION_LICENSE_KEY);
		APP_ERROR_CHECK(err_code);

		// Configure the gateway library.
		// This step must be performed before starting the gateway library.
		memset(&gateway_config, 0, sizeof(ant_blaze_gateway_config_t)); // clear the contents of gateway_config
		get_node_and_network_id(&(gateway_config.node_id), &(gateway_config.network_id));

		gateway_config.num_channels = MM_FREQ_NUM;
		gateway_config.radio_freqs[0] = MM_FREQ_A;
		gateway_config.radio_freqs[1] = MM_FREQ_B;
		gateway_config.radio_freqs[2] = MM_FREQ_C;

		gateway_config.tx_power = MM_TX_POWER;
		gateway_config.encryption_enabled = MM_USE_ENCRYPTION;
		gateway_config.p_ant_network_key = m_ant_network_key;
		gateway_config.p_encryption_key = m_encryption_key;
		err_code = ant_blaze_gateway_config(&gateway_config);
		APP_ERROR_CHECK(err_code);

		mm_blaze_common_init();

		// Start the ANT BLAZE library
		err_code = ant_blaze_gateway_start();
		APP_ERROR_CHECK(err_code);
	}
#endif

static void rx_message_handler(ant_blaze_message_t msg)
{
#if LED_DEBUG
	bsp_board_led_on( 0 );
#endif

	uint32_t i;
	for(i = 0; i< MAX_EVT_HANDLERS; ++i)
	{
		if(message_handlers[i] != NULL)
		{
			message_handlers[i](msg);
		}
	}
}

static void backchannel_msg_handler(ant_evt_t* p_ant_evt)
{
    // Application can handle here messages received over the backchannel
    // of the ANT BLAZE master channel(s)
}

static void get_node_and_network_id(uint16_t* p_node_id, uint16_t* p_network_id)
{
    *p_network_id = MM_NETWORK_ID;

    #ifdef MM_BLAZE_GATEWAY
    *p_node_id = MM_GATEWAY_ID;

    #elif defined(NODE_ID_FROM_SWITCHES)
    // Set the node ID by reading value in dip switches. Switch 1 is the lsb, switch 8 the msb.
    *p_node_id = ((nrf_gpio_pin_read(SWITCH_1) << 0) |
                  (nrf_gpio_pin_read(SWITCH_2) << 1) |
                  (nrf_gpio_pin_read(SWITCH_3) << 2) |
                  (nrf_gpio_pin_read(SWITCH_4) << 3) |
                  (nrf_gpio_pin_read(SWITCH_5) << 4) |
                  (nrf_gpio_pin_read(SWITCH_6) << 5) |
                  (nrf_gpio_pin_read(SWITCH_7) << 6) |
                  (nrf_gpio_pin_read(SWITCH_8) << 7));
    #elif defined(NODE_ID_FROM_DEVICE_ID)
    *p_node_id = ((uint16_t)(NRF_FICR->DEVICEID[0]&0x01FF)); // Set 9-bit node address based on internal Device ID.
	#elif defined(NODE_ID_FROM_CONFIG_APP)
	*p_network_id	= network_id;
	*p_node_id  	= node_id;
    #endif
}

static void timer_event(void * p_context)
{
#ifdef MM_BLAZE_NODE
		ant_blaze_node_process_timeout();
#else
		ant_blaze_gateway_process_timeout();
#endif

}

static void mm_blaze_common_init(void)
{
	uint32_t err_code;

	mm_ant_evt_handler_set(mm_ant_evt_dispatch);

	err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_REPEATED, timer_event);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
	APP_ERROR_CHECK(err_code);
}

static void mm_ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
	if (p_ant_evt->channel < ANT_BLAZE_RESERVED_ANT_CHANNELS)
	{
#ifdef MM_BLAZE_NODE
		ant_blaze_node_process_channel_event(p_ant_evt);
#else
		ant_blaze_gateway_process_channel_event(p_ant_evt);
#endif
	}
}
