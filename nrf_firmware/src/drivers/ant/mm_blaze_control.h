/**
file: mm_blaze_control.h
brief:
notes:
*/

#ifndef MM_BLAZE_CONTROL_H
#define MM_BLAZE_CONTROL_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "ant_blaze_defines.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

#ifdef MM_BLAZE_NODE
	void mm_blaze_node_init(void);
#else
	void mm_blaze_gateway_init(void);
#endif

/* Send a message through blaze */
void mm_blaze_send_message(ant_blaze_message_t * message);

#endif /* MM_BLAZE_CONTROL_H */
