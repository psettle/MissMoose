#ifndef MM_POSITION_CONFIG_H
#define MM_POSITION_CONFIG_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "nrf_soc.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define POSITION_CONFIG_PAGE_NUM        ( 0x11 )
#define CM_PER_GRID_OFFSET				( 5 )

/**********************************************************
                       DECLARATIONS
**********************************************************/

typedef struct mm_node_position_struct {
	uint16_t node_id;

	uint8_t node_type;
	uint8_t node_rotation;

	int8_t grid_position_x;
	int8_t grid_position_y;

	int8_t grid_offset_x;
	int8_t grid_offset_y;

	bool is_valid;
} mm_node_position_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

#ifdef MM_BLAZE_GATEWAY

	/* Initialize position configuration */
	void mm_position_config_init( void );

	/* Gets the entire array of node positions for the system */
	mm_node_position_t const * get_node_positions( void );

	/* Gets a specific node position by node id. Returns NULL if
	 * the node doesn't exist in the list yet.
	 */
	mm_node_position_t const * get_position_for_node( uint16_t node_id );

	/* Gets the current number of nodes whose positions have been configured. */
	uint16_t get_number_of_nodes( void );

	/* Determines if the node positions have changed since the last read.
	 * Assumes that clear_unread_node_positions is called after reading
	 * "unread" position changes. */
	bool have_node_positions_changed( void );

	/* Used to clear "unread" changes in node positions. */
	void clear_unread_node_positions( void );

#endif

#endif /* MM_POSITION_CONFIG_H */
