#ifndef EXTERN_SHUTDOWN_H
#define EXTERN_SHUTDOWN_H

extern char shutdown_player[PLAYER_S_NAME_SZ];
extern char shutdown_msg[SHUTDOWN_MSG_SZ];

extern player *current_player; /* used for logging purposes */

extern void shutdown_do_timer(void);

extern void shutdown_error(const char *, ... )
    __attribute__ ((__format__ (printf, 1, 2))) __attribute__ ((__noreturn__));

extern void shutdown_exit(void);

extern void init_shutdown(void);

extern void cmds_init_shutdown(void);

#endif
