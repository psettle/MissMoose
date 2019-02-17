/**
file: test_output_logger.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <ctime>
#include <iostream>
#include <fstream>

#include "test_output_logger.hpp"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define LOG_FILE_NAME_EXTENSION         ( std::string(".txt") )

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
    test_output_file.open(test_name + LOG_FILE_NAME_EXTENSION);

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
void log_message(std::string message)
{
    std::time_t current_time = std::time(nullptr);

	/* There's a newline at the end of the string asctime returns. This removes it. */
	char * current_time_chars = std::asctime(std::localtime(&current_time));
	current_time_chars[strlen(current_time_chars) - 1] = 0;

    if (test_output_file.is_open())
    {
        /**
         * For example, the output would look like:
         * 
         * [Sun Feb 17 02:07:24 2019] Hello World!
         */
        test_output_file << "[" << current_time_chars << "] " << message << "\n";
    }
}

/**
 * Writes message heading to opened log file. Naturally
 * appends newline to the end of the message. Doesn't do anything
 * if there is no currently open output stream.
 */
void log_heading(std::string heading)
{
	if (test_output_file.is_open())
	{
		/**
		 * For example, the output would look like:
		 *
		 * [AN EVENT HAS OCCURRED]
		 */
		test_output_file << "[" << heading << "]\n";
	}
}