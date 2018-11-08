/**
 * This software is subject to the ANT+ Shared Source License
 * www.thisisant.com/swlicenses
 * Copyright (c) Dynastream Innovations, Inc. 2012
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 * 1) Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3) Neither the name of Dynastream nor the names of its
 *    contributors may be used to endorse or promote products
 *    derived from this software without specific prior
 *    written permission.
 *
 * The following actions are prohibited:
 * 1) Redistribution of source code containing the ANT+ Network
 *    Key. The ANT+ Network Key is available to ANT+ Adopters.
 *    Please refer to http://thisisant.com to become an ANT+
 *    Adopter and access the key.
 *
 * 2) Reverse engineering, decompilation, and/or disassembly of
 *    software provided in binary form under this license.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE HEREBY
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; DAMAGE TO ANY DEVICE, LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE. SOME STATES DO NOT ALLOW
 * THE EXCLUSION OF INCIDENTAL OR CONSEQUENTIAL DAMAGES, SO THE
 * ABOVE LIMITATIONS MAY NOT APPLY TO YOU.
 *
 */
/**@file
 * @defgroup ant_hrm_tx_main ANT HRM TX example
 * @{
 * @ingroup nrf_ant_hrm
 *
 * @brief Example of ANT HRM TX profile.
 *
 * Before compiling this example for NRF52, complete the following steps:
 * - Download the S212 SoftDevice from <a href="https://www.thisisant.com/developer/components/nrf52832" target="_blank">thisisant.com</a>.
 * - Extract the downloaded zip file and copy the S212 SoftDevice headers to <tt>\<InstallFolder\>/components/softdevice/s212/headers</tt>.
 * If you are using Keil packs, copy the files into a @c headers folder in your example folder.
 * - Make sure that @ref ANT_LICENSE_KEY in @c nrf_sdm.h is uncommented.
 */

#include <stdio.h>
#include "nrf_delay.h"
#include "nrf.h"
#include "nrf_drv_gpiote.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "bsp.h"
#include "hardfault.h"
#include "app_error.h"
#include "nordic_common.h"
#include "ant_stack_config.h"
#include "softdevice_handler.h"
#include "ant_state_indicator.h"
#include "ant_key_manager.h"
#include "app_timer.h"
#include "ant_blaze_node_interface.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "pir_28027_pub.h"
#include "pir_st_00081_pub.h"

#define ANTPLUS_NETWORK_NUMBER   0 /**< Network number. */

/**@brief Function for dispatching a ANT stack event to all modules with a ANT stack event handler.
 *
 * @details This function is called from the ANT Stack event interrupt handler after a ANT stack
 *          event has been received.
 *
 * @param[in] p_ant_evt  ANT stack event.
 */
void ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
}

/**
 * @brief Function for setup all thinks not directly associated witch ANT stack/protocol.
 * @desc Initialization of: @n
 *         - app_tarce for debug.
 *         - app_timer, presetup for bsp and ant pulse simulation.
 *         - bsp for signaling leds and user buttons (if use button is enabled in example).
 *         - ant pulse simulate for task of filling hrm profile data.
 */
 static void utils_setup(void)
 {
     uint32_t err_code;

     err_code = NRF_LOG_INIT(NULL);
     APP_ERROR_CHECK(err_code);

     err_code = app_timer_init();
     APP_ERROR_CHECK(err_code);

     err_code = nrf_drv_gpiote_init();
     APP_ERROR_CHECK(err_code);

     bsp_board_leds_init();
 }

/**
 * @brief Function for ANT stack initialization.
 *
 * @details Initializes the SoftDevice and the ANT event interrupt.
 */
static void softdevice_setup(void)
{
     uint32_t err_code;

     nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

     err_code = softdevice_ant_evt_handler_set(ant_evt_dispatch);
     APP_ERROR_CHECK(err_code);

     err_code = softdevice_handler_init(&clock_lf_cfg, NULL, 0, NULL);
     APP_ERROR_CHECK(err_code);

     err_code = ant_stack_static_config(); // set ant resource
     APP_ERROR_CHECK(err_code);

     err_code = ant_plus_key_set(ANTPLUS_NETWORK_NUMBER);
     APP_ERROR_CHECK(err_code);
}

void ant_blaze_rx_message_handler(ant_blaze_message_t msg)
{

}

/**@brief Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;

    utils_setup();
    softdevice_setup();

    pir_28027_init();
    // pir_st_00081_init();

    for (;; )
    {
        if (NRF_LOG_PROCESS() == false)
        {
            err_code = sd_app_evt_wait();
            APP_ERROR_CHECK(err_code);
        }
    }
}


/**
 *@}
 **/
