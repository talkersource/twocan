#ifndef SCHEDULE_H
#define SCHEDULE_H

#ifndef SCHED_DEBUG
# ifndef NDEBUG
#  define SCHED_DEBUG 1
# else
#  define SCHED_DEBUG 0
# endif
#endif

/* 2 second timeout... don't want to take it higher or running lots of
 * commands will be a pain */
#if SCHED_DEBUG
# define SCHEDULE_POLL_TIMEOUT 20000 /* stupid... 20 seconds */
#else
# define SCHEDULE_POLL_TIMEOUT 500
#endif

#define SCHEDULE_POLL_SHUTDOWN_TIMEOUT 100

#ifdef SCHEDULE_C

/* on a p133 with 64 Meg Ram, in X */
/* pressing return is about 1345 */
/* pressing return in the editor is about 850 */

# define SCHEDULE_ADMIN_PASS    ((unsigned long) \
  500000) /* half a second */
# define SCHEDULE_STAFF_PASS    ((unsigned long) \
  250000)
# define SCHEDULE_SPOD_PASS     ((unsigned long) \
  125000)
/* residents have to wait 1 second ... newbies 2 seconds ... see schedule.c */
# define SCHEDULE_RESIDENT_PASS ((unsigned long) \
 (250000 / (current_players + 1)))
# define SCHEDULE_NEWBIE_PASS   ((unsigned long) \
 (125000 / (current_players + 1)))

# define SCHEDULE_MAX_AT_ONCE   ((unsigned long) \
 5 * 1000000)
# define SCHEDULE_ABOVE_ONE_INC   ((unsigned long) \
   50000)

#endif
   
#endif
