/**
file: mm_ant_control.h
brief: High Level ANT interface for the mm project
notes:
*/

#ifndef MM_ANT_CONTROL_H
#define MM_ANT_CONTROL_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "ant_parameters.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

typedef struct mm_ant_payload_struct {
	uint8_t  data[ANT_STANDARD_DATA_PAYLOAD_SIZE];
} mm_ant_payload_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/* Init softdevice for ANT usage */
void mm_softdevice_init(void);

/* Initialize required ANT channels */
void mm_ant_init(void);

/* Set a broadcast payload. Persistents until called again. */
void mm_ant_set_payload(mm_ant_payload_t const * payload);

#endif /* MM_ANT_CONTROL_H */
