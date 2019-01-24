#ifndef MM_POSITION_CONFIG_H
#define MM_POSITION_CONFIG_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include "nrf_soc.h"
#include "mm_sensor_transmission.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define POSITION_CONFIG_PAGE_NUM        ( 0x11 )
#define CM_PER_GRID_OFFSET				( 5 )

#define MAX_NUMBER_NODES				( 9 )
#define MAX_SENSORS_PER_NODE			( 2 )

/**********************************************************
                       DECLARATIONS
**********************************************************/

typedef enum
{
    NODE_ROTATION_0,
    NODE_ROTATION_45,
    NODE_ROTATION_90,
    NODE_ROTATION_135,
    NODE_ROTATION_180,
    NODE_ROTATION_225,
    NODE_ROTATION_270,
    NODE_ROTATION_315,

	NODE_ROTATION_COUNT
} node_rotation_t;

typedef struct mm_node_position_struct {
	uint16_t node_id;

	uint8_t 		node_type;
	node_rotation_t node_rotation;

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

	/* Gets a specific node position by using its coordinates. Returns NULL if
	 * the node doesn't exist in the list yet.
	 */
	mm_node_position_t const * get_node_for_position(int8_t x, int8_t y);

	/* Gets the current number of nodes whose positions have been configured. */
	uint16_t get_number_of_nodes( void );

	/* Determines if the node positions have changed since the last read.
	 * Assumes that clear_unread_node_positions is called after reading
	 * "unread" position changes.
	 *
	 * Allows users to avoid making redundant
	 * checks on node position data.*/
	bool have_node_positions_changed( void );

	/* Used to clear "unread" changes in node positions.
	 *
	 * Allows users to keep track of whether or not node positions
	 * have changed recently, and therefore, allows them to avoid
	 * making redundant checks on node position data.*/
	void clear_unread_node_positions( void );

	/**
		Returns the sensor rotation constants for a given node type
	*/
	void get_sensor_rotations( uint8_t node_type, sensor_rotation_t* sensor_1_rotation, sensor_rotation_t* sensor_2_rotation );

#endif

#endif /* MM_POSITION_CONFIG_H */
