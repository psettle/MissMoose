/**
file: mm_rgb_led_pub.h
brief: Interface for controling an RGB LED strip
notes: To use this RGB strip library, there's generally two simple steps:
       1. Pick your colour with mm_rgb_led_set_colour. the argument can
          be any RGB hex code, but typical colours are included in this header.
       2. Choose how often you want the LED strip to be on, with
          mm_rgb_set_on_off_cycle().
          To turn the strip to be constantly on, set the first argument to be
          any number greater than zero, and the second argument to 0.
          To turn the strip to be constantly off, set the first argument to be
          0, and the second argument to any number greater than zero.
          To set the strip to flash at some rate, set the first argument to how
          long you want the led to be on, and the second argument to how long
          you want the strip to be off. The arguments are in milliseconds.
        This information and more is also available on our google drive:
        https://docs.google.com/document/d/13zAYUJn4e_S34eO-0rkp5wCtLWdB2El22oPR3_LJ65c/edit

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
#include "stdbool.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MM_RGB_LED_DUTY_0   (0x000000)

/* Define some colours with sets of duty cycle values for each colour */
#define MM_RGB_COLOUR_RED    (0xFF0000)
#define MM_RGB_COLOUR_GREEN  (0x00FF00)
#define MM_RGB_COLOUR_BLUE   (0x0000FF)
#define MM_RGB_COLOUR_YELLOW (0xFF2000)
#define MM_RGB_COLOUR_PURPLE (0xFF00FF)
#define MM_RGB_COLOUR_WHITE  (0xFFFFFF)

/**********************************************************
                           TYPES
**********************************************************/

/**********************************************************
                        DECLARATIONS
**********************************************************/

/**
 * @brief sets the desired colour of the LEDs
 */
void mm_rgb_led_set_colour(uint32_t colour);

/**
 * @brief sets the desired colour of the LEDs, with separate values for colour
 */
void mm_rgb_led_set_colour_individual(uint8_t red_duty_cycle, uint8_t green_duty_cycle, uint8_t blue_duty_cycle);

/**
 * @brief Initializes the LED strip, including all the GPIOs and timers that it uses.
 */
void mm_rgb_led_init(bool trick_power_bank);

/**
 * @brief Sets the amount of time the whole strip should be on, and off.
 */
void mm_rgb_set_on_off_cycle(uint16_t on_ticks_ms, uint16_t off_ticks_ms);

#ifdef __cplusplus
}
#endif

#endif /* MM_RGB_LED_PUB_H */