#define COMMANDS_C
/*
 *  Copyright (C) 1999, 2000 James Antill, John Tobin
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


/* functions... */

Timer_q_base scripting_timer_queue;


static void user_set_age(player *p, const char *str)
{
 int new_age;
 
 if (!*str)
   TELL_FORMAT(p, "<number>");
 
 new_age = atoi(str);
 if (new_age < 0)
 {
  fvtell_player(NORMAL_T(p), "%s", " You can't be of a negative age !\n");
  return;
 }
 
 if ((p->age = new_age))
   fvtell_player(NORMAL_T(p), " Your age is now set to %d years old.\n",
                 p->age);
 else
   fvtell_player(NORMAL_T(p),
                 " You have turned off your age so no-one can see it.\n");

 if (p->flag_use_birthday_as_age)
 {
  fvtell_player(NORMAL_T(p),
                " Your age isn't automatically worked out anymore"
                " from your birthday.\n");
  p->flag_use_birthday_as_age = FALSE;
 }
}

static void user_set_birthday(player *p, const char *str)
{
 const char *tmp = str;
 struct tm *birth_tm = gmtime(&now);
 
 if (!*tmp)
   TELL_FORMAT(p, "<day/month[/year] | off>");

 if (!strcasecmp(tmp, "off"))
 {
  fvtell_player(NORMAL_T(p), "%s", " Birthday cleared.\n");
  p->birthday = 0;
  return;
 }

 birth_tm->tm_mday = skip_atoi(&tmp);
 if ((birth_tm->tm_mday <= 0) || (birth_tm->tm_mday > 31))
 {
  fvtell_player(NORMAL_T(p), "%s", " Not a valid day of the month.\n");
  return;
 }

 if (*tmp)
   tmp++;
 
 birth_tm->tm_mon = skip_atoi(&tmp);
 if ((birth_tm->tm_mon <= 0) || (birth_tm->tm_mon > 12))
 {
  fvtell_player(NORMAL_T(p), "%s", " Not a valid month.\n");
  return;
 }
 --birth_tm->tm_mon;
 
 if (*tmp)
   tmp++;
 
 birth_tm->tm_year = atoi(tmp);
 if (birth_tm->tm_year > 1900)
   birth_tm->tm_year -= 1900;
 
 birth_tm->tm_sec = 5;
 birth_tm->tm_hour = 1;
 birth_tm->tm_min = 1;
 if (birth_tm->tm_year < 3)
 {
  time_t b_day;
  
  birth_tm->tm_year = 3; /* can't use < 3 as that's shagged on Linux libc5
                          * and probably others ... *sighs*/

  if ((b_day = mktime(birth_tm)) == -1)
  {
   fvtell_player(NORMAL_T(p), "%s", " That is not a valid birthday.\n");
   return;
  }
  p->birthday = b_day;
  
  p->flag_birthday_show_year = FALSE;
  fvtell_player(NORMAL_T(p), " Your birthday is set to the %s.\n",
		disp_date_birthday_string(p->birthday, FALSE));
 }
 else
 {
  time_t b_day;
  
  if ((b_day = mktime(birth_tm)) == -1)
  {
   fvtell_player(NORMAL_T(p), "%s", " That is not a valid birthday.\n");
   return;
  }
  p->birthday = b_day;

  p->flag_birthday_show_year = TRUE;
  fvtell_player(NORMAL_T(p), " Your birthday is set to the %s.\n",
		disp_date_birthday_string(p->birthday, TRUE));
 }

 if (!p->flag_use_birthday_as_age)
   fvtell_player(NORMAL_T(p),
                 " Your age isn't automatically worked out"
                 " from your birthday.\n");
}

static void user_su_system_com(player *p, const char *str)
{
 if (p->saved->priv_coder && p->saved->priv_admin)
   system(str);
 else
   fvtell_player(NORMAL_T(p), "%s", " System is up.\n");
}

static void user_recapitalise_name(player *p, const char *str)
{
 player *p2 = p;
 
 if (!*str)
   TELL_FORMAT(p, "<name>");
 
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL)))
   return;
 
 if ((p != p2) && !p->saved->priv_normal_su)
 {
  fvtell_player(NORMAL_T(p), "%s", " You are only privileged to "
                "recapitialise ^S^B_your_^s name.\n");
  return;
 }
 
 qstrcpy(p2->saved->name, str);
 fvtell_player(NORMAL_T(p), " Name changed to '%s'\n", p2->saved->name);

 marriage_update_spouce_name(p2);
}

static void user_resident_count_show(player *p)
{
 fvtell_player(NORMAL_T(p),
               " $Talker-Name current resident count is ^B%d^N.\n",
               no_of_resis);
}

static int inorder_finger(multi_node *entry, va_list va)
{
 player *p = va_arg(va, player *);
 
 if ((p->saved != entry->parent) ||
     multi_check_node_for_flag(entry, MULTI_TO_SELF))
 {
  if (P_IS_ON(entry->parent))
    fvtell_player(TALK_T(entry->parent, p),
                  " %s is logged on.\n",
                  entry->parent->name);
  else
    fvtell_player(TALK_T(entry->parent, p),
                   " %s was last seen %s.\n",
                  entry->parent->name,
                  DISP_TIME_P_STD(entry->parent->logoff_timestamp, p));
 }
 
 return (TRUE);
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
                  " You cannot ^S^B%s^s everyone as you're all alone.\n", str);
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
       " The name -- %s -- is not a valid name.\n",
       values->error_name); */
    return; /* multi verbose */
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
    fvtell_player(SYSTEM_T(p), " Error occured in ^S^B%s^s (%u).\n",
                  str, values->error_number);
 }
}

static void user_multi_finger(player *p, const char *str)
{
 multi_return *values = NULL;

 parameter_holder params;

 get_parameter_init(&params);

 if (!get_parameter_parse(&params, &str, 1))
   TELL_FORMAT(p, "<player(s)>");
 
 values = multi_add(p->saved, GET_PARAMETER_STR(&params, 1),
                    MULTI_USE_NOT_ON |
                    MULTI_LIVE_ON_SMALL |
                    MULTI_NO_COMS_CHECKS |
                    MULTI_MUST_CREATE |
                    MULTI_DESTROY_CLEANUP, PLAYER_FIND_SC_EXTERN_ALL);
 
 if (values->multi_number)
 {
  ptell_mid(NORMAL_T(p), "Multi finger", FALSE);
  do_inorder_multi(inorder_finger, values->multi_number, MULTI_SHOW_ALL, p);
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
  
  multi_cleanup(values->multi_number, MULTI_DEFAULT);
 }
 else
   if (values->error_number == MULTI_NOT_ENOUGH_PEOPLE)
   {
    player_tree_node *p2_saved = values->single_player;
#ifdef USE_NORMAL_FINGER_IN_MULTI_FINGER
    char buffer[PLAYER_S_NAME_SZ];
    /* what is better more info or consistancy ? --
     * info might be nice for ppl,
     * but consistancy would be better if we do grep */
    
    strcpy(buffer, p2_saved->lower_name);
    user_finger(p, buffer);
#else
    if (P_IS_ON(p2_saved))
      fvtell_player(TALK_T(p2_saved, p),
                    " %s is logged on.\n",
                    p2_saved->name);
    else
      fvtell_player(TALK_T(p2_saved, p),
                    " %s was last seen %s.\n",
                    p2_saved->name,
                    DISP_TIME_P_STD(p2_saved->logoff_timestamp, p));
#endif
   }
   else
     check_other_error_flags_tell_and_remote(values, p, "finger");

 pager(p, PAGER_DEFAULT);
}

static void internal_display_birthday_age(player *p, player *p2)
{
 int tmp = real_age(p2);
 
 if (tmp) /* might want a seperate flag, so we can have a 0 age */
   if (p2->birthday)
     fvtell_player(INFO_T(p2->saved, p),
                   "%s%s %s ^S^B%d years old^s and %s birthday %s on "
                   "the ^S^B%s^s.\n",
                   gender_choose_str(p2->gender, "", "", "The ", "The "),
                   p2->saved->name,
                   (p2->gender == GENDER_PLURAL) ? "are" : "is",
                   tmp,
                   gender_choose_str(p2->gender, "his", "her", "their", "its"),
                   (p2->gender == GENDER_PLURAL) ? "are" : "is",
                   disp_date_birthday_string(p2->birthday, 
                                             p2->flag_birthday_show_year));
   else
     fvtell_player(INFO_T(p2->saved, p),
                   "%s%s %s ^S^B%d years old^s.\n",
                   gender_choose_str(p2->gender, "", "", "The ", "The "),
                   p2->saved->name,
                   (p2->gender == GENDER_PLURAL) ? "are" : "is", tmp);
 else if (p2->birthday) /* FIXME: broken */
   fvtell_player(INFO_T(p2->saved, p),
                 "%s birthday %s on the ^S^B%s^s.\n",
                 gender_choose_str(p2->gender, "His", "Her", "Their", "Its"),
                 (p2->gender == GENDER_PLURAL) ? "are" : "is",
                 disp_date_birthday_string(p2->birthday, FALSE));
}

void user_finger(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!*str)
   TELL_FORMAT(p, "<player>");

 INTERCOM_USER_FINGER(p, str);
 
 if (!strcasecmp(str, "friends"))
 {
  char friend_string[] = "friends";
  
  user_multi_finger(p, friend_string);
  return;
 }
 
 if (!(p2 = player_find_load(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
   return;
 
 ptell_mid(INFO_T(p2->saved, p), "Finger info for $F-Name", FALSE);
 if (p->see_raw_twinkles)
   fvtell_player(INFO_T(p2->saved, p), "%s%s%s^N\n",
                 p2->saved->name, isits1(p2->title), isits2(p2->title));
 else
   fvtell_player(INFO_T(p2->saved, p), "%s%s%.*s^N\n",
                 p2->saved->name,
                 isits1(p2->title), OUT_LENGTH_TITLE, isits2(p2->title));   
 
 if ((p->saved->priv_coder || p->saved->priv_basic_su) && p2->su_comment[0])
 {
  ptell_mid(INFO_T(p2->saved, p), "su comment", FALSE);
  fvtell_player(INFO_T(p2->saved, p), "%s^N\n", p2->su_comment);
 }
 
 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
 
 if (p2->is_fully_on)
 {
  char buf[256];
  
  fvtell_player(INFO_T(p2->saved, p),
                "%s%s %s been logged in for ^S^B%s^s,\n that is since %s.\n",
                gender_choose_str(p2->gender, "", "", "The ", "The "),
                "$F-Name_full",
                (p2->gender == GENDER_PLURAL) ? "have" : "has",
                word_time_long(buf, sizeof(buf),
                               difftime(now, p2->logon_timestamp),
                               WORD_TIME_DEFAULT),
                DISP_TIME_P_STD(p2->logon_timestamp, p));
 }
 else if (p2->saved->priv_base)
 {
  fvtell_player(INFO_T(p2->saved, p),
                "%s%s %s last seen ^S^B%s^s.\n",
                gender_choose_str(p2->gender, "", "", "The ", "The "),
                p2->saved->name,
                (p2->gender == GENDER_PLURAL) ? "were" : "was",
                DISP_TIME_P_STD(p2->saved->logoff_timestamp, p));
 }
 
 if ((p->gmt_offset != p2->gmt_offset) && !p2->flag_gmt_offset_hide)
 {
  char buf[256];
  
  fvtell_player(INFO_T(p2->saved, p),
                "%s%s %s in a timezone ^S^B%s %s^s you.\n",
                gender_choose_str(p2->gender, "", "", "The ", "The "),
                p2->saved->name, (p->gender == GENDER_PLURAL) ? "are" : "is",
                word_time_long(buf, sizeof(buf),
                               abs(p->gmt_offset - p2->gmt_offset),
                               WORD_TIME_DEFAULT),
                (p->gmt_offset > p2->gmt_offset) ? ("behind") : ("ahead of"));
 }
 
 if (!p2->saved->flag_hide_logon_time || PRIV_STAFF(p->saved) || (p == p2))
 {
  char buf[256];
  
  fvtell_player(INFO_T(p2->saved, p),
                "%s total logon time is ^S^B%s^s.%s",
                gender_choose_str(p2->gender, "His", "Her", "Their", "Its"),
                word_time_long(buf, sizeof(buf),
                               real_total_logon_time(p2->saved),
                               WORD_TIME_DEFAULT),
                (p2->saved->flag_hide_logon_time) ? " (Hidden)\n" : "\n");
 }
 else
   fvtell_player(INFO_T(p2->saved, p),
                 "%s total logon time is ^S^Bhidden^s.\n",
                 gender_choose_str(p2->gender, "His", "Her", "Their", "Its"));
 
 if (mail_check_mail_new(p2->saved))
   fvtell_player(INFO_T(p2->saved, p),
                 "%s %s ^S^Bnew mail^s.\n",
                 gender_choose_str(p2->gender, "He", "She", "They", "It"),
                 (p2->gender == GENDER_PLURAL) ? "have" : "has");

 if (p2->phone_numbers[0])
 {
  LIST_SELF_CHECK_FLAG_START(p2, p->saved);
  if ((p2 == p) || LIST_SELF_CHECK_FLAG_DO(friend))
    /* might want to chnage this to have it's own flag ? */
    fvtell_player(INFO_T(p2->saved, p), "Phone-numbers: %s^N\n",
                  p2->phone_numbers);
  LIST_SELF_CHECK_FLAG_END();
 }
 
 if (((p == p2) || p->saved->priv_senior_su ||
      !p2->saved->flag_private_email) &&
     p2->email[0])
 { /* might want to chnage this to be have a flag or use friends too ? */
  fvtell_player(INFO_T(p2->saved, p), "%s", "Email: ^S^B");
  fvtell_player(INFO_FT(RAW_OUTPUT, p2->saved, p), "%s", p2->email);
  fvtell_player(INFO_T(p2->saved, p), "%s%s\n",
                "^s", (p2->saved->flag_private_email ? " (private)" : ""));
 }
 
 if (p2->url[0])
   fvtell_player(INFO_T(p2->saved, p), "Url: %.*s^N\n",
                 OUT_LENGTH_INFO, p2->url);
 
 if (p2->plan[0])
 {
  twinkle_info info;
  
  setup_twinkle_info(&info);
  
  info.returns_limit = OUT_RETURNS_INFO;
  ptell_mid(INFO_T(p2->saved, p), "plan", FALSE);
  if (p->see_raw_twinkles)
    fvtell_player(INFO_T(p2->saved, p), "%s^N\n", p2->plan);
  else
    fvtell_player(ALL_T(p2->saved, p, &info, 0, now), "%.*s^N\n",
                  OUT_LENGTH_INFO, p2->plan);
 }
 
 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
} 

void user_examine(player *p, const char *str)
{
 player *p2 = NULL;
 char w_time[256];
 
 if (!*str)
   TELL_FORMAT(p, "<player>");

 INTERCOM_USER_EXAMINE(p, str);
  
 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
   return;
 
 ptell_mid(INFO_T(p2->saved, p), "Examine info for $F-Name", FALSE);
 
 if (p->see_raw_twinkles)
   fvtell_player(INFO_T(p2->saved, p), "%s%s%s^N\n",
                 p2->saved->name, isits1(p2->title), isits2(p2->title));
 else
   fvtell_player(INFO_T(p2->saved, p), "%s%s%.*s^N\n",
                 p2->saved->name,
                 isits1(p2->title), OUT_LENGTH_TITLE, isits2(p2->title));

 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
 
 if (*p2->description)
 {
  twinkle_info info;

  setup_twinkle_info(&info);
  info.returns_limit = OUT_RETURNS_INFO;

  if (p->see_raw_twinkles)
    fvtell_player(INFO_T(p2->saved, p), "%s^N\n", p2->description);
  else
    fvtell_player(ALL_T(p2->saved, p, &info, 0, now), "%.*s^N\n%s",
                  OUT_LENGTH_INFO, p2->description, DASH_LEN);
 }
 
 fvtell_player(INFO_T(p2->saved, p),
               "%s%s %s been logged on for ^S^B%s^s,\n that is since %s.\n",
               gender_choose_str(p2->gender, "", "", "The ", "The "),
               "$F-Name_full",
               (p2->gender == GENDER_PLURAL) ? "have" : "has",
               word_time_long(w_time, sizeof(w_time),
                              difftime(now, p2->logon_timestamp),
                              WORD_TIME_DEFAULT),
               DISP_TIME_P_STD(p2->logon_timestamp, p));

 if (!p2->saved->flag_hide_logon_time || PRIV_STAFF(p->saved) || (p == p2))
   fvtell_player(INFO_T(p2->saved, p),
                 "%s total logon time is ^S^B%s^s.%s\n",
		 gender_choose_str(p2->gender, "His", "Her", "Their", "Its"),
                 word_time_long(w_time, sizeof(w_time),
                                real_total_logon_time(p2->saved),
                                WORD_TIME_DEFAULT),
		 (p2->saved->flag_hide_logon_time ? " (Hidden)" : ""));
 else
   fvtell_player(INFO_T(p2->saved, p),
                 "%s total logon time has been ^S^Bhidden^s.\n",
                 gender_choose_str(p2->gender, "His", "Her", "Their", "Its"));
    
 if (*p2->married_to)
 {
  if (p2->flag_married && !p2->flag_marriage_hide)
    fvtell_player(INFO_T(p2->saved, p),
                  "%s is %smarried to %s.\n",
                  p2->saved->name,
                  (p2->flag_no_net_spouse ? "" : "net "),
                  p2->married_to);
  else
    if (!p2->flag_married && !p2->flag_marriage_hide)
      fvtell_player(INFO_T(p2->saved, p),
                    "%s wants to %smarry %s.\n", 
                    p2->saved->name,
                    (p2->flag_no_net_spouse ? "" : "net "),
                    p2->married_to);
 }

 internal_display_birthday_age(p, p2);
 
 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
}

static void user_personal_info(player *p, const char *str)
{
 player *p2 = p;
 
 if (*str && p->saved->priv_normal_su)
 {
  if (!(p2 = player_find_load(p, str, PLAYER_FIND_SC_SU_ALL)))
    return;
 }

 ptell_mid(INFO_T(p2->saved, p), "Personal info for $F-Name", FALSE);
 
 fvtell_player(INFO_FT(CONST_STRLEN("Enter-msg: "), p2->saved, p),
	       "Enter-msg: %s%s%s^N\n",
	       p2->saved->name, isits1(p2->enter_msg), isits2(p2->enter_msg));
 fvtell_player(INFO_FT(CONST_STRLEN("Prompt: "), p2->saved, p),
	       "Prompt: %s^N\n", p2->prompt);
 fvtell_player(INFO_FT(CONST_STRLEN("Conv-Prompt: "), p2->saved, p),
	       "Conv-Prompt: %s^N\n", p2->converse_prompt);

 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);

 if (*p2->prefix)
   fvtell_player(INFO_FT(CONST_STRLEN("Prefix: "), p2->saved, p), "%s",
                 "Prefix: $F-Name_full^N\n");
 else
   fvtell_player(INFO_FT(CONST_STRLEN("Prefix: "), p2->saved, p), "%s",
                 "Prefix:-** none **-\n");

 fvtell_player(INFO_FT(CONST_STRLEN("Title: "), p2->saved, p),
	       "Title: %s%s%s^N\n",
	       p2->saved->name, isits1(p2->title), isits2(p2->title));
 
 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
 
 if (*p2->ignore_msg)
   fvtell_player(INFO_FT(CONST_STRLEN("Ignore-msg: ") | RAW_OUTPUT,
                         p2->saved, p),
		 "Ignore-msg: %s\n", p2->ignore_msg);
 else
   fvtell_player(INFO_FT(CONST_STRLEN("Ignore-msg: ") | RAW_OUTPUT,
                         p2->saved, p), "%s", "Ignore-msg:-** none **-\n");

 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
 
 if (*p2->connect_msg)
   fvtell_player(INFO_FT(CONST_STRLEN("Connect-msg: "), p2->saved, p),
		 "Connect-msg: %s%s%s^N\n", p2->saved->name,
                 isits1(p2->connect_msg), isits2(p2->connect_msg));
 else
   fvtell_player(INFO_FT(CONST_STRLEN("Connect-msg: "), p2->saved, p), "%s",
                 "Connect-msg:-** none **-\n");

 if (*p2->disconnect_msg)
   fvtell_player(INFO_FT(CONST_STRLEN("Disconnect-msg: "), p2->saved, p),
		 "Disconnect-msg: %s%s%s^N\n", p2->saved->name,
                 isits1(p2->disconnect_msg), isits2(p2->disconnect_msg));
 else
   fvtell_player(INFO_FT(CONST_STRLEN("Disconnect-msg: "), p2->saved, p), "%s",
                 "Disconnect-msg:-** none **-\n");
 
 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
 
 if (*p2->phone_numbers)
   fvtell_player(INFO_FT(CONST_STRLEN("Phone-numbers: "), p2->saved, p),
		 "Phone-numbers: %s^N\n", p2->phone_numbers);
 else
   fvtell_player(INFO_FT(CONST_STRLEN("Phone-numbers: "), p2->saved, p), "%s",
                 "Phone-numbers:-** none **-\n");

 if (*p2->follow)
   fvtell_player(INFO_FT(CONST_STRLEN("Follow: "), p2->saved, p),
		 "Follow: %s^N\n", p2->follow);
 else
   fvtell_player(INFO_FT(CONST_STRLEN("Follow: "), p2->saved, p), "%s",
                 "Follow: ** no-one **\n");
 
 fvtell_player(INFO_T(p2->saved, p), "%s", DASH_LEN);
}

static void timed_remove_scripting(int timed_type,
                                   void *passed_player_tree_node)
{
 player_tree_node *current = passed_player_tree_node;
 player *p = current->player_ptr;

 assert(p);
 
 p->flag_tmp_scripting = FALSE; /* as proccess output is called a _lot_ */

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 if (timer_q_find_data(&scripting_timer_queue, p))
   fvtell_player(NORMAL_T(p),
                 " Time is now %s.\n"
                 " Scripting stopped.\n",
                 DISP_TIME_P_STD(now, p));
 else
 {
  assert_log(FALSE);
 }
}

static void user_emergency(player *p, const char *str)
{
 Timer_q_node *current_timer = NULL;
 struct timeval tv;
 
 if ((current_timer = timer_q_find_data(&scripting_timer_queue, p)))
 {
  if (TOGGLE_MATCH_OFF(str))
  {
   fvtell_player(NORMAL_T(p), " Scripting stopped at your request.\n");
   timer_q_quick_del_node(current_timer);
   return;
  }
  
  fvtell_player(NORMAL_T(p), "%s",
                " You are -- ^S^B_already_^s -- scripting.\n");
  return;
 }
 
 if (!*str)
   TELL_FORMAT(p, "<\"stop\"|reason>");
 
 if (TOGGLE_MATCH_OFF(str))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You haven't yet STARTED emergency scripting.\n"
                " Read '^S^Bhelp emergency^s' to learn how to use"
                " the ^S^Bemergency^s command properly.\n");
  return;
 }

 gettimeofday(&tv, NULL);
 TIMER_Q_TIMEVAL_ADD_MINS(&tv, 1, 0);
 
 if (!timer_q_add_node(&scripting_timer_queue,
                       p, &tv, TIMER_Q_FLAG_NODE_DEFAULT))
 {
  P_MEM_ERR(p);
  return;  
 }

 p->flag_tmp_scripting = TRUE;
 sprintf(p->script_file, "logs/emergency/%s_emergency.log",
         p->saved->lower_name);
 
 if (!configure.talker_read_only)
   remove(p->script_file);
 
 fvtell_player(NORMAL_T(p),
               " Emergency scripting started for 60 seconds.\n"
               " Remember, any previous scripts will be deleted\n"
               " Reason given: %s\n"
               " Time is now %s.\n", str, DISP_TIME_P_STD(now, p));

 channels_wall("staff", 3, p,
               " -=> %s%s %s started emergency scripting with the reason "
               "\'%s\'.",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name, (p->gender == GENDER_PLURAL) ? "have" : "has",
               str);
}

static void user_script(player *p, const char *str)
{
 Timer_q_node *current_timer = NULL;
 struct timeval tv;
 
 if ((current_timer = timer_q_find_data(&scripting_timer_queue, p)))
 {
  if (TOGGLE_MATCH_OFF(str))
  {
   timer_q_quick_del_node(current_timer);
   
   fvtell_player(NORMAL_T(p), " Scripting stopped at your request.\n");
   return;
  }

  fvtell_player(NORMAL_T(p), "%s",
                " You are -- ^S^B_already_^s -- scripting!\n");
  return;
 }

 if (!*str)
   TELL_FORMAT(p, "<\"stop\"|reason>");
 
 gettimeofday(&tv, NULL);
 TIMER_Q_TIMEVAL_ADD_HOURS(&tv, 1, 0);
 
 if (!timer_q_add_node(&scripting_timer_queue,
                       p, &tv, TIMER_Q_FLAG_NODE_DEFAULT))
 {
  P_MEM_ERR(p);
  return;  
 }

 p->flag_tmp_scripting = TRUE;
 sprintf(p->script_file, "logs/scripts/%s%s.log",
         p->saved->name, disp_time_file_name(now));
 
 if (!configure.talker_read_only)
   remove(p->script_file);

 fvtell_player(NORMAL_T(p),
               " -=> Scripting started at %s, for reason \'%s\'\n",
               DISP_TIME_P_STD(now, p), str);
 
 channels_wall("staff", 3, p,
               " -=> %s%s %s started continuous scripting with the reason "
               "\'%s\'.",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name,
               (p->gender == GENDER_PLURAL) ? "have" : "has", str);
}

static void user_time(player *p, const char *str)
{
 if (*str != '-')
 {
  char buf1[1024];
  char buf2[1024];
  char buf3[1024];
  
  ptell_mid(NORMAL_T(p), "Time", FALSE);

  fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p),
                " Local time: ^S^B%s^s\n"
                " GMT time: %s\n"
                " $Talker-Name has been alive: ^S^B%s^s\n"
                " Logons: %s\n"
                " Unique Logons: %s\n",
                DISP_TIME_P_STD(now, p),
                disp_time_std(now, 0, p->flag_use_24_clock, TRUE),
                word_time_long(buf1, sizeof(buf1),
                               difftime(now, talker_started_on),
                               WORD_TIME_ALL),
                word_number_base(buf2, sizeof(buf2), NULL, total_logons,
                                 TRUE, word_number_def),
                word_number_base(buf3, sizeof(buf3), NULL, total_uniq_logons,
                                 TRUE, word_number_def));
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 }
 else
   fvtell_player(NORMAL_T(p), " Your time is ^S^B%s^s.\n",
                 DISP_TIME_P_STD(now, p));
}

void user_uptime(player *p, const char *str)
{
 unsigned int days = (difftime(now, talker_started_on) / MK_DAYS(1));
 
 if (str && (*str == '-'))
 {
  char buf[1024];
  fvtell_player(NORMAL_T(p),
                " We have been up for %s.\n",
                word_time_long(buf, sizeof(buf),
                               difftime(now, talker_started_on),
                               WORD_TIME_ALL));
 }
 else
 {
  unsigned int hours = (difftime(now, talker_started_on) / MK_HOURS(1));
  unsigned int mins = (difftime(now, talker_started_on) / MK_MINUTES(1));
  
  if (str)
    fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p), "%s",
                  " $R-Time-Clock up ");
  else
    fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p), "%s",
                  " $Time-Clock up ");
  
  if (days)
    fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p), "%d day%s, ",
                  days, days == 1 ? "" : "s");
  
  if (hours)
    hours %= 24;
  if (mins)
    mins %= 60;
  
  fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p),
                "% 2d:%02d", hours, mins);
  
  fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p),
                ", connections %d/%d, %d on.\n",
                total_logons, total_uniq_logons, current_players);
 }
}

static void user_set_gender(player *p, const char *str)
{
 if (!beg_strcasecmp(str, "male"))
 {
  p->gender = GENDER_MALE;
  fvtell_player(NORMAL_T(p), "%s", " Gender set to Male.\n");
 }
 else if (!beg_strcasecmp(str, "female"))
 {
  p->gender = GENDER_FEMALE;
  fvtell_player(NORMAL_T(p), "%s", " Gender set to Female.\n");
 }
 else if (!beg_strcasecmp(str, "plural"))
 {
  p->gender = GENDER_PLURAL;
  fvtell_player(NORMAL_T(p), "%s", " Gender set to Plural.\n");
 }
 else if (!beg_strcasecmp(str, "none") || !beg_strcasecmp(str, "void") ||
          !beg_strcasecmp(str, "other"))
 {
  p->gender = GENDER_OTHER;
  fvtell_player(NORMAL_T(p), "%s", " Gender set to well, erm, something.\n");
 }
 else
   TELL_FORMAT(p, "[male|female|plural|none]");
}

static void user_toggle_email_public(player *p, const char *str)
{
 TOGGLE_COMMAND_OFF_ON(p, str, p->saved->flag_private_email, TRUE,
                       " Your email address is %sprivate, only the admin "
                       "will be able to see it.\n",
                       " Your email address is %spublic, so everyone can see "
                       "it.\n", TRUE);
}

static void user_toggle_ignore_prefix(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_no_prefix, TRUE,
                       " You are %signoring prefixes.\n",
                       " You are %sseeing prefixes.\n", TRUE);
}

static void user_toggle_ignore_emote_prefix(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_no_emote_prefix, TRUE,
                       " You are %signoring prefixes on emotes.\n",
                       " You are %sseeing prefixes on emotes.\n", TRUE);
}

static void user_set_gmt_offset(player *p, const char *str)
{
 int gmt_offset = 0;
 int is_neg = FALSE;
 int err = 0;
 char buf[256];
 
 if (!*str)
 {
  if (p->gmt_offset)
    fvtell_player(NORMAL_T(p),
                  " Time is currently set to ^S^B%s%s^s offset from GMT.\n",
                  (p->gmt_offset < 0) ? "-" : "",
                  word_time_long(buf, sizeof(buf),
                                 abs(p->gmt_offset), WORD_TIME_DEFAULT));
  else
    fvtell_player(NORMAL_T(p), "%s", " Time is currently set to GMT.\n");
  return;
 }

 if (*str == '-')
 {
  is_neg = TRUE;
  ++str;
 }
 
 gmt_offset = word_time_parse(str, WORD_TIME_PARSE_ERRORS |
                              WORD_TIME_PARSE_DEF_HOURS, &err);
 if (err || (gmt_offset >= MK_HOURS(24)))
 {
  fvtell_player(NORMAL_T(p), " The string -- ^S^B%s%s^s -- isn't a valid "
                "offset from gmt.\n", is_neg ? "-" : "", str);
  return;
 }

 if (!gmt_offset)
   fvtell_player(NORMAL_T(p), "%s", " Time set to GMT.\n");
 else
   fvtell_player(NORMAL_T(p),
                 " Time set to ^S^B%s%s^s offset from GMT.\n",
                 is_neg ? "-" : "",
                 word_time_long(buf, sizeof(buf),
                                gmt_offset, WORD_TIME_DEFAULT));

 if (is_neg)
   p->gmt_offset = -gmt_offset;
 else
   p->gmt_offset = gmt_offset;
}

static void user_showme(player *p, const char *str)
{
 twinkle_info info;

 setup_twinkle_info(&info);
 
 info.returns_limit = UINT_MAX;
 info.allow_fills = TRUE;
 fvtell_player(ALL_T(p->saved, p, &info, 0, now), "%s^N\n", str);
 pager(p, PAGER_DEFAULT);
}

static void user_set_nationality(player *p, const char *str)
{
 int pre_nat = p->nationality;
 
 if (!beg_strcasecmp(str, "british"))
   p->nationality = NATIONALITY_BRITISH;
 else if (!beg_strcasecmp(str, "american"))
   p->nationality = NATIONALITY_AMERICAN;
 else if (!beg_strcasecmp(str, "canadian"))
   p->nationality = NATIONALITY_CANADIAN;
 else if (!beg_strcasecmp(str, "australian"))
   p->nationality = NATIONALITY_AUSTRALIAN;
 else if (!beg_strcasecmp(str, "other"))
   p->nationality = NATIONALITY_OTHER;
 else if (!beg_strcasecmp(str, "void"))
   p->nationality = NATIONALITY_VOID;
 else
   TELL_FORMAT(p, "[american|australian|british|canadian|other|void]");
 
 fvtell_player(NORMAL_T(p), " You have %sset your nationality to be %s.\n",
               TOGGLE_CHANGED(pre_nat, p->nationality),
               get_nationality(p, TRUE));
}

void init_commands(void)
{
 timer_q_add_static_base(&scripting_timer_queue, timed_remove_scripting,
                         TIMER_Q_FLAG_BASE_DEFAULT);
}

void cmds_init_commands(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("age", user_set_age, CONST_CHARS, PERSONAL_INFO);

 CMDS_ADD("birthday", user_set_birthday, CONST_CHARS, PERSONAL_INFO);

 CMDS_ADD("emergency", user_emergency, CONST_CHARS, SYSTEM);
 CMDS_FLAG(no_expand);

 CMDS_ADD("examine", user_examine, CONST_CHARS, INFORMATION);
 CMDS_ADD("finger", user_finger, CONST_CHARS, INFORMATION);

 CMDS_ADD("gender", user_set_gender, CONST_CHARS, PERSONAL_INFO);

 CMDS_ADD("gmt_offset", user_set_gmt_offset, CONST_CHARS, SETTINGS);
 
 CMDS_ADD("multi_finger", user_multi_finger, CONST_CHARS, INFORMATION);
 
 CMDS_ADD("nationality", user_set_nationality, CONST_CHARS, INFORMATION);

 CMDS_ADD("noeprefix", user_toggle_ignore_emote_prefix, CONST_CHARS, SETTINGS);
 CMDS_ADD("noprefix", user_toggle_ignore_prefix, CONST_CHARS, SETTINGS);
 
 CMDS_ADD("pinformation", user_personal_info, CONST_CHARS, PERSONAL_INFO);

 CMDS_ADD("public_email", user_toggle_email_public, CONST_CHARS, SETTINGS);
 
 CMDS_ADD("recapitalise", user_recapitalise_name, CONST_CHARS, PERSONAL_INFO);
 CMDS_ADD("rescount", user_resident_count_show, NO_CHARS, INFORMATION);
 
 CMDS_ADD("save", user_player_save, NO_CHARS, SYSTEM);
 CMDS_PRIV(base);
 CMDS_ADD("screenlock", user_screenlock, NO_CHARS, SYSTEM);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(MISC);
 CMDS_ADD("script", user_script, CONST_CHARS, SYSTEM);
 CMDS_PRIV(command_script);
 CMDS_ADD("showme", user_showme, CONST_CHARS, MISC);
 CMDS_FLAG(no_beg_space); CMDS_FLAG(no_end_space);
 CMDS_ADD("system", user_su_system_com, CONST_CHARS, ADMIN);
 CMDS_FLAG(no_expand);
 
 CMDS_ADD("time", user_time, CONST_CHARS, INFORMATION);

 CMDS_ADD("uptime", user_uptime, CONST_CHARS, INFORMATION);
}
