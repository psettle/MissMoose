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
#include "app_scheduler.h"

/**********************************************************
                        PROJECT INCLUDES
**********************************************************/

#include "pir_st_00081_pub.h"
#include "mm_ant_control.h"
#include "lidar_pub.h"
#include "mm_blaze_static_config.h"
#include "mm_blaze_control.h"
#include "mm_node_config.h"
#include "mm_switch_config.h"
#include "mm_rgb_led_pub.h"
#include "mm_ant_page_manager.h"
#include "mm_monitoring_dispatch.h"
#include "mm_hardware_test_pub.h"
#include "mm_position_config.h"
#include "mm_sensor_manager.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define SENSOR_MANAGER_LED_DEBUG_ENABLED    ( false )

#define SCHEDULER_MAX_EVENT_SIZE			MAX( \
	sizeof(uint8_t),				\
	sizeof(ant_evt_t)			\
)

#define SCHEDULER_MAX_EVENT_COUNT			( 10 )	/* Main should run after every event, so it should be hard to queue up a lot of events. */

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void utils_init(void);
static void scheduler_init(void);

/**********************************************************
                       VARIABLES
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;

    scheduler_init();
    utils_init();

    if(read_hardware_config() == HARDWARE_CONFIG_PIR_LIDAR_LED)
    {
    	mm_rgb_led_init(false);
    }

    mm_softdevice_init();
    mm_ant_init();
    mm_ant_page_manager_init();

    #ifdef NODE_ID_FROM_CONFIG_APP
    // If getting node ID from the configuration app,
    // start the node config procedure
    mm_node_config_init();
    #else
    APP_ERROR_CHECK(true); /* Used to initialize blaze here, that is no longer a supported mode. */
    #endif

	#ifdef MM_BLAZE_GATEWAY

    mm_monitoring_dispatch_init();
	mm_position_config_init();

	#endif

    // mm_hardware_test_init();

    while(true)
    {
    	app_sched_execute();

        #ifdef MM_BLAZE_GATEWAY
                mm_monitoring_dispatch_main();
        #endif
		err_code = sd_app_evt_wait();
		APP_ERROR_CHECK(err_code);
    }
}

/**
 * Function for setup all things not directly associated witch ANT stack/protocol.
 */
 static void utils_init(void)
 {
     uint32_t err_code;

     err_code = app_timer_init();
     APP_ERROR_CHECK(err_code);

     err_code = nrf_drv_gpiote_init();
     APP_ERROR_CHECK(err_code);

     bsp_board_leds_init();

     mm_switch_config_init();
 }

 static void scheduler_init(void)
 {
	 APP_SCHED_INIT(SCHEDULER_MAX_EVENT_SIZE, SCHEDULER_MAX_EVENT_COUNT);
 }
