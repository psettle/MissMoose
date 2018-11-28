/**
file: mm_blaze_static_config.h
brief: Static Blaze config for the missmoose project
notes:
*/

#ifndef MM_BLAZE_STATIC_CONFIG_H
#define MM_BLAZE_STATIC_CONFIG_H

/**********************************************************
                        INCLUDES
**********************************************************/

/**********************************************************
                        BLAZE CONSTANTS
**********************************************************/

#define NODE_ID_FROM_DEVICE_ID

#define MM_NETWORK_ID          ( (uint16_t) 20000 )
#define MM_TX_POWER            ( RADIO_TX_POWER_LVL_3 )
#define MM_CHANNEL_PERIOD      ( ANT_BLAZE_CHANNEL_PERIOD_DYNAMIC )
#define MM_FREQ_NUM            ( (uint8_t) 1 )     /**< Set to number of desired radio frequencies (1 - 3).  */
#define MM_FREQ_A              ( (uint8_t) 11 )    /**< 2411MHz. */
#define MM_FREQ_B              ( (uint8_t) 22 )    /**< 2422MHz. */
#define MM_FREQ_C              ( (uint8_t) 33 )    /**< 2433MHz. */
#define MM_USE_ENCRYPTION      ( ANT_BLAZE_PAYLOAD_ENCRYPTION_ENABLED )
#define MM_NUM_GROUP_ADDRESSES ( (uint16_t) 32 )
#define MM_GATEWAY_ID          ( (uint16_t) 246 )

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

#endif /* MM_BLAZE_STATIC_CONFIG_H */
