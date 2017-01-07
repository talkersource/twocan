#ifndef IDLE_H
#define IDLE_H

/* how long players are allowd to idle before they gain idle logon time */
#define IDLE_TIME_PERMITTED MK_MINUTES(5)

/* the maximum ammount of time between an idle check
 * without this a player can...
 *   set an idle message;
 *   get checked by the computer, to be checked again in 50 (say) mins
 *   then do something (which removes the idle message)
 *   then idle for 50 mins,
 *    when they will get checked and be booted imediatly */
/* use it to update the prompt every now and then as well */
#define IDLE_TIME_STEP_MAX 30

#define IDLE_TIME_NOIDLE_WARN MK_MINUTES(35)
#define IDLE_TIME_NOIDLE_KICK MK_MINUTES(45)
#define IDLE_TIME_IDLE_WARN MK_MINUTES(110)
#define IDLE_TIME_IDLE_KICK MK_MINUTES(120)

#endif
