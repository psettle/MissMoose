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

#endif /* MM_BLAZE_CONTROL_H */
