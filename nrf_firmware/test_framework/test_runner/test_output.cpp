/**
file: test_output.cpp
brief:
notes:
*/

/**********************************************************
                        INCLUDES
**********************************************************/

#include "test_output.hpp"
#include "simulate_time.hpp"

#include <map>


/**********************************************************
                       CONSTANTS
**********************************************************/

#define ON_TIME_WEIGHT  ( 1.00f )
#define OFF_TIME_WEIGHT ( 4.00f )   /* Being off at the right times is much easier than being on at the right times, so it counts for more. */

/**********************************************************
                       DEFINITIONS
**********************************************************/

void TestOutput::initOracle(std::vector<mm_node_position_t const *>& outputNodes)
{
    /* Fetch top 3 positions */
    outputNodes.push_back(get_node_for_position(-1, 1));
    outputNodes.push_back(get_node_for_position(0, 1));
    outputNodes.push_back(get_node_for_position(1, 1));

    /* Put an idle event onto the oracle for each */
    for (auto node : outputNodes)
    {
        if (node != NULL)
        {
            logLedUpdate(
                LedUpdate{
                    0,
                    node->node_id,
                    LED_FUNCTION_LEDS_OFF,
                    LED_COLOURS_RED    /* Colour is irrelevant since the led is off. */
                }
            );
        }
    }
}

void TestOutput::logLedUpdate
(
    int8_t x,
    int8_t y,
    led_function_t  ledFunctionM,
    led_colours_t   ledColourM
)
{
    logLedUpdate(
        LedUpdate{
            get_simulated_time_elapsed(),
            get_node_for_position(x, y)->node_id,
            ledFunctionM,
            ledColourM
        }
    );
}

void TestOutput::logLedUpdate(LedUpdate const & update)
{
    ledUpdatesM.push_back(update);
}

float TestOutput::getMatchScore(TestOutput const & result, TestOutput const & oracle)
{
    uint32_t max_correct_on_time = 0;
    uint32_t max_correct_off_time = 0;

    uint32_t correct_on_time = 0;
    uint32_t correct_off_time = 0;

    std::map<uint16_t, LedUpdate const *> correctOutput;
    std::map<uint16_t, LedUpdate const *> currentOutput;

    uint32_t resultIt = 0;
    uint32_t oracleIt = 0;

    /* Run through the oracle output second by second */
    for(uint32_t t = 0; true; ++t)
    {
        /* If there are no more led update events to process then the comparison is over. */
        if (resultIt >= result.ledUpdatesM.size() && oracleIt >= oracle.ledUpdatesM.size())
        {
            break;
        }

        /* Check for updates to output: */
        while (resultIt < result.ledUpdatesM.size())
        {
            auto const & resultTip = result.ledUpdatesM[resultIt];
            if (resultTip.time_s <= t)
            {
                currentOutput[resultTip.targetNodeIdM] = &resultTip;
                resultIt++;
            }
            else
            {
                break;
            }
        }

        /* Check for updates to oracle: */
        while (oracleIt < oracle.ledUpdatesM.size())
        {
            auto const & oracleTip = oracle.ledUpdatesM[oracleIt];
            if (oracleTip.time_s <= t)
            {
                correctOutput[oracleTip.targetNodeIdM] = &oracleTip;
                oracleIt++;
            }
            else
            {
                break;
            }
        }

        /* Compare currentOutput to correct output. */
        for (auto const & it : correctOutput)
        {
            /* There is the potential of gaining either an on point or an off point here  */
            if (it.second->ledFunctionM > LED_FUNCTION_LEDS_OFF)
            {
                max_correct_on_time++;
            }
            else
            {
                max_correct_off_time++;
            }

            /* Check if this node is defined in current output: */
            if (currentOutput.count(it.first) == 0)
            {
                /* The test hasn't controlled one of the leds it should be controlling, no points awarded. */
                continue;
            }

            if (it.second->ledFunctionM > LED_FUNCTION_LEDS_OFF)
            {
                /*  The output is supposed to be on,
                    Check if the output function and colour match: */
                if (it.second->ledFunctionM == currentOutput[it.first]->ledFunctionM &&
                    it.second->ledColourM == currentOutput[it.first]->ledColourM)
                {
                    /* result shows the correct output and function while the led is supposed to be on, award an on point. */
                    correct_on_time++;
                }
            }
            else
            {
                /* The output is supposed to be off, check if the function matches: */
                if (LED_FUNCTION_LEDS_OFF == currentOutput[it.first]->ledFunctionM)
                {
                    /* result shows the correct function while the led is supposed to be off, award an off point. */
                    correct_off_time++;
                }
            }
        }
    }

    /* Calculate correct on/off fractions.  */
    float correct_on_f = (correct_on_time / (float)max_correct_on_time);
    float correct_off_f = (correct_off_time / (float)max_correct_off_time);

    /* Calculate final score */
    float score = std::pow(correct_on_f, ON_TIME_WEIGHT) * std::pow(correct_off_f, OFF_TIME_WEIGHT);

    return score;
}