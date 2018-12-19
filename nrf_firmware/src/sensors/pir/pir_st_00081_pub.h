/**
 *
 */
#ifndef PIR_ST_00081_PUB_H
#define PIR_ST_00081_PUB_H

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************
                       DECLARATIONS
**********************************************************/
/**
 * @brief Function for initializing the wide-angle PIR sensor.
 */
void pir_st_00081_init(void);

/**
 * @brief Function for disabling the wide-angle PIR sensor.
 */
void pir_st_00081_disable(void);
/**
 * @brief Function for enabling the wide-angle PIR sensor.
 */
void pir_st_00081_enable(void);

#ifdef __cplusplus
}
#endif

#endif /* PIR_ST_00081_PUB_H */

