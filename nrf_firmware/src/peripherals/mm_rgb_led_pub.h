/**
file: mm_rgb_led_pub.h
brief: Interface for controling an RGB LED strip
notes:
*/
#ifndef MM_RGB_LED_PUB_H
#define MM_RGB_LED_PUB_H

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdint.h>

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MM_RGB_LED_DUTY_0    0

/* Define some colours with sets of duty cycle values for each colour */
#define MM_RGB_COLOUR_RED   (0xFF0000)
#define MM_RGB_COLOUR_GREEN (0x00FF00)
#define MM_RGB_COLOUR_BLUE  (0x0000FF)
#define MM_RGB_COLOUR_WHITE (0xFFFFFF)

/**********************************************************
                           TYPES
**********************************************************/

/**********************************************************
                        DECLARATIONS
**********************************************************/

/**
 * @brief sets the current colour of the LEDs
 */
void mm_rgb_led_set_colour(uint32_t colour);

/**
 * @brief sets the current colour of the LEDs, with separate values for colour
 */
void mm_rgb_led_set_colour_individual(uint8_t red_duty_cycle, uint8_t green_duty_cycle, uint8_t blue_duty_cycle);

/**
 * @brief Initializes the LED strip, including all the GPIOs and timers that it uses.
 */
void mm_rgb_led_init(void);

/**
 * @brief Sets the amount of time the whole strip should be on, and off.
 */
void mm_rgb_set_on_off_cycle(uint16_t on_ticks_ms, uint16_t off_ticks_ms);

#ifdef __cplusplus
}
#endif

#endif /* MM_RGB_LED_PUB_H */