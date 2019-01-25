/**
file: mm_activity_variables.h
brief:
notes:
    x is indexed left -> right. (In direction of North-American traffic)
    y is indexed top -> bottom. (Moving away from the road)
*/
#ifndef MM_ACTIVITY_VARIABLES_H
#define MM_ACTIVITY_VARIABLES_H

/**********************************************************
                        INCLUDES
**********************************************************/

#include <stdint.h>

/**********************************************************
                        CONSTANTS
**********************************************************/

#define MAX_AV_SIZE_X           ( MAX_GRID_SIZE_X - 1 )
#define MAX_AV_SIZE_Y           ( MAX_GRID_SIZE_Y - 1 )
#define ACTIVITY_VARIABLES_NUM  ( MAX_AV_SIZE_X * MAX_AV_SIZE_Y )

/**********************************************************
                        MACROS
**********************************************************/

/*
   Ease of access macro, optional usage:
   ex.
   mm_av_multiply(AV_TOP_LEFT, factor);
   val = mm_av_access(AV_TOP_LEFT);
   mm_av_set(AV_TOP_LEFT, val);
   
   */
#define AV_TOP_LEFT             0, 0
#define AV_TOP_RIGHT            1, 0
#define AV_BOTTOM_LEFT          0, 1
#define AV_BOTTOM_RIGHT         1, 1

/**********************************************************
                        TYPES
**********************************************************/

typedef float mm_activity_variable_t;

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * Initialize activity variables.
 */
void mm_activity_variables_init(void);

/**
 * Modify activity variable value, automatically manages upper and lower limits.
 */
void mm_av_multiply(uint8_t x, uint8_t y, float factor);

/**
 * Read-only activity variable access.
 */
mm_activity_variable_t mm_av_access(uint8_t x, uint8_t y);

/**
 * Set activity variable value, automatically manages upper and lower limits.
 */
void mm_av_set(uint8_t x, uint8_t y, mm_activity_variable_t val);

#endif /* MM_ACTIVITY_VARIABLES_H */