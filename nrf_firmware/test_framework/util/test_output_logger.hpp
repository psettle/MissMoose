/**
file: test_output_logger.hpp
brief:
notes:
*/
#ifndef TEST_OUTPUT_LOGGER_HPP
#define TEST_OUTPUT_LOGGER_HPP

/**********************************************************
                        INCLUDES
**********************************************************/

#include <string>

/**********************************************************
                        VARIABLES
**********************************************************/

std::string log_filename;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Initializes a test output logger for recording system state.
 */
static void init_test_output_logger(std::string const & test_name);

/**
 * De-initializes the test output logger.
 */
static void deinit_test_output_logger(void);

#endif /* TEST_OUTPUT_LOGGER_HPP */