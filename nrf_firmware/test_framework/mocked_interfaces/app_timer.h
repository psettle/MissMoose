/**
file: app_timer.h
brief:
notes:
*/
#ifndef APP_TIMER_H
#define APP_TIMER_H

/* App timer is not used, instead we manually trigger second events to simulate time.
   We need to mock these out so the algorithm *thinks* it lauched the timers
   when really we are controlling time. */
#define APP_TIMER_DEF(x)
/* 0 return codes to match NRF_SUCCESS */
#define app_timer_create(x,y,z)      ( 0 )
#define app_timer_start(x,y,z)       ( 0 )

#endif /* APP_TIMER_H */