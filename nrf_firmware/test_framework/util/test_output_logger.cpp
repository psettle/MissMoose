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
 * De-initializes the test output logger.
 */
void deinit_test_output_logger(void)
{
    /* If the file wasn't successfully opened, we don't need to close it. */
    if (test_output_file.is_open())
    {
        test_output_file.close();
    }
}