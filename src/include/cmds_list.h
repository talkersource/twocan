#ifndef CMDS_LIST_H
#define CMDS_LIST_H
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

#ifndef CMDS_DEBUG
# ifndef NDEBUG
#  define CMDS_DEBUG 1
# else
#  define CMDS_DEBUG 0
# endif
#endif

#define CMDS_PARAM_NOTHING 0
#define CMDS_PARAM_CONST_CHARS 1
#define CMDS_PARAM_CHARS_SIZE_T 2
#define CMDS_PARAM_RET_CHARS_SIZE_T 3
#define CMDS_PARAM_NO_CHARS 4
#define CMDS_PARAM_PARSE_PARAMS 5

/* remmber to add the init's to cmds_list.c */
#define CMDS_SIZE_ALPHA 26
#define CMDS_SIZE_SUB 12
#define CMDS_SIZE_MISC 1
#define CMDS_SIZE_SECTION 35

/* these _MUST_NOT_ change as aliases rely on the same numbers --
   just add to end */
#define CMDS_SECTION_HIDDEN 0
#define CMDS_SECTION_SOCIAL 1
#define CMDS_SECTION_COMMUNICATION 2
#define CMDS_SECTION_LOCAL 3
#define CMDS_SECTION_INFORMATION 4
#define CMDS_SECTION_SETTINGS 5
#define CMDS_SECTION_PERSONAL_INFO 6
#define CMDS_SECTION_LIST 7
#define CMDS_SECTION_SYSTEM 8
#define CMDS_SECTION_MISC 9
#define CMDS_SECTION_MULTI 10
#define CMDS_SECTION_GAME 11

#define CMDS_SECTION_SPOD 12
#define CMDS_SECTION_MINISTER 13
#define CMDS_SECTION_SU 14
#define CMDS_SECTION_ADMIN 15
/* 
 * room for growth in normal commands
 */
#define CMDS_SUB_SECTION_START 23 /* used to convert section => sub */
#define CMDS_SECTION_STATS 23
#define CMDS_SECTION_ROOM 24
#define CMDS_SECTION_CHECK 25
#define CMDS_SECTION_EDITOR 26
#define CMDS_SECTION_MAIL 27
#define CMDS_SECTION_NEWS 28
#define CMDS_SECTION_NEWSGROUP 29
#define CMDS_SECTION_DRAUGHTS 30
#define CMDS_SECTION_INTERCOM 31
#define CMDS_SECTION_CHLIM 32
#define CMDS_SECTION_CHANNELS 33
#define CMDS_SECTION_CONFIGURE 34
/* room for growth upto... 127 */

#define CMDS_MISC_RESTRICTED 0

#define CMDS_MALLOC_OBJS_BLOCK_SZ 128
#define CMDS_MALLOC_LIST_BLOCK_SZ 16

#define CMDS_MATCH_DEFAULT_LOOP 4

typedef struct command_node
{
 const char *name;

 struct cmds_function func;
 
 int totals;

 int (*test_can_run)(struct player_tree_node *);
 
 bitflag flag_no_space_needed : 1;
 bitflag flag_no_expand : 1;
 bitflag flag_no_clever_expand : 1;
 bitflag flag_no_beg_space : 1;
 bitflag flag_no_end_space : 1;
 bitflag flag_disabled : 1;
} command_node;

typedef struct cmds_function cmds_function;

typedef struct command_base
{
 command_node **ptr;
 size_t size;
 size_t max_size;
} command_base;

#define CMDS_INIT_BASE() {NULL, 0, 0}

#endif
