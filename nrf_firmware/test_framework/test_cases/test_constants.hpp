/**
file: test_constants.hpp
brief: A list of constants to be used in test cases
notes:
*/
#ifndef TEST_CONSTANTS_HPP
#define TEST_CONSTANTS_HPP

/**********************************************************
						INCLUDES
**********************************************************/

/**********************************************************
						CONSTANTS
**********************************************************/
#define LIDAR_DETECT_DISTANCE_350CM         (350) //350 cm = 3.5 meters
#define LIDAR_DETECT_DISTANCE_OUTOFRANGE    (30000) //30 meters - Outside of the range lidars are used to detect
#define PIR_DETECTION_START                 (true) //Used to indicate that a PIR has started detecting
#define PIR_DETECTION_END                   (false) //Used to indicate that a PIR has stopped detecting



#endif // TEST_CONSTANTS_HPP
