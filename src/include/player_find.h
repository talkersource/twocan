#ifndef PLAYER_FIND_H
#define PLAYER_FIND_H

#define PLAYER_FIND_DEFAULT 0
#define PLAYER_FIND_NEWBIES (1<<0)
#define PLAYER_FIND_EXPAND (1<<1)
#define PLAYER_FIND_VERBOSE (1<<2)
#define PLAYER_FIND_PICK_FIRST (1<<3)
#define PLAYER_FIND_SELF (1<<4)
#define PLAYER_FIND_NICKS (1<<5)
#define PLAYER_FIND_BANISHED (1<<6)

/* usefull shortcuts... */

#define PLAYER_FIND_SC_SU (PLAYER_FIND_VERBOSE | PLAYER_FIND_SELF)
#define PLAYER_FIND_SC_SU_ALL \
 (PLAYER_FIND_NEWBIES | PLAYER_FIND_SC_SU)

#define PLAYER_FIND_SC_COMS \
 (PLAYER_FIND_NEWBIES | PLAYER_FIND_EXPAND | PLAYER_FIND_VERBOSE | \
  PLAYER_FIND_NICKS)

#define PLAYER_FIND_SC_EXTERN \
 (PLAYER_FIND_EXPAND | PLAYER_FIND_VERBOSE | \
  PLAYER_FIND_NICKS | PLAYER_FIND_SELF)
#define PLAYER_FIND_SC_EXTERN_ALL \
 (PLAYER_FIND_NEWBIES | PLAYER_FIND_SC_EXTERN)

#define PLAYER_FIND_SC_INTERN \
 (PLAYER_FIND_EXPAND | PLAYER_FIND_NICKS | PLAYER_FIND_SELF | \
  PLAYER_FIND_PICK_FIRST)
#define PLAYER_FIND_SC_INTERN_ALL \
 (PLAYER_FIND_NEWBIES | PLAYER_FIND_SC_INTERN)

#define PLAYER_FIND_SC_SYS \
 (PLAYER_FIND_EXPAND | PLAYER_FIND_PICK_FIRST | PLAYER_FIND_SELF)
#define PLAYER_FIND_SC_SYS_ALL \
 (PLAYER_FIND_NEWBIES | PLAYER_FIND_SC_INTERN)

#define PLAYER_FIND_MSG_ARR_SZ 8

#define PLAYER_FIND_MSG_TYPE_STR_TOO_LONG 1
#define PLAYER_FIND_MSG_TYPE_NO_SELF 2
#define PLAYER_FIND_MSG_TYPE_NO_NICKNAME_SELF 3
#define PLAYER_FIND_MSG_TYPE_NO_LOCAL_NICKNAME 4
#define PLAYER_FIND_MSG_TYPE_NO_ON_NICKNAME 5
#define PLAYER_FIND_MSG_TYPE_NO_LOCAL_MATCH_START 6
#define PLAYER_FIND_MSG_TYPE_NO_ON_MATCH_START 7
#define PLAYER_FIND_MSG_TYPE_NO_ON_RESIDENT_MATCH_START 8
#define PLAYER_FIND_MSG_TYPE_NO_ON_OTHER_MATCH_START 9
#define PLAYER_FIND_MSG_TYPE_NO_ON_OTHER_RESIDENT_MATCH_START 10
#define PLAYER_FIND_MSG_TYPE_NO_ON_MATCH 11
#define PLAYER_FIND_MSG_TYPE_NO_ON_RESIDENT_MATCH 12
#define PLAYER_FIND_MSG_TYPE_NO_MATCH_START 13
#define PLAYER_FIND_MSG_TYPE_NO_RESIDENT_MATCH_START 14
#define PLAYER_FIND_MSG_TYPE_NO_OTHER_MATCH_START 15
#define PLAYER_FIND_MSG_TYPE_NO_OTHER_RESIDENT_MATCH_START 16
#define PLAYER_FIND_MSG_TYPE_NO_MATCH 17
#define PLAYER_FIND_MSG_TYPE_NO_RESIDENT_MATCH 18
#define PLAYER_FIND_MSG_TYPE_BANISHED 19
#define PLAYER_FIND_MSG_TYPE_SYSTEM_CHARACTER 20

#endif
