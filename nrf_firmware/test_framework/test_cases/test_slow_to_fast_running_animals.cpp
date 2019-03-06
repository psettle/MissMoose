/**
file: test_slow_to_fast_running_animals.cpp
brief:
notes: Idea is to test the behaviour of animals that start moving slowly, then suddenly start moving quickly
(For example - They are suddenly startled) in a different direction
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
static void slow_to_fast_top_right_sprint_up(TestOutput& oracle);

static void slow_to_fast_top_left_sprint_down(TestOutput& oracle);
static void slow_to_fast_top_right_sprint_down(TestOutput& oracle);
static void slow_to_fast_bottom_left_sprint_down(TestOutput& oracle);
static void slow_to_fast_bottom_right_sprint_down(TestOutput& oracle);


/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_slow_to_fast_running_animals(std::vector<TestCase>& tests)
{
    ADD_TEST(slow_to_fast_bottom_left_sprint_up);
    ADD_TEST(slow_to_fast_top_left_sprint_up);
    ADD_TEST(slow_to_fast_bottom_right_sprint_up);
    ADD_TEST(slow_to_fast_top_right_sprint_up);

    ADD_TEST(slow_to_fast_top_left_sprint_down);
    ADD_TEST(slow_to_fast_top_right_sprint_down);
    ADD_TEST(slow_to_fast_bottom_left_sprint_down);
    ADD_TEST(slow_to_fast_bottom_right_sprint_down);
}

// Animal starts in the bottom left, entering slowly, then sprints upwards towards the road.
static void slow_to_fast_bottom_left_sprint_up(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Start detection from left end of network. First the PIR sensor
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    // Possible detection in bottom left. This causes a concern output in the top left (-1,1) LED
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);


    // 4 seconds later - Lidar detects the same animal
    simulate_time(4);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 400);
    // Detection in bottom left. Alarm in top left (-1, 1), concern in middle-left (0,1) 
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
    // Possible detection in top left quadrant - Alarm on top left, concern on top center, and idle on top right
    // Also possible detection in bottom left quadrant - Concern, idle, idle (No changes to above output, already at max alert on relevant nodes)
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Then the LIDAR after a second
    simulate_time(1);
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 700);
    // Confirmed detection in top left - Alarm, alarm, concern
    // Confirmed detection in bottom left - Alarm, concern, idle (No changes to above output, already at max alert on relevant nodes)
    // Top left alarm - Already present
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);


    // Then the top two sensors, a lidar and PIR, a second after that
    simulate_time(1);
    test_send_pir_data(0, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 650);
    // After 1 second and 3 seconds, the Lidars stop detecting. After 13 and 15 seconds, the PIRs stop detecting
    simulate_time(1);
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 3250);
    simulate_time(2);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 2200);
    simulate_time(10);
    test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
    simulate_time(2);
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
    simulate_time(30);
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

    // End of detections. After 2 second, lidar ends. After 9 further seconds, PIR detection ends.
    simulate_time(2);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 3800);
    simulate_time(9);
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    // Animal stays there for about a minute. After 30 seconds, decay LEDs 1 step
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    // After another 30 seconds, decay LEDs 1 more step
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Animal sprints towards the road, near the center nodes. PIR at 1, 0 picks it up. 
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    // Possible detection in bottom right - Alarm, concern, idle
    // Possible detection in top right - Alarm, alarm, concern
    // Combined result: Alarm, alarm, concern
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Lidar at -1, 0 picks it up at about 875cm.
    simulate_time(1);
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 875);
    // Confirmed detection in bottom right - Alarm, alarm, concern
    // Confirmed detection in top right - Alarm, alarm, alarm
    // Only the top right LED changes
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    // After one second, the LIDAR stops picking up the fast moving animal.
    simulate_time(1);
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 2400);
    // After another second, the upper nodes detect the animal - It exits the network. Lidar at -1, 1, and PIR at 1, 1.
    simulate_time(1);
    test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1450);
    // Already in full alarm status, no change.

    // The remaining 3 sensors stop detecing the animal. The Lidar after 3 seconds, the first PIR after 7 further, 
    // the second PIR after 5 further
    simulate_time(1);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 3090);
    simulate_time(7);
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
    simulate_time(5);
    test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

    // Animal has left the network - Alert statuses should decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}

// Animal starts in the top right, entering slowly, then sprints upwards towards the road.
static void slow_to_fast_top_right_sprint_up(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Detection from PIR on 1, 0, sensor rotation 0. 
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Possible detection in top right - Output: Alarm, alarm, concern
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 7 seconds later, detection from lidar on 1,1, sensor rotation 180, near top of network
    simulate_time(7);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 50);
    // Detection in top right - Output: Alarm, alarm, alarm
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    // After 3 and 17 seconds, the Lidar and PIR sensors stop detecting
    simulate_time(3);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2100);
    simulate_time(14);
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

    // Animal remains in network for a minute - After 30 seconds, alert statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    // After 30 more seconds, concern statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    // After a while longer, the animal sprints towards the road (Startled)
    simulate_time(17);
    // The PIR at 1,1, rotation 270 will pick up the animal
    test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_START);
    // Possible detection in top right. Output: Alert, alert, concern
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 3 seconds later, the Lidar at -1, 1 will pick up the animal.
    simulate_time(3);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 1450);
    // Confirmed detection, output: Alert, alert, alert
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    // Sensors stop detecting after 2 (lidar) and 8 (pir) further seconds
    simulate_time(2);
    test_send_lidar_data(-1, 1, SENSOR_ROTATION_90, 3500);
    simulate_time(6);
    test_send_pir_data(1, 1, SENSOR_ROTATION_270, PIR_DETECTION_END);

    // Animal has exited the network. After 30 seconds, alert status decays. After 30 further seconds, concern status decays.
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}


// Animal starts in the top left, entering slowly, then sprints downwards, away from the road.
static void slow_to_fast_top_left_sprint_down(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Start detection from top left end of network. First the PIR sensor at -1, 1
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_START);
    // Possible detection in the top left. Output: Alarm, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 7 seconds later, Lidar on the -1,-1 node picks up the animal.
    simulate_time(7);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 1372);
    // Confirmed detection in top left. Output: Alarm, alarm, concern
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // The sensors stop detecting, after 3 and 10 seconds respectively
    simulate_time(3);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 3753);
    simulate_time(7);
    test_send_pir_data(-1, 1, SENSOR_ROTATION_180, PIR_DETECTION_END);


    // Animal waits for about a minute. After about 30 seconds, statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    // After another 30 seconds, statuses decay again
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Some time later, the animal bolts downwards, away from the road
    simulate_time(45);
    // PIR at 0,0 picks up animal suddenly:
    test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    // Possible detection in top left - Alarm, concern, idle
    // Possible detection in bottom left - Concern, idle, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Lidar at -1, 0 picks up animal 2 seconds later
    simulate_time(2);
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 458);
    // Detection in top left - Alarm, alarm, concern
    // Detection in bottom left - Alarm, concern, idle. Overall: Alarm, alarm, concern.
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Bottom sensors pick up animal
    simulate_time(1);
    test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    simulate_time(1);
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1300);

    // Animal has left network. Sensors stop detecting it. At this point the top lidar will have stopped detecting.
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 3800);
    simulate_time(3);
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2400);
    simulate_time(7);
    test_send_pir_data(0, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
    simulate_time(4);
    test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

    // Animal has left network. After 30 seconds, statuses decay.
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    // Finish decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}

// Animal starts in the top right, entering slowly, then sprints downwards, away from the road.
static void slow_to_fast_top_right_sprint_down(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Detection from PIR on 1, 0, sensor rotation 0. 
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Possible detection in top right - Output: Alarm, alarm, concern
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 2 seconds later, detection from lidar on 1,1, sensor rotation 180
    simulate_time(2);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 641);
    // Detection in top right - Output: Alarm, alarm, alarm
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    // After 5 and 12 seconds, the Lidar and PIR sensors stop detecting
    simulate_time(5);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1700); //TODO - Make sure this is outside of range. 800 CM + 800 CM = 1600 CM?
    simulate_time(7);
    test_send_pir_data(1, 0, SENSOR_ROTATION_0, PIR_DETECTION_END);

    // Animal remains in network for a minute - After 30 seconds, alert statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    // After 30 more seconds, concern statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    // Some time later, the animal bolts away from the road and gets detected by the sensors on node -1,0 (lidar) and 1,0(PIR)
    simulate_time(10);
    // Detected by the PIR
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_START);
    // Possible detection in bottom right - Alarm, concern, idle
    // Possible detection in top right - Alarm, alarm, concern
    // Combined result: Alarm, alarm, concern
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Lidar at -1, 0 picks it up at about 1125cm.
    simulate_time(1);
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 1125);
    // Detection in bottom right - Alarm, alarm, concern
    // Detection in top right - Alarm, alarm, alarm
    // Only the top right LED changes
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    // The bottom PIR at 0, -1 also detects the animal
    simulate_time(1);
    test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    // Detection already confirmed, no change. The lidar at 1, -1 detects it a moment after that. 
    simulate_time(2);
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 500);
    // The lidar at -1, 0 will also stop detecting at this time
    test_send_lidar_data(-1, 0, SENSOR_ROTATION_90, 3400);

    // The animal has left the network. After 4 more seconds, the 1, -1 lidar stops detecting.
    simulate_time(4);
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2770);
    // Shortly after, the PIRs at stop detecting
    simulate_time(6);
    test_send_pir_data(1, 0, SENSOR_ROTATION_270, PIR_DETECTION_END);
    simulate_time(2);
    test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

    // After 30 seconds, alert statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    // After 30 more seconds, concern statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}

// Animal starts in the bottom left, entering slowly, then sprints downwards, away from the road.
static void slow_to_fast_bottom_left_sprint_down(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Start detection from left end of network. First the -1,0 PIR sensor at rotation 180
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    // Possible detection in bottom left. Output: Concern, idle, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);


    // 2 seconds later - -1, -1 Lidar detects the same animal
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 150);
    simulate_time(2);
    // Detection in bottom left. Output: Alarm, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // About 5 seconds later, the lidar stops detecting. About 12 seconds after that, the PIR stops detecting.
    // Note on Lidar - Anything from about 2000 to 4000 should be an 'end of detection'
    simulate_time(5);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 3500);
    simulate_time(12);
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

    // Animal moves slowly, takes a minute in that AV region. After about 30 seconds, the alert statuses start decaying
    // and after a minute they finish decaying
    simulate_time(30);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

    // Animal sprints downwards after a short time
    simulate_time(5);
    // Detected by the -1, -1 PIR
    test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    // Possible detection in bottom left. Output: Concern, idle, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    // After 2 seconds, the lidar at 1, -1 detects the animal.
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 1387);
    // Detection in bottom left. Output: Alarm, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Sensors stop detecting animal after a few seconds - 3 for lidar, 12 for PIR
    simulate_time(3);
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 3980);
    simulate_time(9);
    test_send_pir_data(-1, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

    // After 30 seconds, alert and concern statuses decay
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);
    // After a further 30 seconds, the remaining concern status decays
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}


// Animal starts in the bottom right, entering slowly, then sprints downwards, away from the road.
static void slow_to_fast_bottom_right_sprint_down(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Detection from PIR on 1, -1, sensor rotation 0. 
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Possible detection in bottom right - Output: Alarm, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 1 second later, detection from lidar on 1,1, sensor rotation 180
    simulate_time(1);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1000);
    // Detection in bottom right - Output: Alarm, alarm, concern
    // -1, 1 - No change
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // End of detections. After 5 second, lidar ends. After 14 further seconds, PIR detection ends.
    simulate_time(5);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 3800);
    simulate_time(14);
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    // Animal stays there for about a minute. After 30 seconds, decay LEDs 1 step
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);

    // After another 30 seconds, decay LEDs 1 more step
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Animal sprints away from road. PIR at 0, -1 picks it up.
    test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_START);
    // Possible detection in bottom right - Output: Alarm, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    
    // After a second, the lidar at 1, -1 picks up the animal
    simulate_time(1);
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 248);
    // Detection in bottom right - Output: Alarm, alarm, concern
    // -1, 1 - No change
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Animal has left the network. After 3 seconds, lidar stops detecting. After 13 seconds, PIR stops detecting
    simulate_time(3);
    test_send_lidar_data(1, -1, SENSOR_ROTATION_270, 2480);
    simulate_time(10);
    test_send_pir_data(0, -1, SENSOR_ROTATION_90, PIR_DETECTION_END);

    // After 30 seconds, LED indications start to decay
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    // After a 30 further seconds, LED indications finish decaying
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}