#ifndef EXTERN_TEXT_OBJS_H
#define EXTERN_TEXT_OBJS_H

extern text_objs_node *text_objs_user_find_id(player_tree_node *, int);
extern int text_objs_add(player_tree_node *, const char *, size_t, int);
extern void text_objs_del_id(int);
extern void text_objs_del_type(int);
extern void text_objs_cleanup(int);

extern void init_text_objs(void);


#endif
