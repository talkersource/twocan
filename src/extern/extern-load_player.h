#ifndef EXTERN_LOAD_PLAYER_H
#define EXTERN_LOAD_PLAYER_H

extern Timer_q_base load_player_timer_queue;

extern void init_players(void);

extern int player_load(player_tree_node *);
extern player_tree_node *player_load_saved(const char *);
extern void player_load_cleanup(player *);
extern void player_load_timer_start(player_tree_node *);


#endif
