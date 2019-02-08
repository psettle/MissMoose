/**
file: app_error.h
brief: Main purpose is to imitate APP_ERROR_CHECK.
notes: 
*/
#ifndef APP_ERROR_H
#define APP_ERROR_H

/**********************************************************
INCLUDES
**********************************************************/

#include <stdint.h>

/**********************************************************
CONSTANTS
**********************************************************/

/* Mock out error checking. If the error code isn't zero, handle the error. */
#define APP_ERROR_CHECK(cond) if(cond) { handle_error(__LINE__, __FILE__, cond); }

/**********************************************************
DECLARATIONS
**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void handle_error(int line, const char* file, uint32_t code);

#ifdef __cplusplus
}
#endif
#endif /* APP_ERROR_H */