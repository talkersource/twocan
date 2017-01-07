#ifndef SAFEMALLOC_H
#define SAFEMALLOC_H

/* temp because I know it will work on linux */
# define USE_POINTER_COMPARES 1

# ifdef SAFEMALLOC_C

# define MALLOC_MALLOC_GARBAGE 0xFA

#  ifdef EXTRA_MALLOC_WRAPPER

/* we only know the size if this is on */
#  define MALLOC_FREE_GARBAGE   0xFB

typedef struct malloc_lookup
{
 struct malloc_lookup *next;
 struct malloc_lookup *prev;
 void *mem_ref;
 const char *file_ref;
 unsigned int line_ref;
 unsigned int type;
 size_t size;
} malloc_lookup;

#  endif /* using warns */
# endif /* in the safemalloc file */


/* types for calling MALLOC with */
#define MALLOC_TYPE_UNDEFINED 0
#define MALLOC_TYPE_PLAYER 1
#define MALLOC_TYPE_ROOM 2
#define MALLOC_TYPE_MULTI_NODE 3
#define MALLOC_TYPE_MULTI_BASE 4
#define MALLOC_TYPE_INPUT_NODE 5
#define MALLOC_TYPE_OUTPUT_NODE 6
#define MALLOC_TYPE_PLAYER_TREE_NODE 7
#define MALLOC_TYPE_PLAYER_LINKED_LIST 8
#define MALLOC_TYPE_PLAYER_LINKED_DOUBLE_LIST 9
#define MALLOC_TYPE_ALIAS_NODE 10
#define MALLOC_TYPE_ALIAS_LIB_NODE 11
#define MALLOC_TYPE_NICKNAME_NODE 12
#define MALLOC_TYPE_PLAYER_FILE 13
#define MALLOC_TYPE_NEWS_GROUP 14
#define MALLOC_TYPE_NEWS_ARTICLE 15
#define MALLOC_TYPE_CHILD_COM 16
#define MALLOC_TYPE_ROOM_EXIT_NODE 17
#define MALLOC_TYPE_ROOM_AUTOMESSAGE_NODE 18
#define MALLOC_TYPE_LIST_SELF_NODE 19
#define MALLOC_TYPE_LIST_COMS_NODE 20
#define MALLOC_TYPE_LIST_ROOM_NODE 21
#define MALLOC_TYPE_LIST_GAME_NODE 22
#define MALLOC_TYPE_LIST_CHAN_NODE 23
#define MALLOC_TYPE_TIMER_NODE 24
#define MALLOC_TYPE_TIMER_DOUBLE_NODE 25
#define MALLOC_TYPE_MAIL_SENT 26
#define MALLOC_TYPE_MAIL_RECIEVED 27
#define MALLOC_TYPE_HELP_NODE 28
#define MALLOC_TYPE_AUTH_PLAYER_NAME 29
#define MALLOC_TYPE_AUTH_PLAYER_NET 30
#define MALLOC_TYPE_EDIT_BASE 31
#define MALLOC_TYPE_EDIT_LINE_NODE 32
#define MALLOC_TYPE_TIP_NODE 33
#define MALLOC_TYPE_TIP_BASE 34
#define MALLOC_TYPE_OUTPUT_COMPRESS_LIB 35
#define MALLOC_TYPE_OUTPUT_COMPRESS_STREAM 36
#define MALLOC_TYPE_OUTPUT_COMPRESS_BUF 37
#define MALLOC_TYPE_TERMINAL_TERMCAP 38
#define MALLOC_TYPE_PCRE 39
#define MALLOC_TYPE_DRAUGHTS 40
#define MALLOC_TYPE_CHAN_NODE 41
#define MALLOC_TYPE_CHAN_BASE 42
#define MALLOC_TYPE_CMDS_NODE_OBJS 43
#define MALLOC_TYPE_CMDS_NODE_LIST 44
#define MALLOC_TYPE_CONFIGURE_INTERFACE_NODE 45
#define MALLOC_TYPE_CONFIGURE_IPV4_NODE 46
#define MALLOC_TYPE_CONFIGURE_IPV6_NODE 47
#define MALLOC_TYPE_CONFIGURE_NAME_NODE 48
#define MALLOC_TYPE_SOCKET_INTERFACE_NODE 49
#define MALLOC_TYPE_TEXT_OBJS_NODE 50

/* the size of the array to store the mallocs */
#define MALLOC_TYPES_SIZE 51

#endif
