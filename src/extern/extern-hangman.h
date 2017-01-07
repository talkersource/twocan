#ifndef EXTERN_HANGMAN_H
#define EXTERN_HANGMAN_H

#ifdef TWOCAN_CODE

extern void hangman_clear(player *);

extern void init_hangman(void);

extern void cmds_init_hangman(void);

extern void user_configure_game_hangman_use(player *, const char *);

#endif

#endif
