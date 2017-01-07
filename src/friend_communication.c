#define FRIEND_COMMUNICATION_C
/*
 *  Copyright (C) 1999 James Antill, John Tobin
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
 * email: james@twocan.org, john@twocan.org
 */
#include "main.h"


static int inorder_tell_friends(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 const char *str = va_arg(va, const char *);
 size_t length = va_arg(va, size_t);
 int *count_of_outs = va_arg(va, int *);
 
 LIST_COMS_2CHECK_FLAG_START(entry->parent, p->saved, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(tfs))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();

 if (entry->flags & MULTI_TO_FRIENDS)
   fvtell_player(TALK_TP(p), "%s You %s your friends '%s%s'%s\n",
                 USER_COLOUR_MINE, tell_ask_exclaim_me(p, str, length),
                 str, USER_COLOUR_MINE, "^N");
 else
 {
  fvtell_player(TALK_TP(entry->parent->player_ptr),
                "%s%s%s %s %s '%s%s'%s\n",
                USER_COLOUR_TFRIENDS,
                SHOW_PERSONAL(entry->parent->player_ptr, "* "),
                "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                " t($F-Name_full) f($F-Name))",
                tell_ask_exclaim_group(p, str, length),
                multi_get_names(entry->number, entry, NULL, NULL, NULL,
                                MULTI_NO_MULTIS),
                str, USER_COLOUR_TFRIENDS, "^N");
  ++*count_of_outs;
 }
 
 return (TRUE);
}

static int inorder_remote_friends(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 const char *space = va_arg(va, const char *);
 const char *str = va_arg(va, const char *);
 int *count_of_outs = va_arg(va, int *);

 LIST_COMS_2CHECK_FLAG_START(entry->parent, p->saved, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(tfs))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();
 
 if (entry->flags & MULTI_TO_FRIENDS)
 {
  twinkle_info info;
  
  setup_twinkle_info(&info);
  
  info.output_not_me = TRUE;

  log_assert(p == entry->parent->player_ptr);
  fvtell_player(TALK_IT(&info, p->saved, p),
                "%s You remote: %s%s%s%s (to your friends)%s'\n",
                USER_COLOUR_MINE,
                "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                " t($F-Name_full) f($F-Name))",
                space, str, USER_COLOUR_MINE, "^N");
 }
 else
 {
  fvtell_player(TALK_TP(entry->parent->player_ptr),
                "%s%s%s%s%s%s (to %s)%s\n",
                USER_COLOUR_TFRIENDS,
                SHOW_PERSONAL(entry->parent->player_ptr, "* "),
                "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                " t($F-Name_full) f($F-Name))",
                space, str, USER_COLOUR_TFRIENDS,
                multi_get_names(entry->number, entry, NULL, NULL, NULL,
                                MULTI_NO_MULTIS),
                "^N");
  ++*count_of_outs;
 }
 
 return (TRUE);
}

static void internal_tell_remote_friends(player *p, const char *str,
                                         size_t length, const char *type)
{
 multi_return *values = NULL;
 char name_buffer[] = "friends";
 int count_of_outs = 0;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!*str)
   TELL_FORMAT(p, "<message>");

 values = multi_add(p->saved, name_buffer,
                    MULTI_DIE_EMPTY_GROUP |
                    MULTI_DIE_MATCH_GROUP |
                    MULTI_LIVE_ON_SMALL |
                    MULTI_MUST_CREATE |
                    MULTI_DESTROY_CLEANUP, PLAYER_FIND_SC_COMS);

 if (values->multi_number)  
 {
  if (values->players_added != 1)
  {
   if (!strcmp(type, "tell"))
     do_inorder_multi(inorder_tell_friends, values->multi_number,
                      MULTI_DEFAULT, /* start va_arg args */
                      p, str, length, &count_of_outs);
   else
     do_inorder_multi(inorder_remote_friends, values->multi_number,
                      MULTI_DEFAULT, /* start va_arg args */
                      p, isits1(str), isits2(str), &count_of_outs);
   
   if (!count_of_outs)
     fvtell_player(SYSTEM_T(p), "%s",
                   " All your friends are blocking "
                   "friends coms at the moment.\n");  
  }
  else
    fvtell_player(NORMAL_T(p), "%s",
                  " You don't have any friends on "
                  "at the moment.\n");  
    
  multi_cleanup(values->multi_number, MULTI_DEFAULT);
 }
 else
 {
  if (!strcmp(type, "tell"))
    switch (values->error_number)
    {
     case MULTI_BAD_FRIENDS_SELECTION:
       fvtell_player(NORMAL_T(p), "%s",
                     " You can not tell friends while blocking "
                     "friend tells.\n");
       return;
       
     case MULTI_BAD_FRIENDS_FIND:
       fvtell_player(NORMAL_T(p), "%s",
                     " None of your friends are logged on at the moment, or"
                       " they're all blocking friends tells.\n");
       return;
       
     default:
       fvtell_player(NORMAL_T(p), " ** Error %u.\n", values->error_number);
       return;
    }
  else
    switch (values->error_number)
    {
     case MULTI_BAD_FRIENDS_SELECTION:
       fvtell_player(NORMAL_T(p), "%s", " You can remote to friends while "
                     "blocking friend tells.\n");
       return;
       
     case MULTI_BAD_FRIENDS_FIND:
       fvtell_player(NORMAL_T(p), "%s",
                     " None of your friends are logged on at the moment, or"
                     " they're all blocking remotes to friends.\n");
       return;
       
     default:
       fvtell_player(NORMAL_T(p), " ** Error %u.\n", values->error_number);
       return;
    }
 }
}

void user_tell_friends(player *p, const char *str, size_t length)
{
 internal_tell_remote_friends(p, str, length, "tell");
}

void user_remote_friends(player *p, const char *str, size_t length)
{
 internal_tell_remote_friends(p, str, length, "remote");
}

static int inorder_tell_friends_of(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 multi_node *friends_of = va_arg(va, multi_node *);
 const char *str = va_arg(va, const char *);
 size_t length = va_arg(va, size_t);
 int *count_of_outs = va_arg(va, int *);
 
 LIST_COMS_2CHECK_FLAG_START(entry->parent, p->saved, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(tfsof))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();

 if (entry->parent == p->saved)
   fvtell_player(TALK_TP(p), "%s You %s %s '%s%s'%s\n", USER_COLOUR_MINE,
                 tell_ask_exclaim_me(p, str, length),
                 multi_get_names(entry->number, entry, NULL, NULL, friends_of,
                                 MULTI_NO_MULTIS),
                 str, USER_COLOUR_MINE, "^N");
 else
 {
  fvtell_player(TALK_TP(entry->parent->player_ptr),
                "%s%s%s %s %s '%s%s'%s\n",
                USER_COLOUR_TFOF,
                SHOW_PERSONAL(entry->parent->player_ptr, "} "),
                "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                " t($F-Name_full) f($F-Name))",
                tell_ask_exclaim_group(p, str, length),
                multi_get_names(entry->number, entry, NULL, NULL,
                                friends_of,
                                MULTI_NO_MULTIS),
                str, USER_COLOUR_TFOF, "^N");
  ++*count_of_outs;
 }
 
 return (TRUE);
}

static int inorder_remote_friends_of(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 multi_node *friends_of = va_arg(va, multi_node *);
 const char *space = va_arg(va, const char *);
 const char *str = va_arg(va, const char *);
 int *count_of_outs = va_arg(va, int *);

 LIST_COMS_2CHECK_FLAG_START(entry->parent, p->saved, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(tfsof))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();

 if (entry->parent == p->saved)
 {
  fvtell_player(TALK_TP(p), "%s You remote: %s%s%s%s (to %s)%s\n",
                USER_COLOUR_MINE,
                "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                " t($F-Name_full) f($F-Name))",
                space, str, USER_COLOUR_MINE,
                multi_get_names(entry->number, entry, NULL, NULL, friends_of,
                                MULTI_NO_MULTIS),
                "^N");
 }
 else
 {
  fvtell_player(TALK_TP(entry->parent->player_ptr),
                "%s%s%s%s%s%s (to %s)%s\n",
                USER_COLOUR_TFOF,
                SHOW_PERSONAL(entry->parent->player_ptr, "} "),
                "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                " t($F-Name_full) f($F-Name))",
                space, str, USER_COLOUR_TFOF,
                multi_get_names(entry->number, entry, NULL, NULL,
                                friends_of,
                                MULTI_NO_MULTIS),
                "^N");
  ++*count_of_outs;
 }
 
 return (TRUE);
}

static void internal_tell_remote_friendsof(player *p, const char *str,
                                           size_t length, const char *type)
{
 const char *orig_str = str;
 multi_return *values = NULL;
 char name_buffer[sizeof("friendsof:") + PLAYER_S_NAME_SZ];
 int count_of_outs = 0;
 parameter_holder params;
 
 get_parameter_init(&params);

 assert(!strcmp(type, "tell") || !strcmp(type, "remote"));
 
 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<player> <text>");
 length -= (str - orig_str);

 lower_case(GET_PARAMETER_STR((&params), 1));

 sprintf(name_buffer, "%s%.*s", "friendsof:",
         PLAYER_S_NAME_SZ - 1, GET_PARAMETER_STR((&params), 1));
    
 values = multi_add(p->saved, name_buffer,
                    MULTI_VERBOSE |
                    MULTI_LIVE_ON_SMALL |
                    MULTI_DIE_EMPTY_GROUP |
                    MULTI_DIE_MATCH_GROUP |
                    MULTI_MUST_CREATE |
                    MULTI_DESTROY_CLEANUP, PLAYER_FIND_SC_COMS);

 if (values->multi_number)  
 {
  multi_node *friends_of = multi_get_node_with_flag(values->multi_number,
                                                    MULTI_TO_FRIENDS_OF);

  if (!strcmp(type, "tell"))
    do_inorder_multi(inorder_tell_friends_of, values->multi_number,
                     MULTI_DEFAULT, /* start va_arg args */
                     p, friends_of, str, length, &count_of_outs);
  else
    do_inorder_multi(inorder_remote_friends_of, values->multi_number,
                     MULTI_DEFAULT, /* start va_arg args */
                     p, friends_of, isits1(str), isits2(str), &count_of_outs);

  if (!count_of_outs)
    fvtell_player(NORMAL_T(p),
                  " It seems that %s are blocking friend coms.\n",
		    multi_get_names(values->multi_number, NULL, NULL, NULL,
				    friends_of,
				    MULTI_NO_MULTIS));

  multi_cleanup(values->multi_number, MULTI_DEFAULT);
 }
}

static void user_tell_friendsof(player *p, const char *str, size_t length)
{
 internal_tell_remote_friendsof(p, str, length, "tell");
}

static void user_remote_friendsof(player *p, const char *str, size_t length)
{
 internal_tell_remote_friendsof(p, str, length, "remote");
}

void cmds_init_friend_coms(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("rfriends", user_remote_friends, CHARS_SIZE_T, COMMUNICATION);
 CMDS_ADD("rfriendsof", user_remote_friendsof, CHARS_SIZE_T, COMMUNICATION);

 CMDS_ADD("tfriends", user_tell_friends, CHARS_SIZE_T, COMMUNICATION);
 CMDS_ADD("tfriendsof", user_tell_friendsof, CHARS_SIZE_T, COMMUNICATION);
}
