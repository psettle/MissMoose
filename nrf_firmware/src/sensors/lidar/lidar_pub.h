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
#include "app_error.h"


/**********************************************************
                        CONSTANTS
**********************************************************/

#define LIDAR_MAX_DISTANCE 4000     // cm

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/


/**
 * @brief Different sampling rates for the LIDAR
 */
typedef enum
{
    LIDAR_SAMPLE_RATE_20Hz,
    LIDAR_SAMPLE_RATE_10Hz, ///< Default.
    LIDAR_SAMPLE_RATE_5Hz,
    LIDAR_SAMPLE_RATE_4Hz,
    LIDAR_SAMPLE_RATE_2Hz,
    LIDAR_SAMPLE_RATE_1Hz,
    LIDAR_SAMPLE_RATE_COUNT,
} lidar_sample_rate_type_t;

/**
 * @brief Different sampling rates for the LIDAR
 */
typedef enum
{
    LIDAR_ERROR_NONE,          ///< No Error.
    LIDAR_ERROR_SAMPLING_FAIL,
    LIDAR_ERROR_CODE_COUNT,
} lidar_error_code_type_t;

/**
 * @brief
 */
void lidar_update_main(void);


/**
 * @brief Set the sample rate!
 */
void lidar_set_sample_rate(lidar_sample_rate_type_t rate);

/**
 * @brief Check if there's been an alarming change in distance recently
 *
 */
bool lidar_get_distance_change_flag(void);

/**
 * @brief Get the most recent median-filtered sample from the lidar.
 *
 */
uint16_t lidar_get_filtered_distance(void);

/**
 * @brief
 *
 */
void lidar_init(void);

/**
 * @brief
 *
 */
void lidar_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_SENSORS_LIDAR_LIDAR_PUB_H_ */
