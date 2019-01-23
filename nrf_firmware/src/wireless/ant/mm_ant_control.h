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
#include "ant_stack_handler_types.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

typedef struct mm_ant_payload_struct {
	uint8_t  data[ANT_STANDARD_DATA_PAYLOAD_SIZE];
} mm_ant_payload_t;

typedef ant_evt_handler_t mm_ant_evt_handler_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/* Init softdevice for ANT usage */
void mm_softdevice_init(void);

/* Initialize required ANT channels */
void mm_ant_init(void);

/* Main callback, dispatches ant events. */
void mm_ant_main(void);

/* Add ANT event listeners */
void mm_ant_evt_handler_set(mm_ant_evt_handler_t mm_ant_evt_handler);

/* Set a broadcast payload. Persistent until called again. */
void mm_ant_set_payload(mm_ant_payload_t const * payload);

/* Stop the current ant broadcast indefinitely. */
void mm_ant_pause_broadcast(void);

/* Resume a paused broadcast. */
void mm_ant_resume_broadcast(void);

/* Get the current broadcast state, true for active, false for inactive. */
bool mm_ant_get_broadcast_state(void);

#endif /* MM_ANT_CONTROL_H */
