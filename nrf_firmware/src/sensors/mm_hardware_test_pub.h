/**@file mm_hardware_test_pub.h
 *
 * @brief public header for some code to run hardware tests on the nodes.
 */
#ifndef MM_HARDWARE_TEST_PUB_H_
#define MM_HARDWARE_TEST_PUB_H_

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************
                        INCLUDES
**********************************************************/
#include "app_error.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**
 * @brief This enum copied from patrick's incomplete code review.
 */
typedef enum
{
    PIR_PIR         = 0x00,	/* Switch 0 & 1 off */
    PIR_LIDAR 	    = 0x01, /* Switch 0 on, switch 1 off */
    PIR_LIDAR_LED   = 0x02  /* Switch 0 off, switch 1 on */
} hardware_config_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * @brief Reads the dip switches to find out what the hardware
 *        configuration should be, then initializes sensors
 *        as appropriate.
 */
void mm_hardware_test_init(hardware_config_t);

/**
 * @brief Function for updating the hardware test from main.
 */
void mm_hardware_test_update_main(void);

#ifdef __cplusplus
}
#endif

#endif /* MM_HARDWARE_TEST_PUB_H_ */
