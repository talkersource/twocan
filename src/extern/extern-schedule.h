#ifndef EXTERN_SCHEDULE_H
#define EXTERN_SCHEDULE_H

#define SCHEDULE_CODE_GENERIC_START() do { \
 struct timeval before; struct timeval after; long internal_local_tmp = 0; \
  if (gettimeofday(&before, NULL)) { assert(FALSE); } internal_local_tmp = 0

#define SCHEDULE_CODE_GENERIC_END(y) \
  if (gettimeofday(&after, NULL)) { assert(FALSE); } \
  internal_local_tmp = timeval_diff_time(&after, &before); \
  timeval_add_useconds(y, internal_local_tmp); \
} while (FALSE)

extern void schedule_timer_start(player *p, struct timeval *);
extern void schedule_timer_end(player *p, struct timeval *);
extern int schedule_can_go(player *p);


#endif
