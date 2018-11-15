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

#include "ant_blaze_node_interface.h"
#include "pir_28027_pub.h"
#include "pir_st_00081_pub.h"
#include "mm_ant_control.h"

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

void ant_blaze_rx_message_handler(ant_blaze_message_t msg)
{

}

#define KEY "4556-414c-5541-5449-4f4e-20ff-ffff-88d3"
/**
 * Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;

    utils_setup();
    mm_softdevice_init();
    mm_ant_init();

    mm_ant_payload_t payload;
    memset(&payload, 0, sizeof(payload));
    payload.data[0] = 1;
    payload.data[1] = 2;
    payload.data[2] = 3;
    payload.data[3] = 4;

    mm_ant_set_payload(&payload);



    err_code = ant_blaze_node_init(ant_blaze_rx_message_handler, NULL, NULL, KEY);
    if(NRF_ERROR_INVALID_LICENSE_KEY == err_code)
    {
    	bsp_board_led_on(0);
    }
    else
    {
        bsp_board_led_on(1);
    }

	err_code = ant_blaze_node_start();
	if(NRF_ERROR_NOT_SUPPORTED == err_code)
	{
		bsp_board_led_on(2);
	}
	else
	{
		bsp_board_led_on(3);
	}


    while(true)
    {
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
