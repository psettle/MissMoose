/**
file: mm_power_bank_timer_pub.h
brief: Interface for controlling a timed pin to keep the power bank on.
notes: When using a cycle of on for 10ms and off for 400ms, the recommended
       resistor is 68 Ohms.
       More information is on google drive:
       https://docs.google.com/document/d/13zAYUJn4e_S34eO-0rkp5wCtLWdB2El22oPR3_LJ65c/edit#
*/
#ifndef MM_POWER_BANK_TIMER_PUB_H
#define MM_POWER_BANK_TIMER_PUB_H

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

#define POWER_BANK_DEFAULT_ON_DURATION  10  ///< in ms. minimum amount of time to let current flow through a 33 ohm resistor to keep the bank on.
#define POWER_BANK_DEFAULT_OFF_DURATION 400 ///< in ms. maximum amount of time to let no current flow from the power bank and still keep it on.

/**********************************************************
                           TYPES
**********************************************************/

/**********************************************************
                        DECLARATIONS
**********************************************************/

/**
 * @brief Sets the pin to not draw any more current from the power bank.
 */
void mm_power_bank_disable(void);

/**
 * @brief Sets the amount of time the pin should draw power, and not draw power.
 */
void mm_power_bank_set_on_off_cycle(uint16_t on_ticks_ms, uint16_t off_ticks_ms);

/**
 * @brief Performs some initial setup like creating timers and configuring gpios
 */
void mm_power_bank_init(bool active_high);

#ifdef __cplusplus
}
#endif

#endif /* MM_POWER_BANK_TIMER_PUB_H */