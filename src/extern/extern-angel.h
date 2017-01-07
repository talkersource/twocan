#ifndef EXTERN_ANGEL_H
#define EXTERN_ANGEL_H

extern int total_crashes;
extern time_t angel_started;

extern void angel_socket_do(void);

extern void init_angel(void);

extern void user_su_shutdown_angel(player *, const char *);

extern void cmds_init_angel(void);

#endif
