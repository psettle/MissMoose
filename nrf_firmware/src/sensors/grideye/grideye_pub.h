/**
 *
 */
#ifndef GRIDEYE_PUB_H
#define GRIDEYE_PUB_H

#ifdef __cplusplus
extern "C" {
#endif


/**********************************************************
                        INCLUDES
**********************************************************/

/**********************************************************
                        CONSTANTS
**********************************************************/

/**********************************************************
                       DECLARATIONS
**********************************************************/

/**
 * @brief Enum for conveniently describing the framerate.
 */
typedef enum
{
    GRIDEYE_10FPS,
    GRIDEYE_1FPS,
} grideye_framerate_t;

/**********************************************************
                       DEFINITIONS
**********************************************************/

/**
 * @brief
 */
void grideye_update_main(void);

/**
 * @brief
 */
void grideye_set_framerate(grideye_framerate_t framerate);

/**
 * @brief
 *
 */
void grideye_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GRIDEYE_PUB_H */