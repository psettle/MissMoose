/**
file: test_slow_to_fast_running_animals.cpp
brief:
notes: Idea is to test the behaviour of animals that start moving slowly, then suddenly start moving quickly
(For example - They are suddenly startled)
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include "tests.hpp"

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void slow_to_fast_bottom_left_sprint_up(TestOutput& oracle);
static void slow_to_fast_top_left_sprint_up(TestOutput& oracle);
static void slow_to_fast_bottom_right_sprint_up(TestOutput& oracle);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_slow_to_fast_running_animals(std::vector<TestCase>& tests)
{
    //TODO - Put test call(s) here
    ADD_TEST(slow_to_fast_bottom_left_sprint_up);
    ADD_TEST(slow_to_fast_top_left_sprint_up);
    ADD_TEST(slow_to_fast_bottom_right_sprint_up);
}

// Animal starts in the bottom left, entering slowly, then sprints upwards towards the road.
static void slow_to_fast_bottom_left_sprint_up(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Start detection from left end of network. First the PIR sensor
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    simulate_time(4);
    // Possible detection in bottom-left. This causes a concern output in the top left (-1,1) LED
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);


    // 4 seconds later - Lidar detects the same animal
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 400);
    // Detection in bottom-left. Alarm in top-left (-1, 1), concern in middle-left (0,1) 
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // About 3 seconds later, the lidar stops detecting. About 7 seconds after that, the PIR stops detecting.
    // Note on Lidar - Anything from about 2000 to 4000 should be an 'end of detection'
    simulate_time(3);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 3500);
    simulate_time(7);
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);



    // Animal moves slowly, takes a minute in that AV region. After about 30 seconds, the concern LED decays, 
    // and after a minute the alarm LED decays
    simulate_time(30);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

    // Animal suddenly runs towards the road, near the central sensors. It gets detected by:
    // Central PIR
    test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    // Possible detection in top left quadrant - Alarm on top left LED, concern in center LED
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Then the LIDAR after a second
    simulate_time(1);
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 700);
    // Confirmed detection in top left - Alarm on top left, alarm on center, concern on top right
    // Top left alarm - Already present
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);


    // Then the top two sensors, a lidar and PIR, a second after that
    simulate_time(1);
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 650);
    // After 1 second, the Lidar stops detecting. After 15 seconds, the PIR stops detecting
    simulate_time(1);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2200);
    simulate_time(14);
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

    // Things should decay to concern after about 30 seconds
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    // Decay should finish after a further 30 seconds
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}


// Animal starts in the top left, entering slowly, then sprints upwards towards the road.
static void slow_to_fast_top_left_sprint_up(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Start detection from left end of network. First the PIR sensor
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
    // Possible detection in the top left. Output: Alarm, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 3 seconds later, Lidar (on bottom left node!) picks up the animal.
    simulate_time(3);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 1100);
    // Confirmed detection in top left. Output: Alarm, alarm, concern
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    
    // The sensors stop detecting, after 5 and 18 seconds respectively
    simulate_time(5);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2000);
    simulate_time(13);
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);


    // Animal waits for about a minute. After about 30 seconds, statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    // After another 30 seconds, statuses decay again
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Shortly after, the animal suddenly runs towards the road, passing through the top sensors on nodes (-1,1) and (0,1)
    simulate_time(2);
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    // Possible detection - Alarm, concern, idle (No change for the 1,1 LED)
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    simulate_time(1);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 150);
    // Detection - Alarm, alarm, concern
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Animal has exited the network. Lidar stops detecting 8 seconds later, PIR stops detecting 10 seconds after that
    simulate_time(8);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2500);
    simulate_time(10);
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

    // LED Statuses start to decay after about 30 seconds, then finish decaying
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));

}

// Animal starts in the bottom right, entering slowly, then sprints upwards towards the road.
static void slow_to_fast_bottom_right_sprint_up(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));
    
    // Detection from PIR on 1, -1, sensor rotation 0. 
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Possible detection in bottom right - Output: Alarm, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 5 seconds later, detection from lidar on 1,1, sensor rotation 180 (1500cm away, very edge of the network in the second AV area)
    simulate_time(5);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1500);

    // Detection in bottom right - Output: Alarm, alarm, concern
    // -1, 1 - No change
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // TODO - End of detections

    // Animal stays there for about a minute. After 30 seconds, decay LEDs 1 step
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    // After another 30 seconds, decay LEDs 1 more step
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Animal sprints towards the road, near the center nodes. PIR at 1, 0 picks it up. Lidar at -1, 0 picks it up at about 875cm.
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    // TODO - POssible detections updates - From both bottom right and top right sections!
    simulate_time(1);
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 875);
    //TODO - Confirmed detection updates 

}