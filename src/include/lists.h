#ifndef LISTS_H
#define LISTS_H

#ifdef LISTS_C
# define LIST_CLEANUP_TIMEOUT_LOAD MK_MINUTES(16)
# define LIST_CLEANUP_TIMEOUT_CLEANUP MK_MINUTES(2)
# define LIST_CLEANUP_TIMEOUT_SYNC_ANYWAY MK_MINUTES(32)
# define LIST_CLEANUP_TIMEOUT_REDO MK_MINUTES(8)
#endif

#define LIST_TYPE_SELF 0
#define LIST_TYPE_COMS 1
#define LIST_TYPE_ROOM 2
#define LIST_TYPE_GAME 3
#define LIST_TYPE_CHAN 4

#define LIST_HEAD_CHILD list_node l; \
 bitflag list_type : 3; \
 bitflag flag_grouped : 1; \
 bitflag flag_nothing : 1

typedef struct list_node 
{
 char *name;

 struct list_node *next;
 struct list_node *prev;

 time_t c_timestamp;
} list_node;

typedef struct list_self_node 
{
 LIST_HEAD_CHILD;
 
 bitflag article_inform : 1;
 bitflag find : 1;
 bitflag friend : 1;
 bitflag grab : 1;
 bitflag inform : 1;
 bitflag inform_beep : 1;
} list_self_node;

typedef struct list_coms_node
{
 LIST_HEAD_CHILD;

 /* these are all _blocks_ */
 bitflag says : 1;
 bitflag tells : 1;
 bitflag shouts : 1;
 bitflag multis : 1;
 bitflag tfs : 1;
 bitflag tfsof : 1;

 bitflag channels : 1;
 bitflag echos : 1;
 bitflag wakes : 1;
 
 bitflag sessions : 1;
 bitflag comments : 1;
 bitflag autos : 1; /* if you are in their room */
 bitflag movement : 1; /* don't see them logon/logoff, enter/leave rooms */
} list_coms_node;

typedef struct list_room_node
{
 LIST_HEAD_CHILD;
 
 bitflag alter : 1; 
 bitflag bar : 1;
 bitflag bolt : 1;
 bitflag boot : 1;
 bitflag invite : 1;
 bitflag grant : 1;
 bitflag key : 1;
 bitflag link : 1;
} list_room_node;

typedef struct list_game_node
{
 LIST_HEAD_CHILD;

 /* these are all _blocks_ */
 bitflag draughts : 1;
 bitflag sps : 1;
 bitflag ttt : 1;
} list_game_node;

typedef struct list_chan_node
{
 LIST_HEAD_CHILD;

 bitflag boot : 1;
 bitflag config : 1;
 bitflag grant : 1;
 bitflag read : 1;
 bitflag who : 1;
 bitflag write : 1;
} list_chan_node;

#endif
