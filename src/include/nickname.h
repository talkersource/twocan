#ifndef NICKNAME_H
#define NICKNAME_H

#define NICKNAME_NICK_SZ 9
 
typedef struct nickname_node
{
 struct nickname_node *next;
 struct nickname_node *prev;

 time_t c_timestamp;
 
 char nick[NICKNAME_NICK_SZ];
 char name[PLAYER_S_NAME_SZ];
} nickname_node;



#endif
