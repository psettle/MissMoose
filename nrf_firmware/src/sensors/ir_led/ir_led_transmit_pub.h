/**
 *
 */
#ifndef IR_LED_TRANSMIT_PUB_H
#define IR_LED_TRANSMIT_PUB_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function for initializing the IR LED transmitter.
 */
void ir_led_transmit_init(nrf_drv_gpiote_pin_t ctrl_pin, nrf_drv_gpiote_pin_t output_pin);

/**
 * @brief A call for updating the ir led transmitter.
 *
 */
void ir_led_transmit_update_main(void);
#ifdef __cplusplus
}
#endif

#endif /* IR_LED_TRANSMIT_PUB_H */