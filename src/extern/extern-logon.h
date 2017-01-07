#ifndef EXTERN_LOGON_H
#define EXTERN_LOGON_H

/* global variables... */
extern int logging_onto_count;

extern Timer_q_base logon_timer_queue;

extern player *player_create(void);

extern void logon_player_make(player *, const char *, const char *);

extern void logon_shortcut_logon_start(player *);

extern void logon_start(player *);

extern void logon_inform(player *, const char *);

extern void init_logon(void);

#endif
