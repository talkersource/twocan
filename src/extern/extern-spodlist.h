#ifndef EXTERN_SPODLIST_H
#define EXTERN_SPODLIST_H

extern void init_spodlist(void);

extern void spodlist_addin_player(player_tree_node *);
extern void spodlist_remove_player(const char *);

extern void spodlist_check_order(player_tree_node *, unsigned int, int);

extern void cmds_init_spodlist(void);


#endif
