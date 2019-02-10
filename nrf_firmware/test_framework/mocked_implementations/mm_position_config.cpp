/**
file: mm_position_config.cpp
brief: Mocked functions to simulate position configuration information for the test framework
notes:

Author: Elijah Pennoyer
*/

/**********************************************************
                        INCLUDES
**********************************************************/
extern "C" {
    #include "mm_position_config.h"
    #include "mm_switch_config.h"
	#include "app_error.h"
}
#include <string.h>
#include <vector>

/**********************************************************
                        CONSTANTS
**********************************************************/
#define MAX_NUMBER_NODES                    ( 9 )


/**********************************************************
                        ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       VARIABLES
**********************************************************/
static std::vector<mm_node_position_t> node_positions(MAX_NUMBER_NODES);
static bool have_positions_changed = true;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/* 
 * Initialize position configuration 
 * Note: As of Feb 8, 2019: The node positions are based on the Sensor Algorithm 1.0 file.
 * Node IDs are generated right to left, starting from the top left node (ID 1)
 * 
*/
void mm_position_config_init(void) 
{
    have_positions_changed = true;

    //Clear node_positions list, so that anything from the previous test is removed
    node_positions.clear();

    //For now, hardcode inputting the 9 nodes of our network. 

    //Top left node
    mm_node_position_t top_left;
    top_left.node_id = 1;
    top_left.node_type = HARDWARE_CONFIG_PIR_LIDAR_LED;
    top_left.node_rotation = NODE_ROTATION_90;
    top_left.grid_position_x = -1;
    top_left.grid_position_y = 1;
    top_left.grid_offset_x = 0;
    top_left.grid_offset_y = 0;
    top_left.is_valid = true;
    node_positions.push_back(top_left);

    mm_node_position_t top_middle;
    top_middle.node_id = 2;
    top_middle.node_type = HARDWARE_CONFIG_PIR_LIDAR_LED;
    top_middle.node_rotation = NODE_ROTATION_180;
    top_middle.grid_position_x = 0;
    top_middle.grid_position_y = 1;
    top_middle.grid_offset_x = 0;
    top_middle.grid_offset_y = 0;
    top_middle.is_valid = true;
    node_positions.push_back(top_middle);

    mm_node_position_t top_right;
    top_right.node_id = 3;
    top_right.node_type = HARDWARE_CONFIG_PIR_LIDAR_LED;
    top_right.node_rotation = NODE_ROTATION_180;
    top_right.grid_position_x = 1;
    top_right.grid_position_y = 1;
    top_right.grid_offset_x = 0;
    top_right.grid_offset_y = 0;
    top_right.is_valid = true;
    node_positions.push_back(top_right);

    mm_node_position_t middle_left;
    middle_left.node_id = 4;
    middle_left.node_type = HARDWARE_CONFIG_PIR_LIDAR;
    middle_left.node_rotation = NODE_ROTATION_90;
    middle_left.grid_position_x = -1;
    middle_left.grid_position_y = 0;
    middle_left.grid_offset_x = 0;
    middle_left.grid_offset_y = 0;
    middle_left.is_valid = true;
    node_positions.push_back(middle_left);

    mm_node_position_t middle_middle;
    middle_middle.node_id = 5;
    middle_middle.node_type = HARDWARE_CONFIG_PIR_PIR;
    middle_middle.node_rotation = NODE_ROTATION_0;
    middle_middle.grid_position_x = 0;
    middle_middle.grid_position_y = 0;
    middle_middle.grid_offset_x = 0;
    middle_middle.grid_offset_y = 0;
    middle_middle.is_valid = true;
    node_positions.push_back(middle_middle);

    mm_node_position_t middle_right;
    middle_right.node_id = 6;
    middle_right.node_type = HARDWARE_CONFIG_PIR_PIR;
    middle_right.node_rotation = NODE_ROTATION_0;
    middle_right.grid_position_x = 1;
    middle_right.grid_position_y = 0;
    middle_right.grid_offset_x = 0;
    middle_right.grid_offset_y = 0;
    middle_right.is_valid = true;
    node_positions.push_back(middle_right);

    mm_node_position_t bottom_left;
    bottom_left.node_id = 7;
    bottom_left.node_type = HARDWARE_CONFIG_PIR_LIDAR;
    bottom_left.node_rotation = NODE_ROTATION_0;
    bottom_left.grid_position_x = -1;
    bottom_left.grid_position_y = -1;
    bottom_left.grid_offset_x = 0;
    bottom_left.grid_offset_y = 0;
    bottom_left.is_valid = true;
    node_positions.push_back(bottom_left);

    mm_node_position_t bottom_middle;
    bottom_middle.node_id = 8;
    bottom_middle.node_type = HARDWARE_CONFIG_PIR_PIR;
    bottom_middle.node_rotation = NODE_ROTATION_90;
    bottom_middle.grid_position_x = 0;
    bottom_middle.grid_position_y = -1;
    bottom_middle.grid_offset_x = 0;
    bottom_middle.grid_offset_y = 0;
    bottom_middle.is_valid = true;
    node_positions.push_back(bottom_middle);

    mm_node_position_t bottom_right;
    bottom_right.node_id = 9;
    bottom_right.node_type = HARDWARE_CONFIG_PIR_LIDAR;
    bottom_right.node_rotation = NODE_ROTATION_270;
    bottom_right.grid_position_x = 1;
    bottom_right.grid_position_y = -1;
    bottom_right.grid_offset_x = 0;
    bottom_right.grid_offset_y = 0;
    bottom_right.is_valid = true;
    node_positions.push_back(bottom_right);

}

/* Gets the entire array of node positions for the system */
mm_node_position_t const * get_node_positions(void) 
{ 
    return node_positions.data();
}

/* Gets a specific node position by node id. Returns NULL if
 * the node doesn't exist in the list yet.
 */
mm_node_position_t const * get_position_for_node(uint16_t node_id) 
{ 
    for (auto const & position : node_positions)
    {
        if(position.node_id == node_id && position.is_valid)
        {
            return &position;
        }
    }

    return NULL;
}

/* Gets a specific node position by node location. Returns NULL if
 * the node doesn't exist in the list yet.
 */
mm_node_position_t const * get_node_for_position(int8_t x, int8_t y) 
{ 
    for (auto const & position : node_positions)
    {
        if( !position.is_valid )
        {
            continue;
        }

        if(position.grid_position_x != x)
        {
            continue;
        }

        if(position.grid_position_y != y)
        {
            continue;
        }

        return &position;
    }

    return NULL;
}

/* Gets the current number of nodes whose positions have been configured. */
uint16_t get_number_of_nodes(void) 
{ 
    uint16_t number_of_nodes = 0;
    for (auto const & position : node_positions)
    {
        if(position.is_valid)
        {
            number_of_nodes++;
        }
    }

    return number_of_nodes;
}

/* Determines if the node positions have changed since the last read.
 * Assumes that clear_unread_node_positions is called after reading
 * "unread" position changes.
 *
 * Allows users to avoid making redundant
 * checks on node position data.*/
bool have_node_positions_changed(void) 
{ 
    return have_positions_changed; 
}

/* Used to clear "unread" changes in node positions.
 *
 * Allows users to keep track of whether or not node positions
 * have changed recently, and therefore, allows them to avoid
 * making redundant checks on node position data.*/
void clear_unread_node_positions(void) 
{
    have_positions_changed = false;
}

/**
    Returns the number of sensors on a given node type and their sensor rotation constants.
*/
uint8_t get_sensor_rotations(uint8_t node_type, uint16_t sensor_rotations_size, sensor_rotation_t * sensor_rotations) 
{ 
	switch(node_type)
	{
		case(HARDWARE_CONFIG_PIR_PIR):
			APP_ERROR_CHECK_BOOL( sensor_rotations_size >= 2 );
			sensor_rotations[0] = SENSOR_ROTATION_0;
			sensor_rotations[1] = SENSOR_ROTATION_270;
			return 2;
		case(HARDWARE_CONFIG_PIR_LIDAR):
		case(HARDWARE_CONFIG_PIR_LIDAR_LED):
			APP_ERROR_CHECK_BOOL( sensor_rotations_size >= 2 );
			sensor_rotations[0] = SENSOR_ROTATION_0;
			sensor_rotations[1] = SENSOR_ROTATION_90;
			return 2;
	}
	return 0;
}
