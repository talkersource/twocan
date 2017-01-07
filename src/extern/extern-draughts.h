#ifndef EXTERN_DRAUGHTS_H
#define EXTERN_DRAUGHTS_H

/* for checking things */
extern const char *get_game_type(player *, int);
extern int draughts_is_playing_game(player *);
extern int draughts_is_watching_game(player *);

/* for quitting */
extern void draughts_stop_watching_game(player *, int);
extern void draughts_lose_game(player *, int);

/* for ingnore */
extern void check_if_on_draughts_game(player *, player *);

extern void cmds_init_draughts(void);

extern void user_configure_game_draughts_use(player *, const char *);


#endif
