#ifndef EXTERN_PLAYER_FIND_H
#define EXTERN_PLAYER_FIND_H

extern unsigned int player_find_msg_add(int (*)(int, player *,
                                                const char *, const char *));
extern void player_find_msg_del(unsigned int);

extern player *player_find_local(player *, const char *, int);
extern player *player_find_on(player *, const char *, int);
extern player_tree_node *player_find_all(player *, const char *, int);
extern player *player_find_load(player *, const char *, int);

extern void init_player_find(void);

#endif
