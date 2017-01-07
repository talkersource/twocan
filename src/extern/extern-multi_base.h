#ifndef EXTERN_MULTI_BASE_H
#define EXTERN_MULTI_BASE_H

#ifdef MULTI_BASE_C
# define VALID_MULTI(x) (multis_start && multis_end && \
                         ((x) >= multis_start->number) && \
                          ((x) <= multis_end->number))
#endif

/* #define SHOULD_SEE_MULTI(y) (!((y) & (MULTI_TMP_BLOCK | MULTI_BLOCKED |\
				      MULTI_COMPLETE_IGNORE))) */
#define IS_NEW_MULTI(x) ((x)->error_number == MULTI_CREATED)

extern void init_multis(void);
extern void multis_init_for_player(player_tree_node *);

extern multi_node *multi_find_entry(const player_tree_node *, unsigned int);

extern multi_return *multi_add(player_tree_node *, char *,
                               unsigned int, unsigned int);
extern const char *multi_get_names(unsigned int multi, multi_node *,
				   player_tree_node *,
                                   multi_node *, multi_node *, int);
extern const char *multi_get_number(multi_node *);

extern multi_node *do_inorder_multi(int (*) (multi_node *, va_list),
                                    unsigned int, unsigned int, ...);

extern void multi_cleanup(unsigned int, unsigned int);

extern multi_node *multi_get_node_with_flag(unsigned int, unsigned int);
extern unsigned int multi_check_for_flag(unsigned int, unsigned int);
extern unsigned int multi_check_node_for_flag(multi_node *, unsigned int);
extern int multi_compare(unsigned int, unsigned int);
extern unsigned int multi_count_players(unsigned int);

#define CHECK_MULTI_FOR_FLAG_ONLY_AM(x, y) \
(check_multi_for_flag(x, y) && \
 !check_multi_for_flag(x, MULTI_GROUP_FLAGS_AM & ~(y)))
#define CHECK_MULTI_FOR_FLAG_ONLY_TO(x, y) \
   (check_multi_for_flag(x, y) && \
    !check_multi_for_flag(x, MULTI_GROUP_FLAGS_TO & ~(y)))
#define CHECK_MULTI_FOR_FLAG_ONLY(x, y) \
   (check_multi_for_flag(x, y) && \
    !check_multi_for_flag(x, MULTI_GROUP_FLAGS_ALL & ~(y)))
   
#define CHECK_NODE_FOR_FLAG_ONLY_AM(x, y) \
   (check_node_for_flag(x, y) && \
    !check_node_for_flag(x, MULTI_GROUP_FLAGS_AM & ~(y)))
#define CHECK_NODE_FOR_FLAG_ONLY_TO(x, y) \
   (check_node_for_flag(x, y) && \
    !check_node_for_flag(x, MULTI_GROUP_FLAGS_TO & ~(y)))
#define CHECK_NODE_FOR_FLAG_ONLY(x, y) \
   (check_node_for_flag(x, y) && \
    !check_node_for_flag(x, MULTI_GROUP_FLAGS_ALL & ~(y)))
   
extern void multis_destroy_for_player(player_tree_node *);

extern void cmds_init_multi_base(void);

#endif
