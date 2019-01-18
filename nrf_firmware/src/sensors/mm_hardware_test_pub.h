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
#include "mm_switch_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * @brief Reads the dip switches to find out what the hardware
 *        configuration should be, then initializes sensors
 *        as appropriate.
 */
void mm_hardware_test_init(void);

/**
 * @brief Function for updating the hardware test from main.
 */
void mm_hardware_test_update_main(void);

#ifdef __cplusplus
}
#endif

#endif /* MM_HARDWARE_TEST_PUB_H_ */
