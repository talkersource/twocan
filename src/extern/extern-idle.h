#ifndef EXTERN_IDLE_H
#define EXTERN_IDLE_H

extern void idle_check_increment_total(player *);

extern void idle_timer_start(player *);
extern void idle_timer_stop(player *);

extern void init_idle(void);

extern void cmds_init_idle(void);

#endif
