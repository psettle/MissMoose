/**
file: test_output_logger.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <iostream>
#include <fstream>

#include "test_output_logger.hpp"
#include "simulate_time.hpp"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define LOG_FILE_FOLDER                 ( std::string("test_framework/logging/"))
#define LOG_FILE_NAME_EXTENSION         ( std::string(".csv") )

/**********************************************************
                        VARIABLES
**********************************************************/

std::ofstream test_output_file;

/**********************************************************
					   DECLARATIONS
**********************************************************/

/**
 * Initializes a test output logger for recording system state.
 * Creates a new output stream to a log file with name test_name.
 */
void init_test_output_logger(std::string const & test_name)
{
    test_output_file.open(LOG_FILE_FOLDER + test_name + LOG_FILE_NAME_EXTENSION);

    /* Check that the log file was able to be succesfully opened. */
    if (!test_output_file.is_open())
    {
        std::cout << std::string("Unable to open test output log file: ") + test_name + LOG_FILE_NAME_EXTENSION << std::endl;
    }
}

/**
 * De-initializes the test output logger. Closes the 
 * current output stream.
 */
void deinit_test_output_logger(void)
{
    log_message("END");

    /* If the file wasn't successfully opened, we don't need to close it. */
    if (test_output_file.is_open())
    {
        test_output_file.close();
    }
}

/**
 * Writes general message to opened log file. Naturally
 * appends newline to the end of the message. Doesn't do anything
 * if there is no currently open output stream.
 */
void log_message(std::string const & message)
{
    if (test_output_file.is_open())
    {
        test_output_file << get_simulated_time_elapsed() << "," << message << "\n";
    }
}