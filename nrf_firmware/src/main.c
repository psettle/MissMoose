/**
file: main.c
brief: Main file for missmoose project
notes:
*/

/**********************************************************
                        NORDIC INCLUDES
**********************************************************/

#include <stdio.h>
#include <string.h>
#include "nrf_drv_gpiote.h"
#include "nrf_soc.h"
#include "bsp.h"
#include "app_error.h"
#include "app_timer.h"

/**********************************************************
                        PROJECT INCLUDES
**********************************************************/

#include "pir_st_00081_pub.h"
#include "ir_led_transmit_pub.h"
#include "ky_022_receive_pub.h"
#include "mm_ant_control.h"
#include "sensors/lidar/lidar_pub.h"
#include "mm_blaze_static_config.h"
#include "mm_blaze_control.h"
#include "mm_node_config.h"
#include "mm_rgb_led_pub.h"
#include "mm_ant_page_manager.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void utils_setup(void);

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;

    utils_setup();
    mm_softdevice_init();
    mm_ant_init();
    mm_ant_page_manager_init();

    #ifdef NODE_ID_FROM_CONFIG_APP
    // If getting node ID from the configuration app,
    // start the node config procedure
    mm_node_config_init();
    #else
    // Otherwise, just start up BLAZE directly
    mm_blaze_init(0, 0);
    #endif

    //ir_led_transmit_init(BSP_BUTTON_1, BSP_LED_0); // Control pin, output pin
    //ky_022_init(BSP_BUTTON_0, BSP_LED_3); // Input pin, indicator pin
    // pir_st_00081_init(2);

    // lidar_init();
    // mm_rgb_led_init(false);

    while(true)
    {
    	// lidar_update_main();
		mm_node_config_main();
		err_code = sd_app_evt_wait();
		APP_ERROR_CHECK(err_code);
    }
}

/**
 * Function for setup all thinks not directly associated witch ANT stack/protocol.
 */
 static void utils_setup(void)
 {
     uint32_t err_code;

     err_code = app_timer_init();
     APP_ERROR_CHECK(err_code);

     err_code = nrf_drv_gpiote_init();
     APP_ERROR_CHECK(err_code);

     bsp_board_leds_init();
 }
