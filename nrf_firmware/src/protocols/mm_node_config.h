/**
file: mm_node_config.h
brief: Handles the node configuration procedure over ANT
notes:
*/

#ifndef MM_NODE_CONFIG_H
#define MM_NODE_CONFIG_H

/**********************************************************
                        INCLUDES
**********************************************************/

/**********************************************************
                        CONSTANTS
**********************************************************/
#define NODE_STATUS_PAGE_NUM        0x01
#define CONFIG_COMMAND_PAGE_NUM     0x10

#define CONFIG_STATUS_UNCONFIGURED  0
#define CONFIG_STATUS_CONFIGURED    1

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/* Initialize node configuration */
void mm_node_config_init(void);

/*
 * Called from main superloop
 * Provides a safe thread context to make
 * potentially time-consuming function calls.
 */
void mm_node_config_main(void);

#endif /* MM_ANT_CONTROL_H */
