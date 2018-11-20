/**
file: mm_blaze_control.c
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/
#include <string.h>

#include "mm_blaze_control.h"
#include "mm_ant_static_config.h"

#include "app_error.h"
#include "ant_parameters.h"
#include "ant_stack_config.h"
#include "ant_blaze_node_interface.h"
#include "ant_blaze_gateway_interface.h"
#include "ant_blaze_defines.h"
#include "ant_stack_handler_types.h"
#include "ant_interface.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define DEFAULT_GROUP ((uint16_t) 511)


/**********************************************************
                       DECLARATIONS
**********************************************************/

static void rx_message_handler(ant_blaze_message_t msg);
static void backchannel_msg_handler(ant_evt_t* p_ant_evt);
static void get_node_and_network_id(uint16_t* p_node_id, uint16_t* p_network_id);

/**********************************************************
                       VARIABLES
**********************************************************/

#ifdef MM_BLAZE_NODE
	static ant_blaze_node_config_t node_config;
#else
	static ant_blaze_gateway_config_t gateway_config;
#endif

static uint8_t m_ant_network_key[] = {0xE8, 0xE4, 0x21, 0x3B, 0x55, 0x7A, 0x67, 0xC1}; // ANT Public network key. Replace with manufacturer specific ANT network key assigned by Dynastream.
static uint8_t m_encryption_key[] = {0x7D, 0x77, 0xBE, 0xE8, 0xD2, 0xE3, 0x2B, 0x2F, 0x41, 0x4C, 0x7C, 0x30, 0x89, 0xC1, 0x59, 0x1D};  // Encryption key to use for this network

/**********************************************************
                       DEFINITIONS
**********************************************************/

#ifdef MM_BLAZE_NODE
	void mm_blaze_node_init(void)
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

		// Start the ANT BLAZE library
		err_code = ant_blaze_node_start();
		APP_ERROR_CHECK(err_code);
	}
#else
	void mm_blaze_gateway_init(void)
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

		// Start the ANT BLAZE library
		err_code = ant_blaze_gateway_start();
		APP_ERROR_CHECK(err_code);
	}
#endif

static void rx_message_handler(ant_blaze_message_t msg)
{

}

static void backchannel_msg_handler(ant_evt_t* p_ant_evt)
{
    // Application can handle here messages received over the backchannel
    // of the ANT BLAZE master channel(s)
}

static void get_node_and_network_id(uint16_t* p_node_id, uint16_t* p_network_id)
{
    *p_network_id = MM_NETWORK_ID;

    #if defined(NODE_ID_FROM_SWITCHES)
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
    #endif
}
