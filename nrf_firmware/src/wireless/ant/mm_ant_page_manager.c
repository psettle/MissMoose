/**********************************************************
                        INCLUDES
**********************************************************/

#include "mm_ant_page_manager.h"
#include "mm_ant_static_config.h"

#include <string.h>

#include "app_timer.h"

/**********************************************************
                        CONSTANTS
**********************************************************/

#define ANT_PAGE_MANAGER_TIMEOUT_INTERVAL   ( ( MM_CHAN_PERIOD * 2000 ) / 32768 )
#define TIMER_TICKS APP_TIMER_TICKS(ANT_PAGE_MANAGER_TIMEOUT_INTERVAL)
#define ANT_PAGE_MANAGER_MAX_QUEUE_SIZE     (10)

APP_TIMER_DEF(m_timer_id);

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void add_page_single ( uint8_t page_number, mm_ant_payload_t const * payload );
static void timer_event(void * p_context);

typedef struct page_queue_element_struct
{
    uint8_t             page_number;
    mm_ant_payload_t    payload;
} page_queue_element_t;

/**********************************************************
                       VARIABLES
**********************************************************/

static page_queue_element_t ant_page_manager_message_queue [ANT_PAGE_MANAGER_MAX_QUEUE_SIZE];
static uint32_t page_queue_index = 0;
static uint32_t page_queue_size = 0;

/**********************************************************
                       DEFINITIONS
**********************************************************/

void mm_ant_page_manager_init( void )
{
    memset
        (
        &ant_page_manager_message_queue[0],
        0,
        sizeof(page_queue_element_t) * ANT_PAGE_MANAGER_MAX_QUEUE_SIZE
        );

    uint32_t err_code;

    err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_REPEATED, timer_event);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_timer_id, TIMER_TICKS, NULL);
    APP_ERROR_CHECK(err_code);
}

void mm_ant_page_manager_add_page( uint8_t page_number, mm_ant_payload_t const * payload, uint8_t copies )
{
    for ( uint8_t i = 0; i < copies; i++ )
    {
        add_page_single( page_number, payload );
    }
}

void mm_ant_page_manager_remove_all_pages( uint8_t page_number )
{
    for ( uint8_t i = 0; i < ANT_PAGE_MANAGER_MAX_QUEUE_SIZE; i++ )
    {
        if (ant_page_manager_message_queue[i].page_number == page_number )
        {
            ant_page_manager_message_queue[i].page_number = 0;
            page_queue_size--;

            if ( page_queue_size == 0 )
            {
                //default payload
                mm_ant_payload_t payload;
                memset(&payload, 0, sizeof(payload));
                mm_ant_set_payload(&payload);
            }
        }
    }
}

void mm_ant_page_manager_replace_all_pages(uint8_t page_number, mm_ant_payload_t const * payload)
{
    for ( uint8_t i = 0; i < ANT_PAGE_MANAGER_MAX_QUEUE_SIZE; i++ )
    {
        if (ant_page_manager_message_queue[i].page_number == page_number )
        {
            memcpy
                (
                &( ant_page_manager_message_queue[i].payload ),
                payload,
                sizeof(mm_ant_payload_t)
                );
        }
    }
}

static void timer_event(void * p_context)
{
    if (page_queue_size == 0)
    {
        return;
    }

    do
    {
        page_queue_index++;
        page_queue_index %= ANT_PAGE_MANAGER_MAX_QUEUE_SIZE;
    }
    while(ant_page_manager_message_queue[page_queue_index].page_number == 0 );

    mm_ant_set_payload( &( ant_page_manager_message_queue[page_queue_index].payload ) );
}

static void add_page_single ( uint8_t page_number, mm_ant_payload_t const * payload )
{
    for (int i = 0; i < ANT_PAGE_MANAGER_MAX_QUEUE_SIZE; i++)
    {
        if ( ant_page_manager_message_queue[i].page_number == 0 )
        {
            ant_page_manager_message_queue[i].page_number = page_number;
            memcpy( &( ant_page_manager_message_queue[i].payload ), payload, sizeof(mm_ant_payload_t) );

            if ( page_queue_size == 0)
            {
                mm_ant_set_payload( payload );
            }

            page_queue_size++;
            return;
        }
    }

    APP_ERROR_CHECK(true);
}
