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
                        CONSTANTS
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
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

#endif /* MM_ANT_STATIC_CONFIG_H */
