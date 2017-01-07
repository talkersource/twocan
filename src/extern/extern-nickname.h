#ifndef EXTERN_NICKNAME_H
#define EXTERN_NICKNAME_H

extern void nickname_cleanup(player *);

extern void nickname_load(player *, file_io *);
extern void nickname_save(player *, file_io *);

extern player_tree_node *nickname_player_tree_find(player *, const char *);
extern player *nickname_player_find(player *, const char *);

extern void cmds_init_nickname(void);

#endif
