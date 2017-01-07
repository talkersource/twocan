#define MULTI_COMMUNICATION_C
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

/* contains commands that only affect ppl in a list (such as tell) or
   have the power to be used on ppl in other rooms, such as wake/shout */




/* functons... */

const char *tell_ask_exclaim_me(player *p, const char *str, size_t length)
{
 IGNORE_PARAMETER(p); /* symentry is a wonderful thing... */

 if (!length)
   ++length;
 
 switch (str[length - 1])
 {
  case '?':
   return ("ask");
   
  case '!':
   return ("exclaim to");
   
  default:
    break;
 }

 return ("tell");
}

const char *tell_ask_exclaim_group(player *p, const char *str, size_t length)
{
 if (p->gender == GENDER_PLURAL)
   return (tell_ask_exclaim_me(p, str, length));

 if (!length)
   ++length;
 
 switch (str[length - 1])
 {
  case '?':
    return ("asks");
    
  case '!':
    return ("exclaims to");
    
  default:
    break;
 }
 
 return ("tells");
}

/* the atell command, lets admin get through blocks. (no tell sus check) */
void user_su_tell(player *p, const char *str, size_t length)
{
 player *p2 = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<player> <msg>");

 if (!(p2 = player_find_on(p, GET_PARAMETER_STR((&params), 1),
                           PLAYER_FIND_SC_SU_ALL)))
   return;
 
 fvtell_player(ALL_T(p->saved, p2, NULL, RAW_OUTPUT_VARIABLES | 3, now),
               "%s%s%s %s you '%s%s'.%s\n",
               USER_COLOUR_TELL, SHOW_PERSONAL(p2, "> "),
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               tell_ask_exclaim_group(p, str, length),
               str, USER_COLOUR_TELL, "^N");
 
 fvtell_player(ALL_T(p2->saved, p, NULL, RAW_OUTPUT_VARIABLES | 3, now),
               "%s You %s %s '%s%s'.^N\n",
               tell_ask_exclaim_me(p, str, length), USER_COLOUR_MINE,
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               str, USER_COLOUR_MINE);
}

void user_wake(player *p, const char *str)
{
 player *p2 = NULL;

 if (!*str)
   TELL_FORMAT(p, "<player>");

 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_COMS)))
   return;

 LIST_COMS_2CHECK_FLAG_START(p2->saved, p->saved, TRUE);
 if (LIST_COMS_2CHECK1_FLAG_DO(wakes))
 {
  fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking wakes "
                "from you.\n", p2->saved->name);
  return;
 }
 if (LIST_COMS_2CHECK2_FLAG_DO(wakes))
 {
  fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                "to wake you.\n", p2->saved->name);
  return;
 }
 LIST_COMS_2CHECK_FLAG_END();
 
 fvtell_player(TALK_FT(OUTPUT_VARIABLES_NO_CHECKS, p->saved, p2),
               "^B%s!!!!!!!!!! OI !!!!!!!!!!! WAKE UP, %s"
               " wants to speak to you.$Bell^N\n",
               SHOW_PERSONAL(p2, "> "),
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))");
 
 fvtell_player(TALK_T(p2->saved, p),
               " You scream loudly at %s in an attempt to wake %s up.\n",
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               gender_choose_str(p2->gender, "him", "her", "them", "it"));

 check_receive_state(p->saved, p2);
}

static int internal_shout_emote(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 char *str = va_arg(ap, char *);
 int *count = va_arg(ap, int *);

 if (scan == p)
   return (TRUE);
 
 LIST_COMS_2CHECK_FLAG_START(scan->saved, p->saved, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(shouts))
   return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();

 ++*count;
 
 fvtell_player(TALK_T(p->saved, scan),
               "%s%s%s%s%.*s%s\n",
               USER_COLOUR_SHOUTS, SHOW_SHOUTS(scan, "! "),
               "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
               " t($F-Name_full) f($F-Name))",
               isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str), "^N");
 
 return (TRUE);
}

void user_shout_emote(player *p, const char *str)
{
 Timer_q_node *current_timer = NULL;
 int count = 0;
 
 if (!*str)
   TELL_FORMAT(p, "<msg>");
 
 if ((current_timer = timer_q_find_data(&timer_queue_player_no_shout,
                                        p->saved)))
 {
  char buf[256];
  struct timeval *tv = NULL;
  
  gettimeofday(&now_timeval, NULL);

  timer_q_cntl_node(current_timer, TIMER_Q_CNTL_NODE_GET_TIMEVAL, &tv);
  
  fvtell_player(NORMAL_T(p), 
                " You have been prevented from emote shouting for the "
                "next %s.\n",
                word_time_long(buf, sizeof(buf),
                               timer_q_timeval_udiff_secs(&now_timeval, tv),
                               WORD_TIME_ALL));
  return;
 }

 MASK_COMS_P(p, str, emote);
 
 do_inorder_logged_on(internal_shout_emote, p, str, &count);

 if (count)
   fvtell_player(TALK_T(p->saved, p),
                 "%s You emote '%s%s%.*s' to everyone.%s\n",
                 USER_COLOUR_MINE,
                 "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                 " t($F-Name_full) f($F-Name))",
                 isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str), "^N");
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " No-one is listening to your ^S^Bshout emotes^s.\n");

}

static const char *shout_ask_yell_me(player *p, const char *str, size_t length)
{
 IGNORE_PARAMETER(p);
 
 if (!length)
   ++length;
 
 switch (str[length - 1])
 {
  case '!':
    return ("yell");
    
  case '?':
    return ("shout, asking");
    
  default:
    return ("shout");
 }
}

static const char *shout_ask_yell_group(player *p, const char *str,
                                        size_t length)
{
 if (p->gender == GENDER_PLURAL)
   return (shout_ask_yell_me(p, str, length));

 if (!length)
   ++length;

 switch (str[length - 1])
 {
  case '!':
    return ("yells");
  
  case '?':
    return ("shouts, asking");
    
  default:
    break;
 }
 
 return ("shouts");
}

static int internal_shout_say(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *mid = va_arg(ap, const char *);
 const char *str = va_arg(ap, const char *);
 int *count = va_arg(ap, int *);

 if (p == scan)
   return (TRUE);
 
 LIST_COMS_2CHECK_FLAG_START(scan->saved, p->saved, TRUE);
 if (LIST_COMS_2CHECK_FLAG_DO(shouts))
 return (TRUE);
 LIST_COMS_2CHECK_FLAG_END();

 ++*count;
 
 fvtell_player(TALK_T(p->saved, scan), "%s%s%s %s '%.*s%s'.%s\n",
               USER_COLOUR_SHOUTS, SHOW_SHOUTS(scan, "! "),
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               mid, OUT_LENGTH_COMMUNICATION, str,
               USER_COLOUR_SHOUTS, "^N");
 
 return (TRUE);
}

void user_shout_say(player *p, const char *str, size_t length)
{
 Timer_q_node *current_timer = NULL;
 const char *mid = NULL;
 const char *mid_me = NULL;
 int count = 0;
 
 if (!*str)
   TELL_FORMAT(p, "<msg>");
  
 if ((current_timer = timer_q_find_data(&timer_queue_player_no_shout,
                                        p->saved)))
 {
  char buf[256];
  struct timeval *tv = NULL;
  
  gettimeofday(&now_timeval, NULL);

  timer_q_cntl_node(current_timer, TIMER_Q_CNTL_NODE_GET_TIMEVAL, &tv);
  
  fvtell_player(NORMAL_T(p), 
                " You have been prevented from shouting for the "
                "next %s.\n",
                word_time_long(buf, sizeof(buf),
                               timer_q_timeval_udiff_secs(&now_timeval, tv),
                               WORD_TIME_ALL));
  return;
 }

 mid = shout_ask_yell_group(p, str, length);
 mid_me = shout_ask_yell_me(p, str, length);
 
 MASK_COMS_P(p, str, say);
 
 do_inorder_logged_on(internal_shout_say, p, mid, str, &count);

 if (count)
   fvtell_player(NORMAL_T(p), "%s You %s '%.*s%s' to everyone.%s\n", 
                 USER_COLOUR_MINE, mid_me,
                 OUT_LENGTH_COMMUNICATION, str,
                 USER_COLOUR_MINE, "^N");
 else
   fvtell_player(SYSTEM_T(p), "%s",
                 " No-one is listening to your ^S^Bshouts^s.\n");
}

static void check_other_error_flags_tell_and_remote(multi_return *values,
						    player *p,
						    const char *str)
{
 switch (values->error_number)
 {
 case MULTI_BAD_IGNORE:
  fvtell_player(SYSTEM_T(p),
		" You can't %s or add to multi %d as you're blocking "
		"it.\n",
		str, values->error_multi);
  return;
 case MULTI_STOPPED_ALREADY:
  fvtell_player(SYSTEM_T(p),
		" You cannot add to multi %d as it has been stopped.\n",
		values->error_multi);
  return;
 case MULTI_BAD_EVERYONE_SELECTION:
  fvtell_player(SYSTEM_T(p),
		" You cannot %s everyone something whilst you're wearing "
		"earmuffs.\n", str);
  return;
 case MULTI_BAD_EVERYONE_FIND:
  fvtell_player(SYSTEM_T(p),
		" You cannot %s everyone as you're all alone.\n", str);
  return;
 case MULTI_BAD_MULTI_FIND:
  fvtell_player(SYSTEM_T(p),
		" There is no one on multi %d.\n",
		values->error_multi);
  return;
 case MULTI_BAD_MULTI_SELECTION:
  fvtell_player(SYSTEM_T(p),
		" You can't %s or add to multi %u as you're not on it.\n",
		str, values->error_multi);
  return;    
 case MULTI_BAD_NAME_FIND:
 case MULTI_BAD_FRIENDS_OF_NAME_FIND:
   /* fvtell_player(SYSTEM_T(p),
      " The string -- ^S^B%s^s -- does not match"
      " the start of any players name.\n",
      values->error_name); */
   return;
 case MULTI_BAD_NAME_SELECTION:
   fvtell_player(NORMAL_T(p), "%s", " ^S^BPlayer names^s are not allowed.\n");
  return;
 case MULTI_NO_PEOPLE_ADDED:
   fvtell_player(SYSTEM_T(p), "%s",
                 " No one has been found that matches your user list.\n");
  return;
 case MULTI_BAD_MINISTER_SELECTION:
 case MULTI_BAD_SU_SELECTION:
 case MULTI_BAD_NEWBIE_SELECTION:
  fvtell_player(SYSTEM_T(p),
		" You do not have the privs to %s to all the %s's as a "
		"group.\n", str, values->error_name);
  return;
 case MULTI_BAD_MINISTER_FIND:
 case MULTI_BAD_SU_FIND:
 case MULTI_BAD_NEWBIE_FIND:
  fvtell_player(SYSTEM_T(p),
		" There are no %s's logged on at the moment.\n",
		values->error_name);
  return;
 case MULTI_BAD_FRIENDS_SELECTION:
  fvtell_player(SYSTEM_T(p),
		" You cannot do %ss to friends whilst blocking them.\n",
		str);
  return;
 case MULTI_BAD_FRIENDS_FIND:
   fvtell_player(SYSTEM_T(p), "%s",
                 " None of your friends are logged on.\n");
  return;
 case MULTI_BAD_FRIENDS_OF_SELECTION:
  fvtell_player(SYSTEM_T(p),
		" You can't %s to the '%s' as you're not on their "
		"friends list.\n", str, values->error_name);
  return;
 case MULTI_BAD_FRIENDS_OF_FIND:
  fvtell_player(SYSTEM_T(p),
		" You can't %s to the '%s' as none of them are logged on.\n",
		str, values->error_name);
  return;
 case MULTI_BAD_ROOM_SELECTION:
  fvtell_player(SYSTEM_T(p),
		" You cannot %s to the whole room at the moment.\n",
		str);
  return;
 case MULTI_BAD_ROOM_FIND:
   fvtell_player(SYSTEM_T(p), "%s",
                 " There is no one in the room at the moment.\n");
  return;
 
 default:
   fvtell_player(SYSTEM_T(p), " Error occured in the %s (%u).\n", str,
	       values->error_number);
 }
}

static int internal_tell_player_find_msg(int type, player *p,
                                         const char *name,
                                         const char *player_name)
{
 IGNORE_PARAMETER(name && player_name);
 
 switch (type)
 {
  case PLAYER_FIND_MSG_TYPE_NO_SELF:
    fvtell_player(SYSTEM_T(p), "%s", " You can't speak to ^S^Byourself^s!\n");
    return (TRUE);
    
  default:
    break;
 }
 
 return (FALSE);
}

static int internal_remote_player_find_msg(int type, player *p,
                                           const char *name,
                                           const char *player_name)
{
 IGNORE_PARAMETER(name && player_name);

 switch (type)
 {
  case PLAYER_FIND_MSG_TYPE_NO_SELF:
    fvtell_player(SYSTEM_T(p), "%s", " You can't emote to ^S^Byourself^s!\n");
    return (TRUE);
    
  default:
    break;
 }

 return (FALSE);
}

static int internal_recho_player_find_msg(int type, player *p,
                                          const char *name,
                                          const char *player_name)
{
 IGNORE_PARAMETER(name && player_name);
 
 switch (type)
 {
  case PLAYER_FIND_MSG_TYPE_NO_SELF:
    fvtell_player(SYSTEM_T(p), "%s", " You can't echo to ^S^Byourself^s!\n");
    return (TRUE);
    
  default:
    break;
 }

 return (FALSE);
}

static int inorder_tell(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 const char *str = va_arg(va, const char *);
 size_t length = va_arg(va, size_t);
 const char *multi_num = multi_get_number(entry);
 int check_flags = va_arg(va, int);
 int get_name_flags = va_arg(va, int);

 LIST_COMS_2CHECK_FLAG_START(entry->parent, p->saved, TRUE);
 if (LIST_COMS_2CHECK1_FLAG_DO(tells))
 {
  if (check_flags)
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking tells "
                  "from you.\n", entry->parent->name);
  return (TRUE);
 }
 if (LIST_COMS_2CHECK2_FLAG_DO(tells))
 {
  if (check_flags)
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                  "to tell to you.\n", entry->parent->name);
  return (TRUE);
 }
 LIST_COMS_2CHECK_FLAG_END();

 if (entry->parent == p->saved)
   fvtell_player(NORMAL_T(p), "%s%sYou %s %s '%.*s%s'%s\n",
                 USER_COLOUR_MINE,
                 (*multi_num ? multi_num : " "),
                 tell_ask_exclaim_me(p, str, length),
                 multi_get_names(entry->number, entry, p->saved, NULL, NULL,
                                 get_name_flags),
                 OUT_LENGTH_COMMUNICATION, str,
                 USER_COLOUR_MINE,
                 "^N");
 else
   fvtell_player(TALK_TP(entry->parent->player_ptr),
                 "%s%s%s %s %s '%.*s%s'%s\n",
                 USER_COLOUR_TELL,
                 (*multi_num ? multi_num : 
                  SHOW_PERSONAL(entry->parent->player_ptr, "> ")),
                 "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                 " t($F-Name_full) f($F-Name))",
                 tell_ask_exclaim_group(p, str, length),
                 multi_get_names(entry->number, entry, p->saved, NULL,
                                 NULL, get_name_flags),
                 OUT_LENGTH_COMMUNICATION, str,
                 USER_COLOUR_TELL,
                 "^N");
 
 return (TRUE);
}

void user_tell(player *p, const char *str, size_t length)
{
 const char *orig_str = str;
 multi_return *values = NULL;
 int check_flags = FALSE;
 int get_name_flags = MULTI_DEFAULT;
 unsigned int pf_offset = 0;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<player(s)> <msg>");
 length -= (str - orig_str);
 
 lower_case(GET_PARAMETER_STR(&params, 1));

 INTERCOM_USER_TELL(p, name, str, len);

 if (!strcmp(GET_PARAMETER_STR((&params), 1), "friends"))
 {
  user_tell_friends(p, str, length);
  return;
 }

 pf_offset = player_find_msg_add(internal_tell_player_find_msg);
 log_assert(pf_offset);
 
 values = multi_add(p->saved, GET_PARAMETER_STR(&params, 1),
                    MULTI_DIE_MATCH_GROUP |
                    MULTI_DIE_MATCH_NAME |
                    MULTI_DIE_MATCH_MULTI |
                    MULTI_DIE_EMPTY_GROUP, PLAYER_FIND_SC_COMS);

 player_find_msg_del(pf_offset);
 
 if (values->multi_number)
 {
  if (!multi_check_for_flag(values->multi_number, (MULTI_TO_FRIENDS|
                                                   MULTI_TO_FRIENDS_OF|
                                                   MULTI_TO_SUPERS|
                                                   MULTI_TO_MINISTERS|
                                                   MULTI_TO_ROOM|
                                                   MULTI_TO_EVERYONE|
                                                   MULTI_TO_NEWBIES)))
    check_flags = TRUE;
  
  if ((values->error_number == MULTI_CREATED) || values->players_added)
    get_name_flags = MULTI_NO_MULTIS;
  
  do_inorder_multi(inorder_tell, values->multi_number, MULTI_DEFAULT,
                   p, str, length, check_flags, get_name_flags);
  
  multi_cleanup(values->multi_number, MULTI_DEFAULT);
  return;
 }
 else if (values->error_number == MULTI_NOT_ENOUGH_PEOPLE)
 { /* just going to one person */
  player *p2 = values->single_player->player_ptr;
  
  if (p2)
  {
   LIST_COMS_2CHECK_FLAG_START(p2->saved, p->saved, TRUE);
   if (LIST_COMS_2CHECK1_FLAG_DO(tells))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                  "tells from you.\n", p2->saved->name);
    fvtell_player(SYSTEM_FT(RAW_OUTPUT, p), " -=> %s: %.*s\n", p2->saved->name,
                  OUT_LENGTH_COMMUNICATION, p2->ignore_msg);
    return;
   }
   if (LIST_COMS_2CHECK2_FLAG_DO(tells))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                  "to tell to you.\n", p2->saved->name);
    return;
   }
   LIST_COMS_2CHECK_FLAG_END();
   
   fvtell_player(TALK_T(p->saved, p2), "%s%s%s %s you '%.*s%s'%s\n",
                 USER_COLOUR_TELL, SHOW_PERSONAL(p2, "> "),
                 "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                 " t($F-Name_full) f($F-Name))",
                 tell_ask_exclaim_group(p, str, length),
                 OUT_LENGTH_COMMUNICATION, str, USER_COLOUR_TELL, "^N");
   
   fvtell_player(TALK_T(p2->saved, p),
                 "%s You %s %s '%.*s%s'%s\n",
                 USER_COLOUR_MINE, tell_ask_exclaim_me(p, str, length),
                 "$If( ==(1(N) 2($R-Set-Ign_prefix))"
                 " t($F-Name_full) f($F-Name))",
                 OUT_LENGTH_COMMUNICATION, str, USER_COLOUR_MINE, "^N");
   
   check_receive_state(p->saved, p2);
   return;
  }
 }
 
 check_other_error_flags_tell_and_remote(values, p, "tell");
}

static int inorder_remote(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 const char *str = va_arg(va, char *);
 const char *multi_num = multi_get_number(entry);
 int check_flags = va_arg(va, int);
 int get_name_flags = va_arg(va, int);

 LIST_COMS_2CHECK_FLAG_START(entry->parent, p->saved, TRUE);
 if (LIST_COMS_2CHECK1_FLAG_DO(tells))
 {
  if (check_flags)
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                  "remotes from you.\n", entry->parent->name);
  return (TRUE);
 }
 if (LIST_COMS_2CHECK2_FLAG_DO(tells))
 {
  if (check_flags)
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                  "to remote to you.\n", entry->parent->name);
  return (TRUE);
 }
 LIST_COMS_2CHECK_FLAG_END();

 if (entry->parent == p->saved)
 {
  twinkle_info info;
  
  setup_twinkle_info(&info);
  
  info.output_not_me = TRUE;

  fvtell_player(TALK_IT(&info, p->saved, p),
                "%s%sYou emote '%s%s%.*s%s (to %s)'%s\n",
                USER_COLOUR_MINE,
                (*multi_num ? multi_num : " "),
                "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                " t($F-Name_full) f($F-Name))",
                isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str),
                USER_COLOUR_MINE, 
                multi_get_names(entry->number, entry, p->saved, NULL, NULL,
                                get_name_flags),
                "^N");
 }
 else
   fvtell_player(TALK_T(p->saved, entry->parent->player_ptr),
                 "%s%s%s%s%.*s%s (to %s)%s\n",
                 USER_COLOUR_TELL,
                 (*multi_num ? multi_num :
                  SHOW_PERSONAL(entry->parent->player_ptr, "> ")),
                 "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                 " t($F-Name_full) f($F-Name))",
                 isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str),
                 USER_COLOUR_TELL,
                 multi_get_names(entry->number, entry, p->saved, NULL,
                                 NULL, get_name_flags),
                 "^N");
 
 return (TRUE);
}

void user_remote_emote(player *p, const char *str, size_t length)
{
 const char *orig_str = str;
 multi_return *values = NULL;
 int check_flags = FALSE;
 int get_name_flags = MULTI_DEFAULT;
 unsigned int pf_offset = 0;
 parameter_holder params;

 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<player(s)> <msg>");
 length -= (str - orig_str);
 
 lower_case(GET_PARAMETER_STR((&params), 1));

 INTERCOM_USER_REMOTE(p, name, str, len);
 
 if (!strcmp(GET_PARAMETER_STR((&params), 1), "friends"))
 {
  user_remote_friends(p, str, length);
  return;
 }

 pf_offset = player_find_msg_add(internal_remote_player_find_msg);
 log_assert(pf_offset);

 values = multi_add(p->saved, GET_PARAMETER_STR(&params, 1),
                    MULTI_DIE_MATCH_GROUP |
                    MULTI_DIE_MATCH_NAME |
                    MULTI_DIE_MATCH_MULTI |
                    MULTI_DIE_EMPTY_GROUP, PLAYER_FIND_SC_COMS);

 player_find_msg_del(pf_offset);
 
 if (values->multi_number)
 {
  if (!multi_check_for_flag(values->multi_number, (MULTI_TO_FRIENDS|
						   MULTI_TO_FRIENDS_OF|
						   MULTI_TO_SUPERS|
						   MULTI_TO_MINISTERS|
						   MULTI_TO_ROOM|
						   MULTI_TO_EVERYONE|
						   MULTI_TO_NEWBIES)))
    check_flags = TRUE;

  if ((values->error_number == MULTI_CREATED) || values->players_added)
    get_name_flags = MULTI_NO_MULTIS;
  
  do_inorder_multi(inorder_remote, values->multi_number, MULTI_DEFAULT,
                   p, str, check_flags, get_name_flags);
  
  multi_cleanup(values->multi_number, MULTI_DEFAULT);
  return;
 }
 else if (values->error_number == MULTI_NOT_ENOUGH_PEOPLE)
 { /* just going to one person */
  player *p2 = values->single_player->player_ptr;

  if (p2)
  {
   LIST_COMS_2CHECK_FLAG_START(p2->saved, p->saved, TRUE);
   if (LIST_COMS_2CHECK1_FLAG_DO(tells))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                  "remotes from you.\n", p2->saved->name);
    fvtell_player(SYSTEM_FT(RAW_OUTPUT, p), " -=> %s: %.*s\n", p2->saved->name,
                  OUT_LENGTH_COMMUNICATION, p2->ignore_msg);
    return;
   }
   if (LIST_COMS_2CHECK2_FLAG_DO(tells))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                  "to remote to you.\n", p2->saved->name);
    return;
   }
   LIST_COMS_2CHECK_FLAG_END();
     
   fvtell_player(TALK_TP(p2), "%s%s%s%s%.*s%s\n",
                 USER_COLOUR_TELL,
                 SHOW_PERSONAL(p2, "> "),
                 "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                 " t($F-Name_full) f($F-Name))",
                 isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str), "^N");

   fvtell_player(TALK_T(p->saved, p),
                 "%s You emote '%s%s%.*s%s' to %s.%s\n",
                 USER_COLOUR_MINE,
                 "$If( ==(1(N) 2($R-Set-Ign_eprefix))"
                 " t($F-Name_full) f($F-Name))",
                 isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str),
                 USER_COLOUR_MINE, p2->saved->name, "^N");
     
   check_receive_state(p->saved, p2);
   return;
  }
 }
 
 check_other_error_flags_tell_and_remote(values, p, "remote");
}

static int inorder_recho(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 const char *str = va_arg(va, char *);
 const char *multi_num = multi_get_number(entry);
 int check_flags = va_arg(va, int);
 int get_name_flags = va_arg(va, int);
 const char *name_buffer = va_arg(va, char *);

 LIST_COMS_2CHECK_FLAG_START(entry->parent, p->saved, TRUE);
 if (LIST_COMS_2CHECK1_FLAG_DO(tells))
 {
  if (check_flags)
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                  "remotes from you.\n", entry->parent->name);
  return (TRUE);
 }
 if (LIST_COMS_2CHECK2_FLAG_DO(tells))
 {
  if (check_flags)
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                  "to remote to you.\n", entry->parent->name);
  return (TRUE);
 }
 if (LIST_COMS_2CHECK1_FLAG_DO(echos))
 { 
  if (check_flags)
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking echos "
                  "from you.\n", entry->parent->name);
  return (TRUE);
 }
 if (LIST_COMS_2CHECK2_FLAG_DO(echos))
 { 
  if (check_flags)
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                  "to echo to you.\n", entry->parent->name);
  return (TRUE);
 }
 LIST_COMS_2CHECK_FLAG_END();

 if (entry->parent == p->saved)
 {
  fvtell_player(TALK_TP(p), "%s%sYou echo '%.*s%s' to %s.%s\n",
                USER_COLOUR_MINE,
                (*multi_num ? multi_num : " "),
                OUT_LENGTH_COMMUNICATION, str,
                USER_COLOUR_MINE,
                multi_get_names(entry->number, entry, p->saved, NULL, NULL,
                                get_name_flags), "^N");
  
 }
 else
 {
  if (*multi_num)
    fvtell_player(TALK_TP(entry->parent->player_ptr),
                  "%s%s%.*s%s (to %s)%s\n", 
                  USER_COLOUR_ECHO,
                  multi_num,
                  OUT_LENGTH_COMMUNICATION, str,
                  USER_COLOUR_ECHO,
                  multi_get_names(entry->number, entry, p->saved, NULL,
                                  NULL, get_name_flags),
                  "^N");
  else
    fvtell_player(TALK_TP(entry->parent->player_ptr),
                  "%s%s%s%.*s%s (to %s)%s\n", 
                  USER_COLOUR_ECHO,
                  SHOW_ECHO(entry->parent->player_ptr, ">+ ", name_buffer),
                  OUT_LENGTH_COMMUNICATION, str,
                  USER_COLOUR_ECHO,
                  multi_get_names(entry->number, entry, p->saved, NULL,
                                  NULL, get_name_flags),
                  "^N");    
 }

 return (TRUE);
}

void user_remote_echo(player *p, const char *str)
{
 multi_return *values = NULL;
 int check_flags = FALSE;
 int get_name_flags = MULTI_DEFAULT;
 unsigned int pf_offset = 0;
 parameter_holder params;
 char name_buffer[PLAYER_S_NAME_SZ + 3];
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) || !*str)
   TELL_FORMAT(p, "<player(s)> <msg>");
 
 pf_offset = player_find_msg_add(internal_recho_player_find_msg);
 log_assert(pf_offset);

 values = multi_add(p->saved, GET_PARAMETER_STR(&params, 1),
                    MULTI_DIE_MATCH_GROUP |
                    MULTI_DIE_MATCH_NAME |
                    MULTI_DIE_MATCH_MULTI |
                    MULTI_DIE_EMPTY_GROUP, PLAYER_FIND_SC_COMS);

 player_find_msg_del(pf_offset);
 
 sprintf(name_buffer, "[%s] ", p->saved->name);
 
 if (values->multi_number)
 {
  if (!multi_check_for_flag(values->multi_number, (MULTI_TO_FRIENDS|
						   MULTI_TO_FRIENDS_OF|
						   MULTI_TO_SUPERS|
						   MULTI_TO_MINISTERS|
						   MULTI_TO_ROOM|
						   MULTI_TO_EVERYONE|
						   MULTI_TO_NEWBIES)))
    check_flags = TRUE;

  if ((values->error_number == MULTI_CREATED) || values->players_added)
    get_name_flags = MULTI_NO_MULTIS;
  
  do_inorder_multi(inorder_recho, values->multi_number, MULTI_DEFAULT,
                   p, str, check_flags, get_name_flags, name_buffer);
  
  multi_cleanup(values->multi_number, MULTI_DEFAULT);
  return;
 }
 else if (values->error_number == MULTI_NOT_ENOUGH_PEOPLE)
 { /* just going to one person */
  player *p2 = values->single_player->player_ptr;
  
  if (p2)
  {
   LIST_COMS_2CHECK_FLAG_START(p2->saved, p->saved, TRUE);
   if (LIST_COMS_2CHECK1_FLAG_DO(tells))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                  "remotes from you.\n", p2->saved->name);
    fvtell_player(SYSTEM_FT(RAW_OUTPUT, p), " -=> %s: %.*s\n", p2->saved->name,
                  OUT_LENGTH_COMMUNICATION, p2->ignore_msg);
    return;
   }
   if (LIST_COMS_2CHECK2_FLAG_DO(tells))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                  "to remote to you.\n", p2->saved->name);
    return;
   }
   if (LIST_COMS_2CHECK1_FLAG_DO(echos))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                  "echos from you.\n", p2->saved->name);
    fvtell_player(SYSTEM_FT(RAW_OUTPUT, p), " -=> %s: %.*s\n", p2->saved->name,
                  OUT_LENGTH_COMMUNICATION, p2->ignore_msg);
    return;
   }
   if (LIST_COMS_2CHECK2_FLAG_DO(echos))
   {
    fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is not allowed "
                  "to echo to you.\n", p2->saved->name);
    return;
   }
   LIST_COMS_2CHECK_FLAG_END();
   
   fvtell_player(TALK_TP(p), "%s You echo '%.*s%s' to %s.%s\n",
                 USER_COLOUR_MINE, OUT_LENGTH_COMMUNICATION, str,
                 USER_COLOUR_MINE, p2->saved->name, "^N");
   fvtell_player(TALK_TP(p2), "%s%s%s%.*s%s\n", 
                 USER_COLOUR_ECHO,
                 SHOW_ECHO(p2, ">+ ", name_buffer),
                 OUT_LENGTH_COMMUNICATION, str, "^N");
   
   check_receive_state(p->saved, p2);
   return;
  }
 }
 
 check_other_error_flags_tell_and_remote(values, p, "recho");
}

static int internal_su_warn(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 const char *msg = va_arg(va, const char *);
 int *count = va_arg(va, int *);
 
 if (p->saved == entry->parent)
 {
  if (multi_check_node_for_flag(entry, MULTI_TO_SELF))
  {
   ++*count;
   fvtell_player(SYSTEM_T(p), "%s", /* does this count as an easter egg ? */
                 " ^S^BEaster Egg^s: You've just warned yourself.\n");
   user_logoff(p, "");
  }
  return (TRUE);
 }

 ++*count;
 fvtell_player(SYSTEM_FT(HILIGHT | SYSTEM_INFO, entry->parent->player_ptr),
               "\n$Bell -=> %s%s warn%s you: ^S^B%s^s\n\n",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name,
               (p->gender == GENDER_PLURAL) ? "" : "s", msg);

 return (TRUE);
}

void user_su_warn(player *p, const char *str)
{
 multi_return *values = NULL;
 int count = 0;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1))
   TELL_FORMAT(p, "<player(s)> [msg]");

 CHECK_DUTY(p);
 
 if (!*str)
   str = "Stop being annoying.";
 
 values = multi_add(p->saved, GET_PARAMETER_STR(&params, 1),
                    (MULTI_VERBOSE |
                     MULTI_LIVE_ON_SMALL |
                     MULTI_NO_DO_MATCH |
                     MULTI_NO_GROUPS |
                     MULTI_NO_COMS_CHECKS |
                     MULTI_MUST_CREATE |
                     MULTI_DESTROY_CLEANUP) & ~MULTI_NO_NAMES,
                    PLAYER_FIND_SC_SU_ALL);
 
 if (values->multi_number)
 {
  do_inorder_multi(internal_su_warn, values->multi_number, MULTI_DEFAULT,
                   p, str, &count);

  if ((count - multi_check_for_flag(values->multi_number, MULTI_TO_SELF)) > 0)
  {
   vwlog("warn", "%s warn%s %s: %s", p->saved->name,
         (p->gender == GENDER_PLURAL) ? "" : "s",
         multi_get_names(values->multi_number, NULL, p->saved, NULL, NULL,
                         MULTI_NO_MULTIS), str);
   
   fvtell_player(NORMAL_T(p), " You warn ^S^B%s^s: %s%s\n",
                 multi_get_names(values->multi_number, NULL, p->saved,
                                 NULL, NULL, MULTI_NO_MULTIS), str, "^N");
   
   channels_wall("staff", 3, p, " -=> %s warn%s %s: %s",
                 p->saved->name, (p->gender == GENDER_PLURAL) ? "" : "s",
                 multi_get_names(values->multi_number, NULL, p->saved,
                                 NULL, NULL, MULTI_NO_MULTIS),
                 str);
  }
  else
    fvtell_player(SYSTEM_T(p), "%s", " That warn didn't goto anyone.\n");
  
  multi_cleanup(values->multi_number, MULTI_DEFAULT);
 }
}

void user_su_wall(player *p, const char *str)
{
 if (!*str)
   TELL_FORMAT(p, "<message>");
 
 CHECK_DUTY(p);

 vwlog("wall", " %s screams -=> %s <=-\n", p->saved->name, str);

 sys_wall(0, "^B %s screams -=> %s^B <=-$Bell^N\n", p->saved->name, str);
}

void cmds_init_multi_communication(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("atell", user_su_tell, CHARS_SIZE_T, ADMIN);
 CMDS_PRIV(admin);
 CMDS_ADD("emote_shout", user_shout_emote, CONST_CHARS, COMMUNICATION);
 CMDS_ADD("recho", user_remote_echo, CONST_CHARS, COMMUNICATION);
 CMDS_PRIV(command_echo);
 CMDS_ADD("remote", user_remote_emote, CHARS_SIZE_T, COMMUNICATION);
 CMDS_ADD("shout", user_shout_say, CHARS_SIZE_T, COMMUNICATION);
 CMDS_ADD("tell", user_tell, CHARS_SIZE_T, COMMUNICATION);
 CMDS_ADD("wake", user_wake, CONST_CHARS, COMMUNICATION);
 CMDS_ADD("wall", user_su_wall, CONST_CHARS, SU);
 CMDS_PRIV(coder_senior_su);
 CMDS_ADD("warn", user_su_warn, CONST_CHARS, SU);
 CMDS_PRIV(command_warn);
}
