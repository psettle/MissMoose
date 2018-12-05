/**
file: mm_blaze_basic_txrx_demo.c
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_blaze_basic_txrx_demo.h"
#include "bsp.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "mm_blaze_control.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define CONTROL_PIN (BSP_BUTTON_1)

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
static void rx_message_handler(ant_blaze_message_t msg);


/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_blaze_basic_txrx_demo_init(void)
{
	uint32_t err_code;
    /* Sense changes in either direction on the control pin. Pull up the pin,
     * since we expect to use an IO board button for this. */
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(CONTROL_PIN, &in_config, control_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(CONTROL_PIN, true);

    mm_blaze_register_message_listener(rx_message_handler);
}

static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if(nrf_drv_gpiote_in_is_set(CONTROL_PIN))
    {
    	ant_blaze_message_t msg;

    	msg.address = 0;
    	msg.index = 0;
    	msg.p_data = (uint8_t*)"txrx";
    	msg.length = 4;

    	mm_blaze_send_message(&msg);

    	bsp_board_led_invert( 2 );
    }
}

static void rx_message_handler(ant_blaze_message_t msg)
{
	bsp_board_led_invert( 3 );
}
