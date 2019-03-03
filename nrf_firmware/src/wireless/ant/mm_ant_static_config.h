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

#define MM_CHANNEL_NUMBER        ( 0x05 ) //Blaze uses channels 0-4, we have 6 channels allocated, so 5 is the only remaining channel to use.
#define MM_CHANNEL_TYPE         ( CHANNEL_TYPE_MASTER )
#define MM_EXT_ASSIGN           ( 0x00 )
#define MM_RF_FREQ                ( 66 )    //2466 Mhz
#define MM_CHAN_PERIOD          ( 8192 ) //4 Hz
#define MM_CHAN_ID_DEV_NUM      ( (uint16_t) NRF_FICR->DEVICEID[0] )
#define MM_CHAN_ID_DEV_TYPE     ( 1 )
#define MM_CHAN_ID_TRANS_TYPE   ( 1 )
#define MM_ANT_NETWORK_NUMBER   ( 0x00 ) //public network

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

#endif /* MM_ANT_STATIC_CONFIG_H */
