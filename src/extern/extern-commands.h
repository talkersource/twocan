#ifndef EXTERN_COMMANDS_H
#define EXTERN_COMMANDS_H

extern Timer_q_base scripting_timer_queue;

extern void user_examine(player *, const char *);
extern void user_finger(player *p, const char *);
extern void user_uptime(player *p, const char *);

extern void init_commands(void);

extern void cmds_init_commands(void);

#endif
