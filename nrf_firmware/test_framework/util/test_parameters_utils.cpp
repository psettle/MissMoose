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
void test_demo_parse_parameters(mm_sensor_algorithm_config_t* parameters, const std::string executable_path, const std::string individual_index)
{
    std::vector<float> file_values;
    std::string path = executable_path.substr(0, executable_path.find_last_of("\\/")) + PARAMETER_FILE + individual_index + PARAMETER_FILE_NAME_EXTENSION;
    float f;
    std::ifstream fin(path, std::ios::binary);
    while (fin.read(reinterpret_cast<char*>(&f), sizeof(float)))
    {
        std::cout << f << '\n';
        file_values.push_back(f);
    }


    // A somewhat specific order is required for the arguments:
    parameters->activity_variable_min             = file_values[0];
    parameters->activity_variable_max             = file_values[1];
    parameters->common_sensor_weight_factor       = file_values[2];
    parameters->base_sensor_weight_factor_pir     = file_values[3];
    parameters->base_sensor_weight_factor_lidar   = file_values[4];
    parameters->road_proximity_factor_0           = file_values[5];
    parameters->road_proximity_factor_1           = file_values[6];
    parameters->road_proximity_factor_2           = file_values[7];
    parameters->common_sensor_trickle_factor      = file_values[8];
    parameters->base_sensor_trickle_factor_pir    = file_values[9];
    parameters->base_sensor_trickle_factor_lidar  = file_values[10];
    parameters->road_trickle_proximity_factor_0   = file_values[11];
    parameters->road_trickle_proximity_factor_1   = file_values[12];
    parameters->road_trickle_proximity_factor_2   = file_values[13];
    parameters->activity_variable_decay_factor    = file_values[14];
    parameters->activity_decay_period_ms          = (uint16_t)file_values[15];
    parameters->possible_detection_threshold_rs   = file_values[16];
    parameters->possible_detection_threshold_nrs  = file_values[17];
    parameters->detection_threshold_rs            = file_values[18];
    parameters->detection_threshold_nrs           = file_values[19];
    parameters->minimum_concern_signal_duration_s = (uint16_t)file_values[20];
    parameters->minimum_alarm_signal_duration_s   = (uint16_t)file_values[21];
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