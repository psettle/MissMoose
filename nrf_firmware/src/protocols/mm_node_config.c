/**
file: mm_node_config.c
brief: Handles the node configuration procedure over ANT
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/
#include "mm_node_config.h"

#include <string.h>

#include "mm_ant_control.h"
#include "mm_blaze_control.h"
#include "mm_blaze_static_config.h"
#include "mm_switch_config.h"
#include "mm_ant_page_manager.h"
#include "mm_sensor_manager.h"
#include "mm_sensor_transmission.h"
#include "mm_sensor_algorithm.h"
#include "mm_led_control.h"
#include "mm_av_transmission.h"
#include "mm_led_transmission.h"
#include "mm_sensor_error_transmission.h"
#include "mm_sensor_algorithm_config.h"

#include "bsp.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"

#include "app_timer.h"
#include "app_scheduler.h"


/**********************************************************
                        CONSTANTS
**********************************************************/

#define SENSOR_MANAGER_LED_DEBUG_ENABLED    ( false )

#define CONTROL_PIN (BSP_BUTTON_1)

#ifdef MM_BLAZE_GATEWAY
    #define TIMEOUT_PERIOD_S        ( 3600 )
#else
    #define TIMEOUT_PERIOD_S        ( 60 )
#endif

#define SENSOR_MANAGER_LED_DEBUG_ENABLED    ( false )

#define TIMEOUT_PERIOD_MS        ( TIMEOUT_PERIOD_S * 1000 )
#define TIMER_TICKS APP_TIMER_TICKS(TIMEOUT_PERIOD_MS)

/**********************************************************
                        ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/
/* Processes an ANT event */
static void ant_evt_handler(ant_evt_t* evt);
/* Processes an configuration command page. */
static void on_config_command_page(void* evt_data, uint16_t evt_size);

/* Responds to button input. */
static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
/* Processes button input */
static void on_button_press(void* evt_data, uint16_t evt_size);

/* Timer handler */
static void timer_handler(void * p_context);
/* Processes timer timeout */
static void on_timer_event(void* evt_data, uint16_t evt_size);

/* Encodes node status data page. */
static void encode_node_status_page(mm_ant_payload_t * status_page);

/* Run external initialization of blaze and things that depend on blaze. */
static void external_init(void);

/* Pauses the ANT broadcast and turns off LED 2 */
static void pause_ant_broadcast(void);

/* Resumes the ANT broadcast and turns on LED 2 */
static void resume_ant_broadcast(void);

/**********************************************************
                       VARIABLES
**********************************************************/

static uint16_t node_id;
static uint16_t network_id;

APP_TIMER_DEF(m_timer_id);

#ifdef MM_BLAZE_GATEWAY
/**
    Default sensor algorithm configuration constants.
*/
static mm_sensor_algorithm_config_t const sensor_algorithm_config_1551911794 =
{
    2.1684160132, /* activity_variable_min */
    173.0773543340, /* activity_variable_max */
    1.0000000000, /* common_sensor_weight_factor */
    1.0000000000, /* base_sensor_weight_factor_pir */
    1.0000000000, /* base_sensor_weight_factor_lidar */
    5.4670607275, /* road_proximity_factor_0 */
    6.3722118535, /* road_proximity_factor_1 */
    6.7627315568, /* road_proximity_factor_2 */
    1.0000000000, /* common_sensor_trickle_factor */
    1.2259346369, /* base_sensor_trickle_factor_pir */
    1.0000000000, /* base_sensor_trickle_factor_lidar */
    1.0000000000, /* road_trickle_proximity_factor_0 */
    1.0000000000, /* road_trickle_proximity_factor_1 */
    1.0000000000, /* road_trickle_proximity_factor_2 */
    0.9535216014, /* activity_variable_decay_factor */
    1.0000000000, /* activity_decay_period_ms */
    9.7637955926, /* possible_detection_threshold_rs */
    9.7837836894, /* possible_detection_threshold_nrs */
    41.3127704940, /* detection_threshold_rs */
    40.0230364488, /* detection_threshold_nrs */
    36.9740456420, /* minimum_concern_signal_duration_s */
    1.0000000000, /* minimum_alarm_signal_duration_s */
};
#endif


/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_node_config_init(void)
{
    // Register to receive ANT events
    mm_ant_evt_handler_set(&ant_evt_handler);

    mm_ant_pause_broadcast();

    mm_ant_payload_t payload;
    encode_node_status_page(&payload);
    mm_ant_page_manager_add_page(NODE_STATUS_PAGE, &payload, 1);

    //configure control button
    uint32_t err_code;

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(CONTROL_PIN, &in_config, control_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(CONTROL_PIN, true);

    //configure button timeout timer
    err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timer_handler);
    APP_ERROR_CHECK(err_code);
}


static void on_config_command_page(void* evt_data, uint16_t evt_size)
{
    ant_evt_t const * evt = (ant_evt_t const *)evt_data;
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;

    // Only accept node configuration if this node
    // does not already have a node_id and network_id
    if (node_id == 0 && network_id == 0)
    {
        // Set the node ID and network ID
        memcpy(&node_id, &p_message->ANT_MESSAGE_aucPayload[1], sizeof(node_id));
        memcpy(&network_id, &p_message->ANT_MESSAGE_aucPayload[3], sizeof(network_id));

        // Start init now that node ID and network ID are known
        external_init();

        mm_ant_payload_t payload;
        encode_node_status_page(&payload);
        mm_ant_page_manager_replace_all_pages(NODE_STATUS_PAGE, &payload);

    #ifdef MM_BLAZE_NODE
        // Automatically stop ANT broadcast after non-gateway node
        // is configured.
        pause_ant_broadcast();
    #endif
    }
}

static void encode_node_status_page(mm_ant_payload_t * payload)
{
    memset(&payload->data[0], 0x00, 8);

    payload->data[0] = NODE_STATUS_PAGE_NUM;

#ifdef MM_BLAZE_GATEWAY
    uint16_t gateway_id = MM_GATEWAY_ID;
    memcpy(&(payload->data[1]), &gateway_id, sizeof(gateway_id));
#else
    memcpy(&(payload->data[1]), &node_id, sizeof(node_id));
#endif

    memcpy(&(payload->data[3]), &network_id, sizeof(network_id));

    payload->data[5] = read_hardware_config();
#ifdef MM_BLAZE_GATEWAY
    payload->data[5] |= (1 << 7);
#endif

    memset(&payload->data[6], 0xFF, 2);
}

static void on_button_press(void* evt_data, uint16_t evt_size)
{
    uint32_t err_code;

    /* If broadcast is on, pause it and stop the timer. */
    if(mm_ant_get_broadcast_state())
    {
        pause_ant_broadcast();

        //stop timeout timer
        err_code = app_timer_stop(m_timer_id);
        APP_ERROR_CHECK(err_code);
    }
    /* If broadcast is off, start it and launch the timer. */
    else
    {
        resume_ant_broadcast();

        //launch timeout timer
        err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
        APP_ERROR_CHECK(err_code);
    }
}

static void on_timer_event(void* evt_data, uint16_t evt_size)
{
    /* When the timer times out, if the broadcast is on, pause it. */
    if(mm_ant_get_broadcast_state())
    {
        pause_ant_broadcast();
    }
}

static void external_init(void)
{
    /* Init blaze. */
    mm_blaze_init(node_id, network_id);
    /* Init sensor transmission over blaze. */
    mm_sensor_manager_init(SENSOR_MANAGER_LED_DEBUG_ENABLED);
    /* Init sensor transmission over blaze. */
    mm_sensor_transmission_init();
    /* Init LED control transmission over blaze. Placed before algorithm init so it can use LED control. */
    mm_led_control_init();
#ifdef MM_BLAZE_GATEWAY
    /* Init LED output transmission over ant. */
    mm_led_transmission_init();
    /* Init sensor error transmission over ant. */
    mm_sensor_error_transmission_init();
    /* Init sensor data processing now that data can be transmitted. */
    mm_sensor_algorithm_init(&sensor_algorithm_config_1551911794);
    /* Init AV output transmission over ANT now that the sensor algorithm is up and running. */
    mm_av_transmission_init();
#endif
}

/**********************************************************
                    INTERUPT HANDLERS
**********************************************************/

static void ant_evt_handler(ant_evt_t * evt)
{
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;

    switch (evt->event)
    {
        // If this is a "received message" event, take a closer look
        case EVENT_RX:

            if (p_message->ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID
                || p_message->ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID)
            {
                if (p_message->ANT_MESSAGE_aucPayload[0] == CONFIG_COMMAND_PAGE_NUM)
                {
                    uint32_t err_code;
                    /* Kick ant event to main. */
                    err_code = app_sched_event_put(evt, sizeof(ant_evt_t), on_config_command_page);
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}

static void control_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint32_t err_code;

    /* Check that button is actually set. */
    if(!nrf_drv_gpiote_in_is_set(CONTROL_PIN))
    {
        return;
    }

    /* Kick button event to main. */
    err_code = app_sched_event_put(NULL, 0, on_button_press);
    APP_ERROR_CHECK(err_code);
}


static void timer_handler(void * p_context)
{
    uint32_t err_code;

    /* Kick timer event to main. */
    err_code = app_sched_event_put(NULL, 0, on_timer_event);
    APP_ERROR_CHECK(err_code);
}

/* Pauses the ANT broadcast and turns off LED 2 */
static void pause_ant_broadcast(void)
{
    mm_ant_pause_broadcast();
    bsp_board_led_off(2);
}

/* Resumes the ANT broadcast and turns on LED 2 */
static void resume_ant_broadcast(void)
{
    mm_ant_resume_broadcast();
    bsp_board_led_on(2);
}