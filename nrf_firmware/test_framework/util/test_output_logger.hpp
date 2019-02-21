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
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Initializes a test output logger for recording system state.
 * Creates a new output stream to a log file with name test_name.
 */
void init_test_output_logger(std::string const & test_name);

/**
 * De-initializes the test output logger. Closes the 
 * current output stream.
 */
void deinit_test_output_logger(void);

/**
 * Writes general message to opened log file. Naturally
 * appends newline to the end of the message. Doesn't do anything
 * if there is no currently open output stream.
 */
void log_message(std::string const & message);

/**
 * Writes message heading to opened log file. Naturally
 * appends newline to the end of the message. Doesn't do anything
 * if there is no currently open output stream.
 */
void log_heading(std::string const & heading);

#endif /* TEST_OUTPUT_LOGGER_HPP */