#ifndef MM_ANT_PAGE_MANAGER_H
#define MM_ANT_PAGE_MANAGER_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_ant_control.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define NODE_STATUS_PAGE 									( 0x01 )
#define NETWORK_CONFIGURATION_COMMAND_PAGE 					( 0x10 )
#define POSITION_CONFIGURATION_COMMAND_PAGE 				( 0x11 )
#define MONITORING_APPLICATION_DATA_ACKNOWLEDGEMENT_PAGE 	( 0x20 )
#define LIDAR_MONITORING_APP_DATA_PAGE						( 0x21 )
#define PIR_MONITORING_APP_DATA PAGE						( 0x22 )

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_ant_page_manager_init( void );

void mm_ant_page_manager_add_page(uint8_t page_number, mm_ant_payload_t const * payload, uint8_t copies);

void mm_ant_page_manager_remove_all_pages(uint8_t page_number);

void mm_ant_page_manager_replace_all_pages(uint8_t page_number, mm_ant_payload_t const * payload);

#endif /* MM_ANT_PAGE_MANAGER_H */
