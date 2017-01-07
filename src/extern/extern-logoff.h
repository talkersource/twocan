#ifndef EXTERN_LOGOFF_H
#define EXTERN_LOGOFF_H

extern void logoff_player_update(player *);

extern void logoff_real(player *);

extern void player_destroy(player *);

extern int logoff_all(const char *);

extern void user_logoff(player *, const char *);

extern void cmds_init_logoff(void);

#endif
