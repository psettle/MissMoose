/**
file: mm_led_control.c
brief: Methods for controlling LEDs over blaze
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_av_transmission.h"
#include "mm_sensor_algorithm_config.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                          ENUMS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**********************************************************
                       VARIABLES
**********************************************************/

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
    Initialize activity variable state transmission over ANT.
*/
void mm_av_transmission_init(void)
{
    /**
     * TODO: Actual implementation! Adding this for
     * now for use in the testing framework. To be fully implemented
     * later for sending the AV values to the monitoring application.
     */
}

/**
 * Broadcast all activity variable state over ANT.
 */
void mm_av_transmission_send_all_avs(void)
{
    for (uint8_t x = 0; x < MAX_AV_SIZE_X; ++x)
    {
        for (uint8_t y = 0; y < MAX_AV_SIZE_Y; ++y)
        {
            /* Get the region status for AV transmission... */
            activity_variable_state_t av_status = mm_get_status_for_av(&AV(x, y));

            /* Broadcast raw AV values to monitoring application over ANT. */
            mm_av_transmission_send_av_update(x, y, AV(x, y), av_status);
        }
    }
}

/**
    Broadcast latest activity variable state over ANT.
*/
void mm_av_transmission_send_av_update
    (
    uint8_t av_position_x,
    uint8_t av_position_y,
    mm_activity_variable_t av_value,
    activity_variable_state_t av_status
    ) 
{
<<<<<<< HEAD
    /**
     * TODO: Actual implementation! Adding this for
     * now for use in the testing framework. To be fully implemented
     * later for sending the AV values to the monitoring application.
     */
=======
    /* Get a pointer to the relevant activity variable broadcast struct */
    av_page_broadcast_t* av_broadcast = get_av_broadcast(av_position_x, av_position_y); 
    uint8_t* payload = &(av_broadcast->page.data[0]);

    memset(&(av_broadcast->page), 0, sizeof(mm_ant_payload_t));

    av_broadcast->av_status = av_status;

    payload[PAGE_NUM_INDEX] = REGION_ACTIVITY_VARIABLE_PAGE_NUM;
    payload[MESSAGE_ID_INDEX] = message_id;
    /* next - X and Y coordinates. 1 byte, x coordinate is the first half, y coordinate is the second half
       Start with the y coordinate, then bitshift it 4 points to the left, then OR in the x coordinate.  */
    payload[X_Y_COORD_INDEX] = av_position_y;
    payload[X_Y_COORD_INDEX] <<= 4;
    payload[X_Y_COORD_INDEX] |= av_position_x;
    /* Copy AV into payload */
    memcpy(&payload[ACTIVITY_VARIABLE_INDEX], &av_value, sizeof(mm_activity_variable_t));
    payload[AV_STATUS_INDEX] = av_status;

    message_id++;

    /* Set page to be broadcast */
    av_broadcast->state = BROADCASTING;
    /* Update pages being broadcast */
    broadcast_av_pages();
}

static av_page_broadcast_t* get_av_broadcast(uint8_t av_position_x, uint8_t av_position_y)
{
    return &av_page_broadcasts[MAX_AV_SIZE_X * av_position_y + av_position_x];
}

static void broadcast_av_pages(void)
{
    /* Clear all broadcasts with REGION_ACTIVITY_VARIABLE_PAGE_NUM */
    mm_ant_page_manager_remove_all_pages(REGION_ACTIVITY_VARIABLE_PAGE_NUM);

    /* Broadcast all pages where state == BROADCASTING */
    for(uint8_t i = 0; i < ACTIVITY_VARIABLES_NUM; i++)
    {
        if(av_page_broadcasts[i].state == BROADCASTING)
        {
            mm_ant_page_manager_add_page
                (
                    REGION_ACTIVITY_VARIABLE_PAGE_NUM, 
                    &(av_page_broadcasts[i].page),
                    CONCURRENT_PAGE_COUNT
                );
        }
    }
}


static void process_ant_evt(ant_evt_t * evt)
{
    ANT_MESSAGE * p_message = (ANT_MESSAGE *)evt->msg.evt_buffer;

    switch (evt->event)
    {
        /* If this is a "received message" event, take a closer look */
        case EVENT_RX:
            if( (p_message->ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID) ||
                (p_message->ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID) )
            {
                if (p_message->ANT_MESSAGE_aucPayload[PAGE_NUM_INDEX] == MESSAGE_ACKNOWLEDGEMENT_PAGE_NUM)
                {
                    /* This might acknowledge the current broadcast, kick it to main to check. */
                    uint32_t err_code;
                    err_code = app_sched_event_put(evt, sizeof(ant_evt_t), on_message_acknowledge);
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
>>>>>>> 1efcdb6... Fix broadcast bug in av_transmission
}
