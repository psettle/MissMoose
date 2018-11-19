/**
 *
 */
#ifndef KY_022_RECEIVE_PUB_H
#define KY_022_RECEIVE_PUB_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function for initializing the KY-022 IR receiver.
 */
void ky_022_init(nrf_drv_gpiote_pin_t input_pin, nrf_drv_gpiote_pin_t indicator_pin);

/**
 * @brief Function for deinitializing the ky-022 receiver's gpiote settings and events.
 */
void ky_022_deinit(void);

/* Add public function for controlling how frequently a received signal is expected.
 * You could keep track of how freqently the signal is being received by saving a timestamp
 * (or array of several most recent timestamps?) and then setting some flag when the most
 * recent timestamps are too far apart. This would be more of an exercise in power saving,
 * and for now we're just interested in seeing how far apart this guy can receive the signal
 * from the ky-005*/

#ifdef __cplusplus
}
#endif

#endif /* KY_022_RECEIVE_PUB_H */