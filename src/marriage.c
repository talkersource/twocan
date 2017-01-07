#define MARRIAGE_C
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


/* functions... */

/*takes the person you're married to*/
const char *get_spouse_id(player *p, int net_spouse)
{
 assert(p);
 if (!p)
   return ("spouse");
 
 switch (p->gender)
 {
  case GENDER_MALE:
    if (net_spouse)
      return ("net.husband");
    else
      return ("husband");
    
  case GENDER_FEMALE:
    if (net_spouse)
      return ("net.wife");
    else
      return ("wife");
    
  case GENDER_PLURAL:
    /* I don't have a clue what the plural
     * of spouse is - nor does the pocket oxford dictionary, so sod it*/
    if (net_spouse)
      return ("net.spouses");
    else
      return ("spouses");

  case GENDER_OTHER:
    if (net_spouse)
      return ("net.spouse");
    else
      return ("spouse");

  default:
    assert(FALSE);
    return ("** ERROR **");
 }
}

player *is_spouse_on_talker(player *p)
{
 player *p2 = NULL;
 
 if (p->flag_married && *p->married_to)
   p2 = player_find_on(p, p->married_to, PLAYER_FIND_DEFAULT);

 return (p2);
}

void marriage_update_spouce_name(player *p)
{
 player_tree_node *tmp = NULL;
 
 if (p->flag_married && *p->married_to &&
     (tmp = player_find_all(p, p->married_to, PLAYER_FIND_DEFAULT)))
   qstrcpy(p->married_to, tmp->name);
 else
   *p->married_to = 0;
}

static void user_marriage_propose(player *p, const char *str)
{
 player *p2 = NULL;
  
 if (!*str)
   TELL_FORMAT(p, "<person>/off/-");

 if (!strcasecmp("off", str))
 {
  if (*p->married_to)
    if (p->flag_married)
    {
     fvtell_player(NORMAL_T(p), "%s", " You're already married, sorry!\n");
     return;
    }
    else
    {
     fvtell_player(NORMAL_T(p), 
                   " You withdraw your proposal of marriage to %s.\n",
                   p->married_to);
     if ((p2 = player_find_on(p, p->married_to, PLAYER_FIND_DEFAULT)))
     {
      fvtell_player(SYSTEM_FT(HILIGHT, p2), 
                    "\n -=> %s withdraws %s proposal of marriage.\n\n",
                    p->saved->name,
                    gender_choose_str(p->gender, "his", "her",
                                      "their", "its"));
     }
     *p->married_to = 0;
     return;
    }
  else
    fvtell_player(NORMAL_T(p), "%s", " You have not yet propsed to anyone.\n");
  return;
 }
 else if (!strcasecmp("-", str))
 {
  if (*p->married_to)
  {
   if (p->flag_married)
     fvtell_player(NORMAL_T(p), " You're married to %s.\n", p->married_to);
   else
     fvtell_player(NORMAL_T(p), " You have proposed to %s.\n", p->married_to);
  }
  else
  {
   fvtell_player(NORMAL_T(p), "%s", " You have not yet proposed to anyone.\n");
  }
  return;
 }
    
 if (*p->married_to)
 {
  if (p->flag_married)
    fvtell_player(NORMAL_T(p), "%s", 
                  " You cannot propose to someone when you're already"
                  " married.\n");
  else
    fvtell_player(NORMAL_T(p), "%s", 
                  " You cannot propose to someone when you've already "
                  "proposed to another.\n Use: propose off\n");
  return;
 }

 if (!(p2 = player_find_on(p, str, PLAYER_FIND_VERBOSE | PLAYER_FIND_EXPAND)))
   return;

 if (p2 == p)
 {
  fvtell_player(NORMAL_T(p), "%s", 
                " You can't propose to yourself! Talk about egocentric!\n");
  return;
 }
 
 LIST_COMS_CHECK_FLAG_START(p2, p->saved);
 if (LIST_COMS_CHECK_FLAG_DO(tells))
 {
  fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking tells "
                "from you.\n", p2->saved->name);
  return;
 }
 LIST_COMS_CHECK_FLAG_END();
 
 if (*p2->married_to)
 {
  if (!strcasecmp(p->saved->name, p2->married_to))
  {
   fvtell_player(NORMAL_T(p), 
                 " You accept the proposal of marriage from %s.\n",
                 p2->saved->name);
   
   fvtell_player(SYSTEM_FT(HILIGHT, p2), 
                 "\n -=> %s accepts your proposal of marriage.\n\n",
                 p->saved->name);
   
   vwlog("propose", "%s accepts the proposal from %s.",
         p->saved->name, p2->saved->name);
   
   COPY_STR(p->married_to, p2->saved->name, PLAYER_S_NAME_SZ);
  }
  else
    fvtell_player(NORMAL_T(p), 
                  " The player '^S^B%s^s' has already proposed to "
                  "someone else.\n", p2->saved->name);    
  return;
 }

 fvtell_player(NORMAL_T(p), " You ask %s to marry you.\n", p2->saved->name);
 
 fvtell_player(SYSTEM_FT(HILIGHT, p2), 
               "\n -=> %s asks you to marry %s.\n\n", p->saved->name,
               gender_choose_str(p->gender, "him", "her", "them", "it"));
 
 COPY_STR(p->married_to, p2->saved->name, PLAYER_S_NAME_SZ);
 
 vwlog("propose", "%s proposes to %s.", p->saved->name, p2->saved->name);
}

static void user_marriage_decline(player *p, const char *str)
{
 player *p2 = NULL;
 
 if (!(p2 = player_find_load(p, str, PLAYER_FIND_VERBOSE |
                             PLAYER_FIND_EXPAND)))
   return;
 
 if (!strcasecmp(p2->married_to, p->saved->lower_name))
   if (!p2->flag_married)
   {
    if (p2->is_fully_on)
    {
     *p->married_to = 0;

     LIST_COMS_CHECK_FLAG_START(p2, p->saved);
     if (LIST_COMS_CHECK_FLAG_DO(tells))
     {
      fvtell_player(SYSTEM_T(p), " The player -- ^S^B%s^s -- is blocking "
                    "tells from you.\n", p2->saved->name);
      return;
     }
     LIST_COMS_CHECK_FLAG_END();

     fvtell_player(SYSTEM_FT(HILIGHT, p2), 
                   "\n -=> %s declines your proposal of marriage.\n\n", 
                   p->saved->name);
    }
    else
      CONST_COPY_STR_LEN(p2->married_to, "1decline");

    p2->saved->flag_tmp_player_needs_saving = TRUE;
    
    fvtell_player(NORMAL_T(p), 
                  " You decline the proposal of marriage from %s.\n", 
                  p2->saved->name);
   }
   else
     fvtell_player(NORMAL_T(p), "%s", 
                   " You cannot decline a proposal to someone you are "
                   "married to.\n");
 else
   if (strcasecmp(p2->married_to, p->saved->lower_name))
     fvtell_player(NORMAL_T(p),
                   " %s has not proposed to you, therefore you cannot "
                   "decline.\n", p2->saved->name);
}

static void user_su_marriage_marry(player *p, parameter_holder *params)
{
 player *victim1 = NULL;
 player *victim2 = NULL;
 int quiet = FALSE;

 if (p->flag_tmp_minister_channel_block)
 {
  fvtell_player(NORMAL_T(p), "%s", 
                " You cannot marry someone when you're off the minister "
                "list.\n");
  return;
 }
  
 switch (params->last_param)
 {
  default:
    TELL_FORMAT(p, "<person_1> <person_2> [quiet]");
  case 3:
    if (TOGGLE_MATCH_ON(GET_PARAMETER_STR(params, 3)) ||
        TOGGLE_MATCH_TOGGLE(GET_PARAMETER_STR(params, 3)))
      quiet = TRUE;
    else if (TOGGLE_MATCH_OFF(GET_PARAMETER_STR(params, 3)))
      quiet = FALSE;
    else
      TELL_FORMAT(p, "<person_1> <person_2> [quiet]");
  case 2:
    break;
 }

 if (!(victim1 = player_find_on(p, GET_PARAMETER_STR(params, 1),
                                PLAYER_FIND_VERBOSE)))
   return;

 if (!(victim2 = player_find_on(p, GET_PARAMETER_STR(params, 2),
                                PLAYER_FIND_VERBOSE)))
   return;

 if (!(strcmp(victim1->saved->lower_name, p->saved->lower_name)) ||
     !(strcmp(victim2->saved->lower_name, p->saved->lower_name)))
 {
  fvtell_player(NORMAL_T(p), "%s", " You can't marry yourself you plonker!\n");
  return;
 }

 if (victim1->flag_married || victim2->flag_married)
 {
  if (victim1->flag_married)
  {
   fvtell_player(NORMAL_T(p), "\n %s is already married, ",
                 victim1->saved->name);
   
   if (victim2->flag_married)
     fvtell_player(NORMAL_T(p), "as is %s, ", victim2->saved->name);   

   fvtell_player(NORMAL_T(p), "%s", "they must divorce first.\n\n");
  }
  else
    fvtell_player(NORMAL_T(p), 
                  "\n %s is already married, they must divorce first.\n\n",
                  victim2->saved->name);
   
  return;
 }

 if (strcasecmp(victim1->married_to, victim2->saved->name) ||
     (strcasecmp(victim2->married_to, victim1->saved->name)))
 {
  fvtell_player(NORMAL_T(p), "\n %s has proposed to ", victim1->saved->name);

  if (*victim1->married_to)
    fvtell_player(NORMAL_T(p), "%s. %s has proposed to ", victim1->married_to,
		     victim2->saved->name);
  else
    fvtell_player(NORMAL_T(p), "no one. %s has proposed to ",
		     victim2->saved->name);

  if (*victim2->married_to)
    fvtell_player(NORMAL_T(p),
                 "%s.\n Therefore they cannot be married.\n\n",
                 victim2->married_to);
  else
    fvtell_player(NORMAL_T(p), "%s",
                  "no one.\n Therefore they cannot be married.\n\n");
  return;
 }
  
 /* Marry the smeg heads */
 victim1->flag_married = TRUE;
 victim2->flag_married = TRUE;

 fvtell_player(NORMAL_FT(HILIGHT, p), "%s", 
               "\n Another couple are married, try not to feel bad.\n\n");
 
 fvtell_player(SYSTEM_FT(HILIGHT, victim1), 
               "\n -=> Congratulations!! You're now married to "
               "%s.\n\n", victim2->saved->name);
 
 fvtell_player(SYSTEM_FT(HILIGHT, victim2), 
               "\n -=> Congratulations!! You're now married to "
               "%s.\n\n", victim1->saved->name);
 
 if (!quiet)        
   sys_wall(HILIGHT,
            " %s screams -=> Congratulations to %s and %s who have just "
            "been married!! <=-$Bell\n",
            p->saved->name, victim1->saved->name, victim2->saved->name);
 
 vwlog("marriage", "%s is married to %s, by %s.", victim1->saved->name,
       victim2->saved->name, p->saved->name);
}

static int construct_lsm_name_list_do(player *scan, va_list ap)
{
 const char *str = va_arg(ap, const char *);
 int *count = va_arg(ap, int *);
 
 /* tell player stuff */
 player_tree_node *from = va_arg(ap, player_tree_node *);
 player *to = va_arg(ap, player *);
 twinkle_info *info = va_arg(ap, twinkle_info *);
 int flags = va_arg(ap, int);
 time_t my_now = va_arg(ap, time_t);

 if (!scan->saved->priv_minister)
   return (TRUE);
 
 if (scan->flag_tmp_minister_channel_block &&
     ((*str == '-') || (!to->saved->priv_minister && !PRIV_STAFF(to->saved))))
   return (TRUE);
 
 ++*count;

 fvtell_player(ALL_T(from, to, info, flags, my_now), " %-20s",
               scan->saved->name);
 
 if (PRIV_SYSTEM_ROOM(scan->location->owner) && !(*str == '-'))
 {
  char buffer[] = "                                                 ";
  sprintf(buffer, "[%s]", scan->location->id);
    
  fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", buffer);
 }
 else
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%15s", "");
 
 if (difftime(now, scan->last_command_timestamp))
   fvtell_player(ALL_T(from, to, info, flags, my_now),
                 " %03.0f:%02.0f idle ",
                 difftime(now, scan->last_command_timestamp) / 60,
                 fmod(difftime(now, scan->last_command_timestamp), 60.0));
 else
   fvtell_player(ALL_T(from, to, info, flags, my_now),
                 " %03d:00 idle ", 0);

 if (!(*str == '-') && scan->flag_tmp_minister_channel_block)
   fvtell_player(ALL_T(from, to, info, flags, my_now), "%s", " Off List");
 
 fvtell_player(ALL_T(from, to, info, flags, my_now), "%s", "\n");

 return (TRUE);
}

static void user_marriage_list_ministers(player *p, const char *str)
{
 tmp_output_list_storage tmp_save;
 output_node *tmp = NULL;
 int count = 0;

 save_tmp_output_list(p, &tmp_save);
 
 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_lsm_name_list_do, str, &count,
                INFO_FTP(OUTPUT_BUFFER_TMP, p)));

 tmp = output_list_grab(p);
 load_tmp_output_list(p, &tmp_save);
 
 if (!count)
 {
  assert(!tmp);
  fvtell_player(INFO_TP(p), "%s",
                " Sorry, there are no Ministers connected "
                "at the moment.\n");
  return;
 }

 if (count > 1)
 {
  char buffer[sizeof("There are $Number(%4d     )"
                     "-Tostr Ministers connected")];

  sprintf(buffer, "There are $Number(%4d)-Tostr Ministers connected",
          count);
  
  ptell_mid(INFO_TP(p), buffer, FALSE);
 }
 else
 {
  assert(count == 1);
  ptell_mid(INFO_TP(p), "There is one Minister connected", FALSE);
 }

 output_list_linkin(p, 3, &tmp, INT_MAX);
 fvtell_player(INFO_TP(p), "%s", DASH_LEN);

 pager(p, PAGER_DEFAULT);
}

static void user_su_toggle_marriage_lsm_hide(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_tmp_minister_channel_block, TRUE,
                       " You are %snot on the minister list.\n",
                       " You are %son the minister list.\n", TRUE);
}

static void user_su_marriage_divorce(player *p, const char *str)
{
 player *victim1 = NULL;
  
 if (!*str)
   TELL_FORMAT(p, "<player>");
 
 if (!(victim1 = player_find_on(p, str, PLAYER_FIND_SC_SU)))
   return;
 
 if (victim1->flag_married)
 {
  player *victim2 = NULL;
  
  if ((victim2 = player_find_load(p, victim1->married_to,
                                  PLAYER_FIND_SELF)))
  {
   *victim1->married_to = 0;
   *victim2->married_to = 0;
   victim1->flag_married = FALSE;
   victim2->flag_married = FALSE;

   victim2->saved->flag_tmp_player_needs_saving = TRUE;
   
   fvtell_player(NORMAL_FT(HILIGHT, p), 
                 " -=> You divorce %s and %s.\n",
                 victim1->saved->name,
                 victim2->saved->name);
   
   fvtell_player(SYSTEM_FT(HILIGHT, victim1), 
                 " -=> %s divorces you and %s.\n",
                 p->saved->name, victim2->saved->name);
   
   if (victim2->is_fully_on)
     fvtell_player(SYSTEM_FT(HILIGHT, victim2), 
                   " -=> %s divorces you and %s.\n",
                   p->saved->name,
                   victim1->saved->name);
   
   /* Log it */
   vwlog("divorce", "%s is divorced from %s, by %s.",
         victim1->saved->name, victim2->saved->name, 
         p->saved->name);
  }
  else
  {
   fvtell_player(NORMAL_T(p), "%s", 
                 " -=> Cannot locate the players partner.\n Fixing "
                 "marriage information.\n");
   *victim1->married_to = 0;
   victim1->flag_married = FALSE;
   
   fvtell_player(SYSTEM_T(victim1), "%s",
                 " -=> Marriage information has been fixed.\n You "
                 "are now divorced and free to re-propose or marry.\n");
   
   vwlog("divorce", " %s divorces %s, fixing marriage"
         " information.\n",
         p->saved->name, victim1->saved->name);
   return;
  }
 }
 else
   fvtell_player(NORMAL_T(p), "%s", " That person is not married, sorry.\n");
}

static void user_marriage_request_divorce(player *p)
{
 if (p->flag_married)
 {
  vwlog("divorce", "%s requests a divorce from %s.",
        p->saved->name, p->married_to);
  
  fvtell_player(NORMAL_T(p), "%s", 
                " You have requested a divorce. A minister will be in"
                " touch soon.\n");
 }
 else
   fvtell_player(NORMAL_T(p), "%s", 
                 " You can't request a divorce if you're not married!\n");
}

static void user_marriage_emote(player *p, const char *str)
{
 player *spouse = NULL;

 MARRIAGE_CHECK(p);
 
 if (!*str)
   TELL_FORMAT(p, "<msg>");
 
 if ((spouse = player_find_on(p, p->married_to, PLAYER_FIND_DEFAULT)))
 {
  const char *spouse_str = get_spouse_id(spouse, !p->flag_no_net_spouse);

  fvtell_player(TALK_FTP(HILIGHT, spouse), "%s[m] %s%s%.*s%s\n",
                USER_COLOUR_MARRIAGE, p->saved->name,
                isits1(str), OUT_LENGTH_COMMUNICATION, isits2(str), "^N");
  
  fvtell_player(TALK_TP(p), "%s You emote '%s%s%.*s%s' to your %s.%s\n",
                USER_COLOUR_MINE, p->saved->name, isits1(str),
                OUT_LENGTH_COMMUNICATION, isits2(str),
                USER_COLOUR_MINE, spouse_str, "^N");
  
  if (strcmp(p->married_to, spouse->saved->name))
    qstrcpy(p->married_to, spouse->saved->name);
 }
 else
   fvtell_player(NORMAL_T(p),
                 " Your spouse -- ^S^B%s^s -- is not currently logged on.\n",
                 p->married_to);
}

/* FIXME: alias */
static void user_marriage_sing(player *p, const char *str)
{
 player *spouse = NULL;

 MARRIAGE_CHECK(p);
 
 if (!*str)
   TELL_FORMAT(p, "<msg>");

 if ((spouse = player_find_on(p, p->married_to, PLAYER_FIND_DEFAULT)))
 {
  const char *spouse_str = get_spouse_id(spouse, !p->flag_no_net_spouse);

  fvtell_player(TALK_FTP(HILIGHT, spouse), 
                "%s[m] %s sings o/~ %.*s%s o/~%s\n",
                USER_COLOUR_MARRIAGE,
                p->saved->name, OUT_LENGTH_COMMUNICATION, str,
                USER_COLOUR_MARRIAGE, "^N");
  
  fvtell_player(TALK_TP(p), "%s You sing o/~ %.*s%s o/~ to your %s.%s\n",
                USER_COLOUR_MINE, OUT_LENGTH_COMMUNICATION, str,
                USER_COLOUR_MINE, spouse_str, "^N");
  
  if (strcmp(p->married_to, spouse->saved->name))
    qstrcpy(p->married_to, spouse->saved->name);
 }
 else
   fvtell_player(NORMAL_T(p),
                 " Your spouse -- ^S^B%s^s -- is not currently logged on.\n",
                 p->married_to);
}

static void user_marriage_say(player *p, const char *str, size_t length)
{
 player *spouse = NULL;

 MARRIAGE_CHECK(p);
 
 if (!*str)
   TELL_FORMAT(p, "<msg>");

 if ((spouse = player_find_on(p, p->married_to, PLAYER_FIND_DEFAULT)))
 {
  const char *spouse_str = get_spouse_id(spouse, !p->flag_no_net_spouse);

  fvtell_player(TALK_TP(spouse), "%s[m] %s %s '%.*s%s'.%s\n",
                USER_COLOUR_MARRIAGE,
                p->saved->name, say_ask_exclaim_group(p, str, length),
                OUT_LENGTH_COMMUNICATION, str, USER_COLOUR_MARRIAGE, "^N");
  
  fvtell_player(TALK_TP(p), "%s You %s '%.*s%s' to your %s.%s\n",
                USER_COLOUR_MINE, say_ask_exclaim_me(p, str, length),
                OUT_LENGTH_COMMUNICATION, str, USER_COLOUR_MINE,
                spouse_str, "^N");
  
  if (strcmp(p->married_to, spouse->saved->name))
    qstrcpy(p->married_to, spouse->saved->name);
 }
 else
   fvtell_player(NORMAL_T(p),
                 " Your spouse -- ^S^B%s^s -- is not currently logged on.\n",
                 p->married_to);
}

static void user_marriage_request_marriage(player *p)
{
 player *spouse = NULL;
 
 if (!*p->married_to)
   fvtell_player(NORMAL_T(p), "%s", 
                 " You must have proposed to someone to request a "
                 "marriage.\n");
 
 if (!(spouse = player_find_load(p, p->married_to, PLAYER_FIND_DEFAULT)))
 {
  fvtell_player(NORMAL_T(p), "%s", 
                " The person -- ^S^B%s^s -- no longer exists.\n"
                " Fixing marriage information.\n");
  *p->married_to = 0;
  p->saved->flag_tmp_player_needs_saving = TRUE;
  return;
 }

 /* FIXME: what does this mean ? */
 if (!strcasecmp(p->married_to, spouse->saved->lower_name) &&
     !p->flag_married)
 {
  vwlog("marriage", "%s requests a marriage to %s.\n",
        p->saved->name, spouse->saved->name);
  
  fvtell_player(NORMAL_T(p), "%s",
                " You log your request for a marriage. A minister will "
                "be in touch soon.\n");
 }
 else 
   if (!p->flag_married)
     fvtell_player(NORMAL_T(p), "%s", 
                   " The person you proposed to has yet to accept. Please"
                   " try again when they have done.\n");
   else
     fvtell_player(NORMAL_T(p), "%s", " But you're already married!\n");
}

static void user_toggle_marriage_hide(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_marriage_hide, TRUE,
                       " You are %shiding your marriages/proposals"
                       " in your x info.\n",
                       " You are %snot hiding your marriages/proposals"
                       " in your x info.\n", TRUE);
}

static void user_toggle_marriage_no_net_spouse(player *p, const char *str)
{
 if (p->flag_married)
 {
  TOGGLE_COMMAND_ON_OFF(p, str, p->flag_no_net_spouse, TRUE,
                        " You %ssee your spouse/fiance as a non-net "
                        "spouse/fiance.\n",
                        " You %ssee your spouse/fiance as a "
                        "net.spouse/net.fiance.\n", TRUE);
 }
 else
   fvtell_player(NORMAL_T(p), "%s",
                 " You must be married or engaged to use this command.\n");
}

void cmds_init_marriage(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("propose", user_marriage_propose, CONST_CHARS, SYSTEM);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(MISC);
 CMDS_ADD("decline", user_marriage_decline, CONST_CHARS, SYSTEM);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(MISC);

 CMDS_ADD("request_divorce", user_marriage_request_divorce, NO_CHARS, SYSTEM);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(MISC);
 CMDS_ADD("request_marriage", user_marriage_request_marriage, NO_CHARS,
          SYSTEM);
 CMDS_PRIV(base); CMDS_XTRA_SECTION(MISC);

 CMDS_ADD("divorce", user_su_marriage_divorce, CONST_CHARS, MINISTER);
 CMDS_PRIV(minister);
 CMDS_ADD("marry", user_su_marriage_marry, PARSE_PARAMS, MINISTER);
 CMDS_PRIV(minister);
 
 CMDS_ADD("mchannel", user_marriage_say, CHARS_SIZE_T, COMMUNICATION);
 CMDS_PRIV(command_marriage_channel);
 CMDS_ADD("memote", user_marriage_emote, CONST_CHARS, COMMUNICATION);
 CMDS_PRIV(command_marriage_channel);
 CMDS_ADD("msing", user_marriage_sing, CONST_CHARS, COMMUNICATION);
 CMDS_PRIV(command_marriage_channel);

 CMDS_ADD("no_net_spouse", user_toggle_marriage_no_net_spouse, CONST_CHARS,
          SETTINGS);
 CMDS_XTRA_SECTION(MISC);
 CMDS_ADD("hide_marriage", user_toggle_marriage_hide, CONST_CHARS,
          SETTINGS);
 CMDS_PRIV(base);

 CMDS_ADD("lsm_hide", user_su_toggle_marriage_lsm_hide, CONST_CHARS, MINISTER);
 CMDS_PRIV(minister);
 CMDS_ADD("list_ministers", user_marriage_list_ministers, CONST_CHARS, SYSTEM);
}
