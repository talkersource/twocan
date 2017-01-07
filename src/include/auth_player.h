#ifndef AUTH_PLAYER_H
#define AUTH_PLAYER_H

#ifdef AUTH_PLAYER_C

#define AUTH_PLAYER_FILE_VERSION 1

#define AUTH_PLAYER_NET_OPEN 1
#define AUTH_PLAYER_NET_BLOCK_NEWBIES 2
#define AUTH_PLAYER_NET_BLOCK_RESIS 3
#define AUTH_PLAYER_NET_BLOCK_SUS 4
#define AUTH_PLAYER_NET_BLOCK_ALL 5

typedef struct auth_player_net
{
 struct auth_player_net *next;
 struct auth_player_net *prev;
 
 /* match against CIDDR addresses ... Ie. 127.0.0.1/32 */
 unsigned char ips[4];
 short ip_mask;
 
 short auth_type;
 
 bitflag flag_tmp_tmp : 1;
} auth_player_net;

typedef struct auth_player_name
{
 struct auth_player_name *next;
 struct auth_player_name *prev;

 char *code;

 pcre *compiled_code;
 pcre_extra *studied_code;
 
 bitflag block : 1;
 
 bitflag flag_tmp_tmp : 1;
} auth_player_name;


#endif

#endif
