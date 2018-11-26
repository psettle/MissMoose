/**
file: mm_ant_static_config.h
brief: Static ANT network config for the missmoose project
notes:
*/

#ifndef MM_ANT_STATIC_CONFIG_H
#define MM_ANT_STATIC_CONFIG_H

/**********************************************************
                        INCLUDES
**********************************************************/

/**********************************************************
                        ANT CONSTANTS
**********************************************************/


#define MM_CHANNEL_NUMBER	   ( 0x05 )
#define MM_CHANNEL_TYPE		   ( CHANNEL_TYPE_MASTER )
#define MM_EXT_ASSIGN		   ( 0x00 )
#define MM_RF_FREQ			   ( 66 )	//2566 Mhz
#define MM_CHAN_PERIOD 		   ( 8192 )
#define MM_CHAN_ID_DEV_NUM 	   ( 2 )
#define MM_CHAN_ID_DEV_TYPE    ( 2 )
#define MM_CHAN_ID_TRANS_TYPE  ( 1 )
#define MM_ANT_NETWORK_NUMBER  ( 0x00 ) //public network

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

#endif /* MM_ANT_STATIC_CONFIG_H */
