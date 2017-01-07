#ifndef EXTERN_PLAYER_LINKED_LIST_H
#define EXTERN_PLAYER_LINKED_LIST_H

#define PLAYER_LINK_NEXT(x) ((x)->next)
#define PLAYER_LINK_PREV(x) (((player_linked_double_list *)(x))->prev)
#define PLAYER_LINK_GET(x) ((x)->loggedon ? \
 (x)->this.loggedon : (x)->this.saved->player_ptr)
#define PLAYER_LINK_SAV_GET(x) ((x)->loggedon ? \
 (x)->this.loggedon->saved : (x)->this.saved)

extern player_linked_list *player_link_find(player_linked_list *,
                                            player_tree_node *, player *,
                                            unsigned int);
extern player_linked_list *player_link_add(player_linked_list **,
                                           player_tree_node *, player *,
                                           unsigned int, player_linked_list *);
extern int player_link_del(player_linked_list **,
                           player_tree_node *, player *,
                           unsigned int, player_linked_list *);

extern void player_list_alpha_add(player_tree_node *);
extern void player_list_alpha_del(player_tree_node *);
extern player_linked_list *player_list_alpha_start(void);

extern void player_list_cron_add(player *);
extern void player_list_cron_del(player *);
extern player_linked_list *player_list_cron_start(void);

extern void player_list_logon_staff_add(player_tree_node *);
extern void player_list_logon_staff_del(player_tree_node *);
extern player_linked_list *player_list_logon_staff_start(void);
extern int player_list_logon_staff_number(void);

extern void player_list_io_add(player *);
extern void player_list_io_del(player *);
extern player_linked_list *player_list_io_start(void);
extern player_linked_list *player_list_io_find(player *);

/* staff and spod lists */
extern void player_list_perm_staff_add(player_tree_node *);
extern void player_list_perm_staff_del(player_tree_node *);
extern player_linked_list *player_list_perm_staff_start(void);

extern void player_list_spod_add(player_tree_node *);
extern void player_list_spod_del(player_tree_node *);
extern player_linked_list *player_list_spod_start(void);

/* other stuff */
extern player *find_player_cronlist_exact(const char *);

#endif
