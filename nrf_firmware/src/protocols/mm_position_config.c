/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_position_config.h"

#include <string.h>

#include "mm_ant_control.h"
#include "app_error.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_NUMBER_NODES				( 9 )

#define NODE_TYPE_MASK					( 0x03 )
#define NODE_ROTATION_MASK				( 0x07 )

#define GRID_POSITION_X_MASK			( 0x0F )
#define GRID_POSITION_Y_MASK			( 0xF0 )

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
static void decode_position_page( uint8_t const * position_page );

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
                	decode_position_page(&(p_message->ANT_MESSAGE_aucPayload[0]));
                }
            }
            break;

        default:
            break;
    }
}

static void decode_position_page( uint8_t const * position_page )
{
	have_positions_changed = true;
	mm_node_position_t * node_position = NULL;

	uint16_t node_id;
	uint8_t node_type_byte;
	uint8_t node_rotation_byte;
	uint8_t grid_position_byte;

	memcpy(&node_id, &position_page[1], sizeof(node_id));

	for ( uint16_t i = 0; i < MAX_NUMBER_NODES; i++)
	{
		if ( node_positions[i].node_id == node_id && node_positions[i].is_valid )
		{
			node_position = &node_positions[i];
			break;
		}
	}

	if ( node_position == NULL )
	{
		for ( uint16_t i = 0; i < MAX_NUMBER_NODES; i++ )
		{
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

	APP_ERROR_CHECK( node_position == NULL );

	memcpy(&node_type_byte, &position_page[3], sizeof(node_type_byte));
	memcpy(&node_rotation_byte, &position_page[4], sizeof(node_rotation_byte));

	// Extract relevant bits using bitmasks...
	node_position->node_type = node_type_byte & NODE_TYPE_MASK;
	node_position->node_rotation = node_rotation_byte & NODE_ROTATION_MASK;

	memcpy(&grid_position_byte, &position_page[5], sizeof(grid_position_byte));

	// Separate into the x and y grid position nibbles using bitmasks...
	node_position->grid_position_x = sign_extend( grid_position_byte & GRID_POSITION_X_MASK, GRID_POSITION_SIZE );
	node_position->grid_position_y = sign_extend( ( grid_position_byte & GRID_POSITION_Y_MASK) >> 4,  GRID_POSITION_SIZE );

	memcpy(&node_position->grid_offset_x, &position_page[6], sizeof(node_position->grid_offset_x));
	memcpy(&node_position->grid_offset_y, &position_page[7], sizeof(node_position->grid_offset_y));
}

static int8_t sign_extend( uint8_t uint, uint8_t size_bits )
{
	int8_t result;
	memcpy(&result, &uint, sizeof(result));

	uint8_t bitmask = 1 << ( size_bits - 1 );
	uint8_t msb = uint & bitmask;

	while ( msb )
	{
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
