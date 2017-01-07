#ifndef EXTERN_TTT_H
#define EXTERN_TTT_H

extern void user_toggle_ttt_brief(player *, const char *);
extern void user_ttt_quit(player *);

extern void cmds_init_ttt(void);

extern void user_configure_game_ttt_use(player *, const char *);

#endif
