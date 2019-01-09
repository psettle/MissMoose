/**
 *
 */
#ifndef PIR_ST_00081_PUB_H
#define PIR_ST_00081_PUB_H

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************
                       DEFINITIONS
**********************************************************/
#define PIR_SENSOR_1_ID    0
#define PIR_SENSOR_2_ID    1
#define PIR_SENSOR_3_ID    2

/**********************************************************
                       DECLARATIONS
**********************************************************/
/**
 * @brief Struct to hold pinout information for PIR sensors
 */
typedef struct pir_pinout_struct {
    //The output pin controlling the PIR enable
    uint8_t pir_en_pin_out;
    //The input pin from the PIR - High when motion is detected
    uint8_t pir_pin_in;
} pir_pinout_t;

/**
 * @brief Function for initializing the wide-angle PIR sensor.
 * 
 * @param[in] num_pir_sensors The number of PIR sensors on the node (Maximum 3!).
 */
void pir_st_00081_init(uint8_t num_pir_sensors);

/**
 * @brief Function for disabling the wide-angle PIR sensor.
 */
void pir_st_00081_disable(uint8_t pir_sensor_id);
/**
 * @brief Function for enabling the wide-angle PIR sensor.
 */
void pir_st_00081_enable(uint8_t pir_sensor_id);
/**
 * @brief Function for checking if the wide-angle PIR sensor is enabled or disabled.
 */
bool check_pir_st_00081_enabled(uint8_t pir_sensor_id);
/**
 * @brief Function for checking if the wide-angle PIR sensor is detecting anything or not.
 */
bool check_pir_st_00081_detecting(uint8_t pir_sensor_id);

#ifdef __cplusplus
}
#endif

#endif /* PIR_ST_00081_PUB_H */

