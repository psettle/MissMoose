/**
 *
 */
#ifndef SRC_SENSORS_LIDAR_LIDAR_PUB_H_
#define SRC_SENSORS_LIDAR_LIDAR_PUB_H_

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************
                        INCLUDES
**********************************************************/
#include "ant_stack_handler_types.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * @brief
 */
void lidar_update_main(void);

/**
 * @brief
 *
 */
void lidar_init(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_SENSORS_LIDAR_LIDAR_PUB_H_ */
