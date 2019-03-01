/**
 *
 */
#ifndef PIR_ST_00081_PUB_H
#define PIR_ST_00081_PUB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "nrf_drv_gpiote.h"

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
 * @brief Different detection events for the PIR. compatible with boolean as long as true == 1.
 */
typedef enum
{
    PIR_EVENT_CODE_NO_DETECTION, ///< Means that the sensor is not detecting anything
    PIR_EVENT_CODE_DETECTION,    ///< Means that the sensor is currently detecting movement.
    PIR_EVENT_CODE_COUNT,
} pir_event_type_t;

/**@brief PIR event structure. */
typedef struct
{
    pir_event_type_t event;    ///< Event code.
    uint8_t sensor_index;      ///< Index of the sensor that changed it's state.
} pir_evt_t;

/**
 * @brief PIR pin change event structure.
 * Used to help keep the PIR's interrupt handler really short.
 */
typedef struct
{
    nrf_drv_gpiote_pin_t pin;  ///< Number of the pin that changed.
    bool pin_state;            ///< State that the pin changed to.
} pir_pin_change_evt_t;

/**@brief PIR event handler type. */
typedef void (*pir_evt_handler_t) (pir_evt_t * pir_evt);

/**
 * @brief Function for initializing the wide-angle PIR sensor.
 *
 * @param[in] num_pir_sensors The number of PIR sensors on the node (Maximum 3!).
 * @param[in] use_led_debug   Whether or not to use the on-board LEDs for debugging.
 */
void pir_st_00081_init(uint8_t num_pir_sensors, bool use_led_debug);

/**
 * @brief Function for disabling the wide-angle PIR sensor.
 */
void pir_st_00081_disable(uint8_t pir_sensor_id);

/**
 * @brief Function for enabling the wide-angle PIR sensor.
 */
void pir_st_00081_enable(uint8_t pir_sensor_id);

/**
 * @brief Function that can be called to add a register for PIR events.
 */
void pir_evt_handler_register(pir_evt_handler_t pir_evt_handler);

/**
 * @brief Function for checking if the wide-angle PIR sensor is enabled or disabled.
 */
bool check_pir_st_00081_enabled(uint8_t pir_sensor_id);

/**
 * @brief Function for checking if the wide-angle PIR sensor is detecting anything or not.
 */
void check_pir_st_00081_detecting(uint8_t pir_sensor_id, pir_event_type_t* sensor_event);

#ifdef __cplusplus
}
#endif

#endif /* PIR_ST_00081_PUB_H */

