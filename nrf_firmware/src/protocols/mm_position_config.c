/**
file: mm_position_config.c
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <string.h>

#include "app_error.h"
#include "app_scheduler.h"

#include "mm_ant_control.h"
#include "mm_position_config.h"
#include "mm_switch_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_NUMBER_NODES					( 9 )

#define PAGE_NUMBER_INDEX					( 0 )

// Index of the byte containing the node type in
// the position page. (May contain other info in the
// future!)
#define POSITION_PAGE_NODE_TYPE_INDEX		( 3 )

// Index of the byte containing the node rotation in
// the position page. (May contain other info in the
// future!)
#define POSITION_PAGE_NODE_ROTATION_INDEX	( 4 )
#define POSITION_PAGE_GRID_POSITION_INDEX	( 5 )
#define POSITION_PAGE_GRID_OFFSET_X_INDEX	( 6 )
#define POSITION_PAGE_GRID_OFFSET_Y_INDEX	( 7 )

#define NODE_TYPE_MASK						( 0x03 )
#define NODE_ROTATION_MASK					( 0x07 )

#define GRID_POSITION_X_MASK				( 0x0F )
#define GRID_POSITION_Y_MASK				( 0xF0 )

// Number of bits that describe the grid position
#define GRID_POSITION_SIZE				( 4 )

/**********************************************************
                        ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/* Processes an ANT event */
static void process_ant_evt(ant_evt_t * evt);

/* Decodes an ANT position page message payload */
static void decode_position_page(void* p_evt, uint16_t size);

/* Sign extends a number in 2's complement form */
static int8_t sign_extend( uint8_t uint, uint8_t size_bits );

/**********************************************************
                       VARIABLES
**********************************************************/

static mm_node_position_t node_positions[MAX_NUMBER_NODES];

static uint16_t current_number_of_nodes = 0;
static bool have_positions_changed = false;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_position_config_init( void )
{
	memset(&node_positions[0], 0, sizeof( node_positions ) );

	// Register to receive ANT events
    mm_ant_evt_handler_set(&process_ant_evt);
}

static void process_ant_evt(ant_evt_t * evt)
{
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;

    switch (evt->event)
    {
        // If this is a "received message" event, take a closer look
        case EVENT_RX:

            if (p_message->ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID
                || p_message->ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID)
            {
                if (p_message->ANT_MESSAGE_aucPayload[0] == POSITION_CONFIG_PAGE_NUM)
                {
                    app_sched_event_put(evt, sizeof(ant_evt_t), decode_position_page);
                }
            }
            break;

        default:
            break;
    }
}

static void decode_position_page(void* p_evt, uint16_t size)
{
    ant_evt_t * evt = (ant_evt_t*)p_evt;
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;
    uint8_t const * position_page = &(p_message->ANT_MESSAGE_aucPayload[0]);

	have_positions_changed = true;
	mm_node_position_t * node_position = NULL;

	uint16_t node_id;

	// These three bytes are "temp" variables which
	// allow us to apply a bitmask to decode
	// specific data from the payload.
	uint8_t node_type_byte;
	uint8_t node_rotation_byte;
	uint8_t grid_position_byte;

	memcpy(&node_id, &position_page[1], sizeof(node_id));

	// Iterate through the list of maintained node positions
	// and determine if the node from the message payload
	// already exists in the list...
	for ( uint16_t i = 0; i < MAX_NUMBER_NODES; i++)
	{
		if ( node_positions[i].node_id == node_id && node_positions[i].is_valid )
		{
			// If so, we're going to replace it's previous entry with
			// the new one.
			node_position = &node_positions[i];
			break;
		}
	}

	// Otherwise, if we couldn't find the node from the payload
	// in the list, we need to add it...
	if ( node_position == NULL )
	{
		// ...by iterating through the existing list...
		for ( uint16_t i = 0; i < MAX_NUMBER_NODES; i++ )
		{
			// ..and finding the first empty (non-valid) entry.
			if ( !node_positions[i].is_valid )
			{
				node_position = &node_positions[i];
				node_position->node_id = node_id;
				node_position->is_valid = true;

				current_number_of_nodes++;
				break;
			}
		}
	}

	// If we couldn't add a new node position (because the existing
	// list was full), APP_ERROR
	APP_ERROR_CHECK( node_position == NULL );

	// Read node type and rotation bytes...
	memcpy
		(
		&node_type_byte,
		&position_page[POSITION_PAGE_NODE_TYPE_INDEX],
		sizeof(node_type_byte)
		);
	memcpy
		(
		&node_rotation_byte,
		&position_page[POSITION_PAGE_NODE_ROTATION_INDEX],
		sizeof(node_rotation_byte)
		);

	// Extract relevant bits using bitmasks...
	node_position->node_type = node_type_byte & NODE_TYPE_MASK;
	node_position->node_rotation = node_rotation_byte & NODE_ROTATION_MASK;

	// Read grid position byte...
	memcpy
		(
		&grid_position_byte,
		&position_page[POSITION_PAGE_GRID_POSITION_INDEX],
		sizeof(grid_position_byte)
		);

	// Separate into the x and y grid position nibbles using bitmasks...
	node_position->grid_position_x = sign_extend( ( grid_position_byte & GRID_POSITION_X_MASK ), GRID_POSITION_SIZE );
	node_position->grid_position_y = sign_extend( ( grid_position_byte & GRID_POSITION_Y_MASK ) >> 4,  GRID_POSITION_SIZE );

	// Read grid offset bytes. Grid offset fills an entire byte, so no need
	// for bitmask.
	memcpy
		(
		&node_position->grid_offset_x,
		&position_page[POSITION_PAGE_GRID_OFFSET_X_INDEX],
		sizeof(node_position->grid_offset_x)
		);
	memcpy
		(
		&node_position->grid_offset_y,
		&position_page[POSITION_PAGE_GRID_OFFSET_Y_INDEX],
		sizeof(node_position->grid_offset_y)
		);
}

static int8_t sign_extend( uint8_t uint, uint8_t size_bits )
{
	int8_t result;
	memcpy(&result, &uint, sizeof(result));

	// Create a bitmask to extract the most significant
	// bit (MSB) from "uint"...
	uint8_t bitmask = 1 << ( size_bits - 1 );

	// Apply the bitmask to get the MSB
	uint8_t msb = uint & bitmask;

	// While MSB is not 0...
	while ( msb )
	{
		// ..shifting the MSB left and OR-ing
		// sign extends "uint"
		msb <<= 1;
		result |= msb;
	}

	return result;
}

mm_node_position_t const * get_node_positions( void )
{
	return node_positions;
}

mm_node_position_t const * get_position_for_node( uint16_t node_id )
{
	for ( uint16_t i = 0; i < MAX_NUMBER_NODES; i ++ )
	{
		if ( node_positions[i].node_id == node_id && node_positions[i].is_valid )
		{
			return &node_positions[i];
		}
	}

	return NULL;
}

mm_node_position_t const * get_node_for_position(int8_t x, int8_t y)
{
	for ( uint16_t i = 0; i < MAX_NUMBER_NODES; ++i )
	{
		if( !node_positions[i].is_valid )
		{
			continue;
		}

		if(node_positions[i].grid_position_x != x)
		{
			continue;
		}

		if(node_positions[i].grid_position_y != y)
		{
			continue;
		}

		return &node_positions[i];
	}

	return NULL;
}

uint16_t get_number_of_nodes( void )
{
	return current_number_of_nodes;
}

bool have_node_positions_changed( void )
{
	return have_positions_changed;
}

void clear_unread_node_positions( void )
{
	have_positions_changed = false;
}

/**
    Returns the number of sensors on a given node type and their sensor rotation constants.
*/
uint8_t get_sensor_rotations( uint8_t node_type, uint16_t sensor_rotations_size, sensor_rotation_t sensor_rotations[] )
{
	switch(node_type)
	{
		case(HARDWARE_CONFIG_PIR_PIR):
			APP_ERROR_CHECK_BOOL( sensor_rotations_size >= 2 );
			sensor_rotations[1] = SENSOR_ROTATION_0;
			sensor_rotations[2] = SENSOR_ROTATION_270;
			return 2;
		case(HARDWARE_CONFIG_PIR_LIDAR):
		case(HARDWARE_CONFIG_PIR_LIDAR_LED):
			APP_ERROR_CHECK_BOOL( sensor_rotations_size >= 2 );
			sensor_rotations[1] = SENSOR_ROTATION_0;
			sensor_rotations[2] = SENSOR_ROTATION_90;
			return 2;
	}
}
