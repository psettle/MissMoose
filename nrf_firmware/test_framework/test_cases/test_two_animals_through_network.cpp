/**
file: test_slow_to_fast_running_animals.cpp
brief:
notes: Testing scenarios with two animals in the network
*/

/**********************************************************
                       INCLUDES
**********************************************************/

#include "tests.hpp"

/**********************************************************
                       DECLARATIONS
**********************************************************/

static void two_animals_passing_bottom_half(TestOutput& oracle);
static void two_animals_both_bottom_left(TestOutput& oracle);

/**********************************************************
                       DEFINITIONS
**********************************************************/

void test_two_animals_through_network(std::vector<TestCase>& tests)
{
    ADD_TEST(two_animals_passing_bottom_half);
    ADD_TEST(two_animals_both_bottom_left);
}

// One animal starts on the bottom left (to the side), and moments later another animal passing the other way enters the bottom right.
// The animals pass each other and exit the system on opposite sides.
static void two_animals_passing_bottom_half(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Start detection of animal 1 from left end of network. First the PIR sensor
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    // Possible detection in bottom left. Output: Concern, idle, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 2 seconds after that, the PIR on node 1, -1 picks up animal 2
    simulate_time(2);
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Possible detection in bottom right. Output: Alarm, concern, idle. This superceeds the above output in all cases.
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 5 seconds after the first PIR detection, the left lidar detects animal 1
    simulate_time(3);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 270);
    // Detection in bottom left. Output: Alarm, concern, idle. The alarm and idle LEDs remains the same. The center LED
    // will get upgraded to alert because of two concern inputs!
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    // NOTE: The third LED for node 1, 1 remains idle so far!
    
    // A second later - 4 seconds after second PIR detection - The Lidar on the right end (node 1,1) picks up the animal
    simulate_time(1);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 982);
    // Detection in bottom right. Output: Alarm, alarm, concern. Only change is the third LED becomes concern.
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // Some more time passes and the lidars stop detecting - The left lidar first
    simulate_time(2);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 3570);
    simulate_time(1);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 3125);

    // Some time after that the PIRs stop detecting, the right one takes longer to turn off.
    simulate_time(8);
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);
    simulate_time(3);
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    // After a short time, animal 1 (from the left) will pass the center nodes - 0, 0 and 0, -1.
    simulate_time(2);
    test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Lidar - From node 0,1 - detects a moment later
    simulate_time(2);
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1320);

    // Animal 1 keeps moving. However, animal 2 approaches from the other side and keeps the PIR active! The lidar stops detecting:
    simulate_time(3);
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2300);
    // Then after a moment picks up animal 2:
    simulate_time(2);
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1400);

    // Both rear AVs should still be maxed, and so the LEDs shouldn't change!
    // After 4 seconds the lidar stops detecting again.
    simulate_time(4);
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 2303);

    // After 4 more seconds, the PIR stops. 
    simulate_time(4);
    test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    // 1 second after that, Animal 1 has reached the right end of the network. Picked up by PIR then lidar
    simulate_time(1);
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Detection state is still "detection". Have a detection by the PIR: 
    simulate_time(2);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1200);
    // Detection state is still "detection". The Lidar stops detecting:
    simulate_time(3);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 3000);

    //return; // Works to here

    // The right side PIR is still detecting when animal 2 is picked up by the left side PIR
    simulate_time(1);
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    // Shortly after the left side lider picks up animal 2
    simulate_time(2);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 200);

    // Right side PIR stops detecting Animal 1, and left side lidar also stops detecting animal 2
    simulate_time(3);
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 3570);

    // Some time after, the left side PIR stops detecting animal 2.
    simulate_time(8);
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

    // Both animals have exited the network. Things will calm down.
    // After 30 seconds, LED statuses decay.
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    // After 30 further seconds, LED statuses decay again.
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}


// Two animals, one following the other, enter the system from the bottom left and exit from the bottom right.
static void two_animals_both_bottom_left(TestOutput& oracle)
{
    // Start idle for a bit. 
    simulate_time(MINUTES(2));

    // Start detection of animal 1 from left end of network. First the PIR sensor
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_START);
    // Possible detection in bottom left. Output: Concern, idle, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // 4 seconds after that, the lidar on node -1, -1 picks up that animal
    simulate_time(4);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 200);
    // Detection in bottom left - Alarm, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // First animal moves on. After 2 seconds, the lidar stops detecting it. However, the PIR remains active because of movement
    // of the second animal
    simulate_time(2);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2450);

    // After 2 more seconds, the lidar detects the second (following) animal
    simulate_time(2);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 300);

    // After 3 seconds, the lidar stops detecting. After 10 seconds, the PIR stops detecting.
    simulate_time(3);
    test_send_lidar_data(-1, -1, SENSOR_ROTATION_0, 2450);
    simulate_time(7);
    test_send_pir_data(-1, 0, SENSOR_ROTATION_180, PIR_DETECTION_END);

    // After 5 seconds, the central PIR - On node 0, -1 - Will pick up the first animal
    simulate_time(5);
    test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Detection state stays the same in the bottom left AV - Alarm, concern, idle
    // Possible detection in bottom right - Output: Alarm, concern, idle
    // Left LED - Stays alarm. Middle LED becomes alarm because of two concern outputs. Right LED remains idle. 
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_RED);

    // After 2 seconds, the lidar on node 0, 1 will pick that animal up.
    simulate_time(2);
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1350);
    // Detection in bottom left still - Alarm, concern, idle.
    // Detection in bottom right - Alarm, alarm, concern.
    // Left LED - Stays alarm. Middle LED - Stays alarm. Right LED becomes concern.
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);

    // After 2 seconds lidar stops detecting first animal. The second animal is keeping the PIR active.
    simulate_time(2);
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 3743);
    // After 1 second lidar picks up second animal
    simulate_time(1);
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 1237);

    // Both sides in alert status still.
    // After 3 seconds, lidar stops detecting, and after 10, PIR finally stops detecting
    simulate_time(3);
    test_send_lidar_data(0, 1, SENSOR_ROTATION_180, 3741);
    simulate_time(7);
    test_send_pir_data(0, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    // The animals pass through the right side of the network. First, the first animal is detected by the PIR after 2 seconds.
    simulate_time(2);
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_START);
    // Both AVs remain in the detected state, no LED changes. 3 seconds later, the right Lidar on node 1, 1 detects the first animal.
    simulate_time(3);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1322);
    // After another second it stops detecting that animal
    simulate_time(1);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2640);
    // After 3 more seconds it detects the second animal
    simulate_time(3);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 1267);
    // After 4 more seconds it stops detecing that animal
    simulate_time(4);
    test_send_lidar_data(1, 1, SENSOR_ROTATION_180, 2639);

    // After 12 more seconds, the PIR stops detecting that both animals
    simulate_time(12);
    test_send_pir_data(1, -1, SENSOR_ROTATION_0, PIR_DETECTION_END);

    // It's been about 25 seconds since anything affected the bottom left AV. So, after about 5 seconds, that AV should decay.
    // Output was alarm, concern, idle, and becomes concern, idle, idle. However, the right edge is still alarm, alarm, concern
    // So there is no effect on output.
    simulate_time(5);

    // After 30 further seconds, the AVs should decay a step. The bottom left AV shows no detection, the bottom right AV decays.
    // Output becomes concern, concern, idle
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_BLINKING, LED_COLOURS_YELLOW);
    oracle.logLedUpdate(1, 1, LED_FUNCTION_LEDS_OFF);
    // After 30 further seconds, LED statuses decay again.
    simulate_time(30);
    oracle.logLedUpdate(-1, 1, LED_FUNCTION_LEDS_OFF);
    oracle.logLedUpdate(0, 1, LED_FUNCTION_LEDS_OFF);

    // Let things settle
    simulate_time(MINUTES(10));
}