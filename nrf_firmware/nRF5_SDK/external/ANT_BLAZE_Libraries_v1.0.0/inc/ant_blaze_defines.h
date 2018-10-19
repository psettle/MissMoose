/*
This software is subject to the license described in the file: License Agreement for ANT SoftDevice and Associated Software.txt
included with this software distribution. You may not use this file except in compliance with this license.

Copyright (c) Dynastream Innovations Inc. 2017
All rights reserved.
*/

#ifndef __ANT_BLAZE_DEFINES__
#define __ANT_BLAZE_DEFINES__

#include <stdint.h>

/**@brief ANT BLAZE Message
 *
 * Represents messages sent and received from the ANT BLAZE library.
 */
typedef struct
{
    uint16_t address;       /**< Address. */
    uint32_t index;         /**< Index. */
    uint8_t length;         /**< Payload length (max ANT_BLAZE_MAX_MESSAGE_LENGTH). */
    uint8_t* p_data;        /**< Pointer to array holding data payload. */
} ant_blaze_message_t;


/**@brief Function definition for handling library errors
 * @param   error_code              Error code returned by the library.
 * @param   line_num                Line number within the library code where the error occured.
 * @param   p_file_name             File name where the error occured.
 */
typedef void (*ant_blaze_lib_error_handler_t)(uint32_t error_code, uint32_t line_num, const uint8_t* p_file_name);


/**@brief Function definition for messages received over the ANT BLAZE network.
 *
 * If this device is a node in the ANT BLAZE network (using the ANT BLAZE node library) then the message originated from a gateway.
 * In this case, the address field in the received message refers to the intended destination of the message (i.e. either ANT_BLAZE_ADDRESS_ALL_NODES,
 * a group ID, or a node ID). This allows the application to differentiate between global or individual messages.
 *
 * If this device is a gateway (using the ANT BLAZE gateway library) then the message originated from a node in the network.
 * In this case, the address field in the received message refers to the the node ID of the source node.
 *
 * @param   ant_blaze_message_t            Received message
 */
typedef void (*ant_blaze_rx_message_handler_t)(ant_blaze_message_t);


#define ANT_BLAZE_ADDRESS_ALL_NODES                 ((uint16_t) 0x0000)    /**< Message intended for all nodes.*/
#define ANT_BLAZE_ADDRESS_MAX                       ((uint16_t) 0x01FF)    /**< Maximum valid node address.*/

#define ANT_BLAZE_RESERVED_ANT_CHANNELS             5                      /**< Number of channels reserved for the ANT BLAZE library, starting at channel 0.
                                                                            *   Additional channels can be used by the application; make sure to modify ant_stack_config_defs.h to
                                                                            *   enable at least ANT_BLAZE_RESERVED_ANT_CHANNELS number of channels, plus any channels required
                                                                            *   by the application. */
#define ANT_BLAZE_NETWORK_NUMBER                    ((uint8_t) 0)          /**< Network number the mesh will operate in. Configure the ANT network key
                                                                            *   using ant_blaze_config. If application specific channels are intended to operate
                                                                            *   on the same network, assign them to this network */
#define ANT_BLAZE_MAX_MESSAGE_LENGTH                ((uint8_t) 40)         /**< Maximum length of message payload that can be processed by the ANT BLAZE library, in bytes.*/
#define ANT_BLAZE_SINGLE_PACKET_PAYLOAD_LENGTH      ((uint8_t) 5)          /**< Length of message payload in single ANT BLAZE packet, in bytes.
                                                                            *   Larger messages are automatically split and reassembled by the library. */
#define ANT_BLAZE_ENCRYPTION_KEY_LENGTH             ((uint8_t) 16)         /**< Length of encryption key, in bytes. */

#define ANT_BLAZE_PAYLOAD_ENCRYPTION_DISABLED       ((uint8_t) 0)          /**< Payload encryption disabled. */
#define ANT_BLAZE_PAYLOAD_ENCRYPTION_ENABLED        ((uint8_t) 1)          /**< Encrypt payload of ANT BLAZE packets using AES-128 CTR. */

#define ANT_BLAZE_MAX_GROUPS_PER_NODE               ((uint8_t) 8)          /**< Maximum number of groups a node can be assigned to. */
#define ANT_BLAZE_MAX_GROUP_ADDRESS_COUNT           ANT_BLAZE_ADDRESS_MAX  /**< Maximum number of addresses that can be configured as group addresses.
                                                                            *   Group addresses are assigned in the upper end of the available address space,
                                                                            *   i.e, by default, the following addresses would be reserved as group addresses:
                                                                            *   0x1E0 - 0x1FF.*/
#define ANT_BLAZE_MAX_RF_FREQ_COUNT                 ((uint8_t) 3)          /**< Maximum number of RF frequencies supported by the library. */

#define ANT_BLAZE_TIMEOUT_INTERVAL                  1000                   /**< Timer interval required for calling ant_blaze_process_timeout, in ms*/

#define ANT_BLAZE_CHANNEL_PERIOD_DYNAMIC           ((uint16_t) 0)          /**< Channel period automatically and dynamically configured by the library.*/
#define ANT_BLAZE_CHANNEL_PERIOD_1HZ               ((uint16_t) 32768)      /**< 1 Hz. */
#define ANT_BLAZE_CHANNEL_PERIOD_2HZ               ((uint16_t) 16384)      /**< 2 Hz. */
#define ANT_BLAZE_CHANNEL_PERIOD_4HZ               ((uint16_t) 8192)       /**< 4 Hz. */
#define ANT_BLAZE_CHANNEL_PERIOD_8HZ               ((uint16_t) 4096)       /**< 8 Hz. */
#define ANT_BLAZE_CHANNEL_PERIOD_16HZ              ((uint16_t) 2048)       /**< 16 Hz. */
#define ANT_BLAZE_CHANNEL_PERIOD_32HZ              ((uint16_t) 1024)       /**< 32 Hz. */

/**@brief Evaluation key for the this library. Use this key in initialization call for evaluating the library.
 * You MUST obtain a valid commercial license key BEFORE releasing a product to market that uses the library.
 */
#define ANT_BLAZE_EVALUATION_LICENSE_KEY "4556-414c-5541-5449-4f4e-20ff-ffff-88d3" // This is an EVALUATION license key - DO NOT USE FOR COMMERCIAL PURPOSES
#ifndef ANT_BLAZE_EVALUATION_LICENSE_KEY
   #error "You must obtain a valid license key to use this library. You may use the evaluation key for non commercial use only by uncommenting it above this error. Commercial use license keys are available from ANT Wireless."
#endif

/**@brief ANT BLAZE Node Configuration
 *
 * Configuration parameters for ANT BLAZE node library.
 * All parameters of this structure must be explicitly configured when initializing the library.
 * For nodes within the same network, all of the parameters must match, except by node_id, which should be unique for
 * each node.
 *
 * By default, the top ANT_BLAZE_DEFAULT_GROUP_ADDRESS_COUNT addresses are reserved as group addresses. The
 * num_group_addresses field in the sf_channel_init_t struct can be used to change this.  This value can be configured
 * to any value from 0 to ANT_BLAZE_ADDRESS_MAX.  The set of addresses that will be reserved as group addresses ranges from
 * ANT_BLAZE_ADDRESS_MAX - number_addresses + 1 to ANT_BLAZE_ADDRESS_MAX.  Addresses in the range configured for group addresses should
 * not be assigned to individual nodes as the node_id.
 */
typedef struct
{
    uint16_t node_id;               /**< 9 bit node ID.  Must be different from zero. */
    uint16_t network_id;            /**< Non zero network ID. */
    uint16_t channel_period;        /**< Valid ANT channel period.  Set to 0 to enable dynamic channel period.*/
    uint8_t radio_freqs[ANT_BLAZE_MAX_RF_FREQ_COUNT];  /**< Valid ANT RF frequencies for all beacons. */
    uint8_t num_channels;           /**< Number of beacons to enable (max ANT_BLAZE_MAX_RF_FREQ_COUNT). */
    uint8_t tx_power;               /**< Valid ANT transmit power for beacon channels, refer to ant_parameters.h for possible values. */
    uint16_t num_group_addresses;   /**< Number of addresses reserved as group addresses. */
    uint8_t encryption_enabled;     /**< Encryption configuration. Must be either ANT_BLAZE_PAYLOAD_ENCRYPTION_ENABLED or ANT_BLAZE_PAYLOAD_ENCRYPTION_DISABLED. */
    uint8_t* p_ant_network_key;     /**< Manufacturer specific ANT network key managed by Dynastream Innovations Inc. Pointer to 8-byte array. */
    uint8_t* p_encryption_key;      /**< Encryption key for this network (pointer to 16-byte array). */
} ant_blaze_node_config_t;


/**@brief ANT BLAZE Gateway Configuration
 *
 * Parameters to configure the ANT BLAZE gateway library.
 * All parameters of this structure must be explicitly configured when initializing the library.
 *
 * The gateway's node ID should be different from all of the node IDs and group IDs in the network. Network ID, radio frequencies,
 * CRC mode, network key, and encryption parameters should be exactly the same as those of the nodes in the network.
 */
typedef struct
{
    uint16_t node_id;           /**< 9 bit node ID for the gateway.  Must be different from zero and from all IDs on network. */
    uint16_t network_id;        /**< Non zero network ID. */
    uint16_t channel_period;    /**< Valid ANT channel period.  Set to 0 to enable dynamic channel period.*/
    uint8_t radio_freqs[ANT_BLAZE_MAX_RF_FREQ_COUNT];  /**< Valid ANT RF frequencies for all beacons. */
    uint8_t num_channels;       /**< Number of beacons to enable (max ANT_BLAZE_MAX_RF_FREQ_COUNT). */
    uint8_t tx_power;           /**< Valid ANT transmit power for beacon channels, refer to ant_parameters.h for possible values. */
    uint8_t encryption_enabled; /**< Encryption configuration. Must be either ANT_BLAZE_PAYLOAD_ENCRYPTION_ENABLED or ANT_BLAZE_PAYLOAD_ENCRYPTION_DISABLED. */
    uint8_t* p_ant_network_key; /**< Manufacturer specific ANT network key managed by Dynastream Innovations Inc. Pointer to 8-byte array. */
    uint8_t* p_encryption_key;  /**< Encryption key for this network (pointer to 16-byte array). */
} ant_blaze_gateway_config_t;

#endif
