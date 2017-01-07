#ifndef EXTERN_PLAYER_TREE_H
#define EXTERN_PLAYER_TREE_H

extern void player_tree_add(player_tree_node *new_node);
extern void player_newbie_add(player_tree_node *new_newbie);

extern player_tree_node *player_tree_find_exact(const char *);
extern player_tree_node *player_tree_find(const char *);

extern player_tree_node *player_newbie_start(void);
extern player_tree_node *player_newbie_find_exact(const char *);
extern player_tree_node *player_newbie_find(const char *);

extern void player_newbie_del(player_tree_node *);
extern void player_tree_del(player_tree_node *old_node);

extern int player_newbie_number(void);

extern void do_inorder_all(void (*command) (player_tree_node *,
                                                va_list),  ...);
extern void do_inorder_all_load(void (*command) (player *, va_list),  ...);

extern void player_tree_all_destroy(void);

extern int player_file_remove(const char *);

extern player_tree_node *player_tree_random(void);

#endif

