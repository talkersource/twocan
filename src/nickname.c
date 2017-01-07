#define NICKNAME_C
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
#include "main.h"



static nickname_node *nickname_new_node(player *p,
                                        const char *nick, const char *name,
                                        time_t c_timestamp)
{
 nickname_node *insert = p->nicknames_start;
 nickname_node *new_node = NULL;
 player_tree_node *tmp = NULL;
 
 if (!(tmp = player_find_all(NULL, name, PLAYER_FIND_DEFAULT)) ||
     (difftime(tmp->c_timestamp, c_timestamp) >= 0))
   return (NULL);
 
 if (!(new_node = XMALLOC(sizeof(nickname_node), NICKNAME_NODE)))
   return (NULL);
 
 if (insert)
 {
  while (insert->next && (strcmp(nick, insert->nick) > 0))
    insert = insert->next;
  
  if (!strcmp(nick, insert->nick))
  {
   XFREE(new_node, NICKNAME_NODE);
   return (NULL);
  }
  
  if (strcmp(nick, insert->nick) > 0)
  {
   if ((new_node->next = insert->next))
     new_node->next->prev = new_node;
   
   new_node->prev = insert;
   insert->next = new_node;
  }
  else
  {
   assert(strcmp(nick, insert->nick) < 0);
   
   if ((new_node->prev = insert->prev))
     new_node->prev->next = new_node;
   else
     p->nicknames_start = new_node;
   
   new_node->next = insert;
   insert->prev = new_node;
  }
 }
 else
 {
  p->nicknames_start = new_node;
  new_node->next = NULL;
  new_node->prev = NULL;
 }

 new_node->c_timestamp = c_timestamp;
 COPY_STR(new_node->nick, nick, NICKNAME_NICK_SZ);
 COPY_STR(new_node->name, name, PLAYER_S_NAME_SZ);
 
 return (new_node);
}

static nickname_node *nickname_find_node(player *p, const char *nick)
{
 nickname_node *scan = p->nicknames_start;
 
 while (scan && (strcmp(nick, scan->nick) > 0))
   scan = scan->next;

 if (scan && !strcmp(nick, scan->nick))
   return (scan);
 else
   return (NULL);
}

static int nickname_destroy_node(player *p, const char *nick)
{
 nickname_node *tmp = nickname_find_node(p, nick);
 
 if (tmp)
 {
  if (tmp->next)
    tmp->next->prev = tmp->prev;

  if (tmp->prev)
    tmp->prev->next = tmp->next;
  else
    p->nicknames_start = tmp->next;

  XFREE(tmp, NICKNAME_NODE);
  
  return (TRUE);
 }

 return (FALSE);
}

void nickname_load(player *p, file_io *io_player)
{
 int count = 0;
 int real_count = 0;
 
 file_section_beg("header", io_player);

 p->max_nicknames = file_get_int("max_nicknames", io_player);
 p->number_of_nicknames = file_get_int("number_of_nicknames", io_player);
 
 file_section_end("header", io_player);

 file_section_beg("list", io_player);
 
 while (count < p->number_of_nicknames)
 {
  char nickname[NICKNAME_NICK_SZ];
  char realname[PLAYER_S_NAME_SZ];
  char buffer[BUF_NUM_TYPE_SZ(int)];
  time_t c_timestamp;
  
  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_player);

  c_timestamp = file_get_time_t("c_timestamp", io_player);
  file_get_string("nickname", nickname, NICKNAME_NICK_SZ, io_player);
  file_get_string("realname", realname, PLAYER_S_NAME_SZ, io_player);
  
  if (nickname_new_node(p, nickname, realname, c_timestamp))
    ++real_count;

  file_section_end(buffer, io_player);
 }
 assert(p->number_of_nicknames == count);
 p->number_of_nicknames = real_count;

 file_section_end("list", io_player);
}

void nickname_save(player *p, file_io *io_player)
{
 int count = 0;
 nickname_node *scan = p->nicknames_start;
 
 file_section_beg("header", io_player);

 file_put_int("max_nicknames", p->max_nicknames, io_player);
 file_put_int("number_of_nicknames", p->number_of_nicknames, io_player);
 
 file_section_end("header", io_player);

 file_section_beg("list", io_player);

 while (scan)
 {
  char buffer[BUF_NUM_TYPE_SZ(int)];

  sprintf(buffer, "%04d", ++count);

  file_section_beg(buffer, io_player);

  file_put_time_t("c_timestamp", scan->c_timestamp, io_player);
  file_put_string("nickname", scan->nick, 0, io_player);
  file_put_string("realname", scan->name, 0, io_player);

  file_section_end(buffer, io_player);
  
  scan = scan->next;
 }
 log_assert((p->number_of_nicknames == count) && !scan);

 file_section_end("list", io_player);
 
 p->number_of_nicknames = count;
}

void nickname_cleanup(player *p)
{
 nickname_node *scan = p->nicknames_start;
 
 while (scan)
 {
  nickname_node *scan_next = scan->next; 

  XFREE(scan, NICKNAME_NODE);
  
  scan = scan_next;
 }

 p->nicknames_start = NULL;
 p->flag_tmp_dont_save_after_this = TRUE;
}

player_tree_node *nickname_player_tree_find(player *p, const char *nick)
{
 nickname_node *scan = NULL;

 log_assert(p);

 scan = p->nicknames_start;
 
 while (scan && (strcmp(nick, scan->nick) > 0))
   scan = scan->next;

 if (scan && !strcmp(nick, scan->nick))
 {
  char lower_name[PLAYER_S_NAME_SZ];

  qstrcpy(lower_name, scan->name);
  lower_case(lower_name);

   /* no nickname in here so no recursion */
  return (player_tree_find_exact(lower_name));
 }
 else
   return (NULL);
}

player *nickname_player_find(player *p, const char *nick)
{
 player_tree_node *tmp = nickname_player_tree_find(p, nick);

 if (tmp)
   return (tmp->player_ptr);
 else
   return (NULL);
}

static void user_nicknames_show_all(player *p, parameter_holder *params)
{
 player *p2 = p;
 nickname_node *scan = NULL;
 char buffer[sizeof("Used %d of %d nicknames") + (BUF_NUM_TYPE_SZ(int) * 2)];
 
 if (p->saved->priv_basic_su && params->last_param)
 {
  player *tmp = player_find_on(p, GET_PARAMETER_STR(params, 1),
                               PLAYER_FIND_SC_SU);

  if (tmp)
    p2 = tmp;
  
  get_parameter_shift(params, 1);
 }
 
 if (!(scan = p2->nicknames_start))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You don't have any ^S^Bnicknames^s currently defined.\n");
  return;
 }

 sprintf(buffer, "Used %d of %d nicknames",
         p2->number_of_nicknames, p2->max_nicknames);
 ptell_mid(NORMAL_T(p), buffer, FALSE);
 
 fvtell_player(NORMAL_T(p), "%-*s - Real name\n\n",
               16, " - Nickname -");

 while (scan)
 {
  fvtell_player(NORMAL_FT(RAW_OUTPUT, p), "%-*s - %s\n",
                16, scan->nick, scan->name);

  scan = scan->next;
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

static void user_nickname_add(player *p, parameter_holder *params)
{ 
 if (params->last_param != 2)
   TELL_FORMAT(p, "<nickname> <realname>");
 
 if ((p->number_of_nicknames + 1) > p->max_nicknames)
 {
  fvtell_player(NORMAL_T(p), 
                " You have reached your nickname limit of"
                " ^S^B%d^s.\n",
                p->max_nicknames);
  return;
 }
 
 if (GET_PARAMETER_LENGTH(params, 1) >= NICKNAME_NICK_SZ)
 {
  int longer_tmp = GET_PARAMETER_LENGTH(params, 1) -  (NICKNAME_NICK_SZ - 1);
  fvtell_player(NORMAL_T(p), " Nickname is %d character%s longer than "
                "the maximum length allowed.\n",
                longer_tmp, (longer_tmp > 1 ? "s" : ""));
  return;
 }

 if (!player_find_all(p, GET_PARAMETER_STR(params, 2), PLAYER_FIND_VERBOSE))
   return;
 
 if (nickname_new_node(p, GET_PARAMETER_STR(params, 1),
                       GET_PARAMETER_STR(params, 2), now))
 {
  fvtell_player(NORMAL_T(p), " Nickname ^S^B%s^s added.\n",
                GET_PARAMETER_STR(params, 1));
  p->number_of_nicknames++;
 }
 else
   fvtell_player(NORMAL_T(p), " Cannot add nickname -- ^S^B%s^s -- "
                 "with name -- ^S^B%s^s --.\n",
                 GET_PARAMETER_STR(params, 1), GET_PARAMETER_STR(params, 2));
}

static void user_nickname_del(player *p, parameter_holder *params)
{ 
 if (params->last_param != 1)
   TELL_FORMAT(p, "<nickname>");
 
 if (nickname_destroy_node(p, GET_PARAMETER_STR(params, 1)))
 {
  p->number_of_nicknames--;
  fvtell_player(NORMAL_T(p), " Nickname ^S^B%s^s removed.\n",
                GET_PARAMETER_STR(params, 1));
 }
 else
   fvtell_player(NORMAL_T(p), " Nickname ^S^B%s^s doesn't exist.\n",
                 GET_PARAMETER_STR(params, 1));
}

void cmds_init_nickname(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("nickname_add", user_nickname_add, PARSE_PARAMS, MISC);
 CMDS_PRIV(base);
 CMDS_ADD("nickname_delete", user_nickname_del, PARSE_PARAMS, MISC);
 CMDS_PRIV(base);
 CMDS_ADD("nickname_show", user_nicknames_show_all, PARSE_PARAMS, MISC);
 CMDS_PRIV(base);
}
