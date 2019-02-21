/**
file: app_error.cpp
brief: Implements handling app errors.
notes:
*/

/**********************************************************
INCLUDES
**********************************************************/

#include "app_error.h"
#include <stdexcept>
#include <sstream>
#include <iostream>

/**********************************************************
DECLARATIONS
**********************************************************/

/**********************************************************
DEFINITIONS
**********************************************************/

extern "C" {

void handle_error(int line, const char* file, uint32_t code)
{
    std::stringstream ss;
    ss << file << " : Line " << line << " Err Code " << code;
    throw std::runtime_error(ss.str());
}

}