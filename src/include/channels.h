#ifndef CHANNELS_H
#define CHANNELS_H

#ifdef CHANNELS_C
# define CHANNELS_TIMEOUT_SAVE MK_MINUTES(4)
# define CHANNELS_TIMEOUT_SYNC MK_MINUTES(2)
# define CHANNELS_TIMEOUT_SYNC_ANYWAY MK_MINUTES(32)
# define CHANNELS_TIMEOUT_REDO MK_MINUTES(4)
#endif

#define CHANNELS_FILE_VERSION 1

#define CHANNELS_JOIN_LEAVE_SYS_ON 1
#define CHANNELS_JOIN_LEAVE_SYS_OFF 2
#define CHANNELS_JOIN_LEAVE_USR_ON 4
#define CHANNELS_JOIN_LEAVE_USR_OFF 8
#define CHANNELS_JOIN_LEAVE_USR_BOOT 16
#define CHANNELS_JOIN_LEAVE_SYS_DIE 32
#define CHANNELS_JOIN_LEAVE_FILE_ON 64

#define CHANNELS_COLOUR_SZ 8
#define CHANNELS_SEPERATOR_SZ 12

#define CHANNELS_NAME_SZ 32

typedef struct channels_base
{
 struct channels_base *next;
 struct channels_base *prev;
 
 char name[CHANNELS_NAME_SZ];

 struct list_node *list_start;

 size_t name_sz;
 
 int players_num;
 
 struct player_linked_list *players_start;

 short int def_colour_type;
 short int def_name_sep;

 void (*join_leave_func)(struct player_tree_node *, int);
 
 bitflag flag_no_kill : 1;
 bitflag flag_no_blocks : 1;
 bitflag flag_tmp_needs_saving : 1;
} channels_base;

typedef struct channels_node
{
 struct player_linked_double_list link;
 
 struct channels_node *next;
 struct channels_node *prev;
 
 struct channels_base *base;

 short int colour_type;
 short int name_sep;
} channels_node;

#endif
