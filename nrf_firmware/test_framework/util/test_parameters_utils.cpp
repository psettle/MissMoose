/**
file: test_parameters_utils.cpp
brief:
notes:
*/

/**********************************************************
INCLUDES
**********************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "test_parameters_utils.hpp"

/**********************************************************
CONSTANTS
**********************************************************/

#define PARAMETER_FILE                ( std::string("\\..\\..\\genetic_algo\\output\\individual_file"))
#define SCORE_FILE                    ( std::string("\\..\\..\\genetic_algo\\output\\individual_score"))
#define PARAMETER_FILE_NAME_EXTENSION ( std::string(".bin") )

/**********************************************************
VARIABLES
**********************************************************/

/**********************************************************
DECLARATIONS
**********************************************************/


/**
* Parses the parameters for the algorithm out of a binary file.
*/
void test_demo_parse_parameters(std::vector<float> & parameters, const std::string executable_path, const std::string individual_index)
{
    std::string path = executable_path.substr(0, executable_path.find_last_of("\\/")) + PARAMETER_FILE + individual_index + PARAMETER_FILE_NAME_EXTENSION;
    parameters.clear();
    float f;
    std::ifstream fin(path, std::ios::binary);
    while (fin.read(reinterpret_cast<char*>(&f), sizeof(float)))
    {
        std::cout << f << '\n';
        parameters.push_back(f);
    }
}

/**
* Writes the score for testing to a binary file.
*/
void test_demo_write_score(float score, const std::string executable_path, std::string individual_index)
{
    std::string path = executable_path.substr(0, executable_path.find_last_of("\\/")) + SCORE_FILE + individual_index + PARAMETER_FILE_NAME_EXTENSION;
    
    std::ofstream out;
    out.open(path, std::ios::out | std::ios::binary);
    float f = 0.5;
    out.write(reinterpret_cast<const char*>(&score), sizeof(float));
    out.close();
}