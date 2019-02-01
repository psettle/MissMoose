extern "C" {
    #include "mm_position_config.h"
}

/* Initialize position configuration */
void mm_position_config_init(void) {}

/* Gets the entire array of node positions for the system */
mm_node_position_t const * get_node_positions(void) { return NULL; }

/* Gets a specific node position by node id. Returns NULL if
 * the node doesn't exist in the list yet.
 */
mm_node_position_t const * get_position_for_node(uint16_t node_id) { return NULL; }

/* Gets a specific node position by node location. Returns NULL if
 * the node doesn't exist in the list yet.
 */
mm_node_position_t const * get_node_for_position(int8_t x, int8_t y) { return NULL; }

/* Gets the current number of nodes whose positions have been configured. */
uint16_t get_number_of_nodes(void) { return 0; }

/* Determines if the node positions have changed since the last read.
 * Assumes that clear_unread_node_positions is called after reading
 * "unread" position changes.
 *
 * Allows users to avoid making redundant
 * checks on node position data.*/
bool have_node_positions_changed(void) { return false; }

/* Used to clear "unread" changes in node positions.
 *
 * Allows users to keep track of whether or not node positions
 * have changed recently, and therefore, allows them to avoid
 * making redundant checks on node position data.*/
void clear_unread_node_positions(void) {}