#ifndef ROOM_H
#define ROOM_H

#define ROOM_TRANS_DEFAULT 0
#define ROOM_TRANS_VERBOSE (1<<0) /* for errors */
#define ROOM_TRANS_LOGON (1<<1)
#define ROOM_TRANS_NO_PERMS (1<<2)

#ifdef ROOM_C

# define ROOM_TRANS_NO_LEAVE_MSG (1<<3)
# define ROOM_TRANS_NO_ENTER_MSG (1<<4)
# define ROOM_TRANS_NO_OWNER_EVENT (1<<5)
# define ROOM_TRANS_NO_FOLLOW (1<<6)

# define ROOM_CLEANUP_TIMEOUT_LOAD MK_MINUTES(4)
# define ROOM_CLEANUP_TIMEOUT_CLEANUP MK_MINUTES(2)
# define ROOM_CLEANUP_TIMEOUT_SYNC_ANYWAY MK_MINUTES(32)
# define ROOM_CLEANUP_TIMEOUT_REDO MK_MINUTES(4)

#endif

#define ROOM_PLAYERS_LIST_LIMIT 16

#define ROOM_AUTOMESSAGE_SZ 160
#define ROOM_WHERE_DESCRIPTION_SZ 50
#define ROOM_ID_SZ 15
#define ROOM_ENTER_MSG_SZ PLAYER_S_ENTER_MSG_SZ

#define ROOM_DESCRIPTION_CHARS_SZ 1500
#define ROOM_DESCRIPTION_LINES_SZ 150

typedef struct automessage_node
{
 struct automessage_node *next;
 struct automessage_node *prev;
 char message[ROOM_AUTOMESSAGE_SZ];
} automessage_node;

typedef struct exit_node
{
 struct exit_node *next;
 struct exit_node *prev;
 char name[PLAYER_S_NAME_SZ];
 char id[ROOM_ID_SZ];
} exit_node;

typedef struct room
{
 struct room *next;
 
 struct player_tree_node *owner;
 
 struct player_linked_list *room_list_alpha;
 struct player_linked_list *room_list_cron;

 struct list_node *list_room_local_start;
 
 exit_node *exits_start;
 automessage_node *automessages_start;

 int exits_num;
 int automessages_num;

 int automessage_min_seconds;
 
 Timer_q_node automessage_timer;

 Timer_q_node load_timer;
 time_t a_timestamp;
 time_t l_timestamp;
 
 char *where_description;
 char id[ROOM_ID_SZ];

 char *description;
 char *enter_msg;

 /* so we can implement caching on the room */
 struct timeval last_entered_left;
 
 unsigned int players_num;

 time_t autos_timestamp;
 unsigned int autos_in_seconds;

 int number;

 bitflag flag_home : 1;
 bitflag flag_automessage : 1;
 bitflag flag_locked : 1;
 bitflag flag_bolted : 1;
 bitflag flag_conectable : 1;

 bitflag flag_tmp_in_core : 1;
} room;


#define ROOM_MAIN_FILE_VERSION 1
#define ROOM_PLAYER_FILE_VERSION 1


#endif
