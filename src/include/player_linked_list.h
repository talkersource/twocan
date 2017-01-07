#ifndef PLAYER_LINKED_LIST_H
#define PLAYER_LINKED_LIST_H

#define PLAYER_LINK_DEFAULT 0
#define PLAYER_LINK_NAME_ORDERED 1
#define PLAYER_LINK_LOGGEDON 2
#define PLAYER_LINK_DOUBLE 4

#define PLAYER_LINK_BAD_FLAGS (~(1 | 2 | 4))

typedef union player_linked_saved_loggedon
{
 struct player_tree_node *saved;
 struct player *loggedon;
} player_linked_saved_loggedon;

typedef struct player_linked_list
{
 player_linked_saved_loggedon this;
 
 struct player_linked_list *next;
 
 bitflag loggedon : 1;
 bitflag has_prev : 1;
} player_linked_list;

typedef struct player_linked_double_list
{
 player_linked_list s;
 player_linked_list *prev;
} player_linked_double_list;

#endif
