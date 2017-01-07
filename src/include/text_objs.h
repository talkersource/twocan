#ifndef TEXT_OBJS_H
#define TEXT_OBJS_H

#ifdef TEXT_OBJS_C
#define TEXT_OBJS_CLEANUP_TIMEOUT_INIT MK_MINUTES(5)
#define TEXT_OBJS_CLEANUP_TIMEOUT_REDO MK_MINUTES(1)
#define TEXT_OBJS_CLEANUP_TIMEOUT_ACCESS MK_MINUTES(1)
#define TEXT_OBJS_CLEANUP_TIMEOUT_CREATE MK_MINUTES(32)

#endif

#define TEXT_OBJS_ID_MAX (INT_MAX)

#define TEXT_OBJS_TYPE_DEFAULT 0
#define TEXT_OBJS_TYPE_PARAMS 1


typedef struct text_objs_node
{
 struct text_objs_node *next;

 int id;

 int type;

 time_t c_timestamp;
 time_t a_timestamp;
 
 char *str;
 size_t len;

 char owner[PLAYER_S_NAME_SZ];
 
 bitflag can_del : 1;
} text_objs_node;

#endif
