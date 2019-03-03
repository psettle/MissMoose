/**
file: mm_led_control.hpp
brief: Header file for test framework functions used to simulate sensors detecting data
notes:

Author: Elijah Pennoyer
*/

#ifndef MM_LED_CONTROL_HPP
#define MM_LED_CONTROL_HPP

/**********************************************************
                        INCLUDES
**********************************************************/

#include "test_output.hpp"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * Get the output log for the current test.
 */
TestOutput const & test_led_control_get_output(void);

#endif