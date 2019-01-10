#ifndef MM_LOCATION_CONFIG_H
#define MM_LOCATION_CONFIG_H

/**********************************************************
                        INCLUDES
**********************************************************/

/**********************************************************
                        CONSTANTS
**********************************************************/
#define LOCATION_CONFIG_PAGE_NUM        0x11

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/* Initialize location configuration */
void mm_location_config_init(uint16_t node);

/*
 * Called from main superloop
 * Provides a safe thread context to make
 * potentially time-consuming function calls.
 */
void mm_location_config_main(void);

#endif /* MM_LOCATION_CONFIG_H */
