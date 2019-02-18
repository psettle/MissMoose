/**
file: test_output_logger.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include <ctime>
#include <time.h>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "test_output_logger.hpp"
#include "simulate_time.hpp"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define LOG_FILE_NAME_EXTENSION         ( std::string(".txt") )

/**********************************************************
                        VARIABLES
**********************************************************/

std::ofstream test_output_file;

/**********************************************************
                       DEFINITIONS
**********************************************************/

static std::string format_time_elapsed(uint32_t time_elapsed);

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
void log_message(std::string const & message)
{
    if (test_output_file.is_open())
    {
        /**
         * For example, the output would look like:
         * 
         * [001:02:07:24] Hello World!
		 *
		 * Where the timestamp is simulated time formatted
		 * into DDD:HH:MM:SS.
         */
        test_output_file << "[" << format_time_elapsed(get_simulated_time_elapsed()) << "] " << message << "\n";
    }
}

/**
 * Writes message heading to opened log file. Naturally
 * appends newline to the end of the message. Doesn't do anything
 * if there is no currently open output stream.
 */
void log_heading(std::string const & heading)
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

static std::string format_time_elapsed(uint32_t time_elapsed)
{
	char formatted_time [13];
	std::time_t seconds(time_elapsed);
	std::tm * organized_time = gmtime(&seconds);

	std::sprintf(formatted_time, "%.3i:%.2i:%0.2i:%0.2i", organized_time->tm_yday, organized_time->tm_hour, organized_time->tm_min, organized_time->tm_sec);

	return formatted_time;
}