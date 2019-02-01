/**
file: app_scheduler.h
brief:
notes:
*/
#ifndef APP_SCHEDULER_H
#define APP_SCHEDULER_H

/* Mock out schedule command, we control events manually. */
/* 0 return code to match NRF_SUCCESS */
#define app_sched_event_put(x,y,z)      ( 0 )

#endif /* APP_SCHEDULER_H */
