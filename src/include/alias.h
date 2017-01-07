#ifndef ALIAS_H
#define ALIAS_H
/*
 *  Copyright (C) 1999 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@twocan.org
 */

#ifdef ALIAS_C
/* this doesn't affect much...
   apart from defining a limit so the generic code works */
# define ALIAS_MAX_SYSTEM_ALIASES (1024 * 1024)

# define ALIAS_LIB_FILE_VERSION 1
# define ALIAS_LIB_INDEX_FILE_VERSION 1

typedef struct alias_flag_offset
{
 int type;
 char *data;
} alias_flag_offset;

#endif

#define ALIAS_LIB_LD_SO_SZ 16
#define ALIAS_LIB_NAME_SZ 64

typedef struct alias_lib_node
{
 struct alias_lib_node *next;
 
 struct alias_node *head;
 int num;
 
 char name[ALIAS_LIB_NAME_SZ];
 unsigned int priv_type;

 bitflag needs_help : 1;
 bitflag player_lib : 1;
 bitflag must_use : 1;
} alias_lib_node;

#define ALIAS_COMMAND_SZ 32
#define ALIAS_COMMENT_SZ 128
#define ALIAS_STR_SZ 512

/* NOTE: these _MUST_ be the same as in cmds_list.h */
#define ALIAS_COMMAND_TYPE_NONE 0
#define ALIAS_COMMAND_TYPE_HIDDEN 0
#define ALIAS_COMMAND_TYPE_SOCIALS 1
#define ALIAS_COMMAND_TYPE_COMMUNICATION 2
#define ALIAS_COMMAND_TYPE_LOCAL 3
#define ALIAS_COMMAND_TYPE_INFO 4
#define ALIAS_COMMAND_TYPE_SETTINGS 5
#define ALIAS_COMMAND_TYPE_PERSONAL_INFO 6
#define ALIAS_COMMAND_TYPE_LIST 7
#define ALIAS_COMMAND_TYPE_SYSTEM 8
#define ALIAS_COMMAND_TYPE_MISC 9
#define ALIAS_COMMAND_TYPE_MULTIS 10
#define ALIAS_COMMAND_TYPE_GAMES 11

#define ALIAS_COMMAND_TYPE_SPOD 12
#define ALIAS_COMMAND_TYPE_MINISTER 13
#define ALIAS_COMMAND_TYPE_SU 14
#define ALIAS_COMMAND_TYPE_ADMIN 15

#define ALIAS_COMMAND_TYPE_STATS 23
#define ALIAS_COMMAND_TYPE_ROOM 24
#define ALIAS_COMMAND_TYPE_CHECK 25
#define ALIAS_COMMAND_TYPE_EDITOR 26
#define ALIAS_COMMAND_TYPE_MAIL 27
#define ALIAS_COMMAND_TYPE_NEWS 28
#define ALIAS_COMMAND_TYPE_NEWSGROUP 29
#define ALIAS_COMMAND_TYPE_DRAUGHTS 30
#define ALIAS_COMMAND_TYPE_INTERCOM 31

typedef struct alias_node
{
 struct alias_node *next;
 struct alias_node *prev;
 
 char command[ALIAS_COMMAND_SZ];
 char *str; /* ALIAS_COMMAND_SZ */
 char *comment; /* ALIAS_COMMENT_SZ */
 
 unsigned int command_type;
 
 bitflag flag_no_beg_space : 1;
 bitflag flag_disabled : 1;
 bitflag flag_clever_match : 1;
 bitflag flag_expand_match : 1;
 bitflag flag_hidden : 1;
 bitflag flag_use_name : 1;
 bitflag flag_in_sub_command : 1;
 bitflag flag_in_sub_command_only : 1;
 bitflag flag_public : 1;
 bitflag flag_prived : 1;
} alias_node;


typedef struct alias_search_node
{
 alias_node *player_current;
 alias_node *sys_list[ALIAS_LIB_LD_SO_SZ];
 unsigned int num;
 bitflag is_system_alias : 1;
} alias_search_node;


#endif
