#define CHECK_C
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

static void check_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", 
               " Re-Entering check mode. Use ^Bhelp check^N for help\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n");
}

static int check_command(player *p, const char *str, size_t length)
{
 ICTRACE("check_command");
 
 if (MODE_IN_MODE(p, CHECK))
   MODE_HELPER_COMMAND();
 else
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T((&tmp_cmd), check_command);
    CMDS_FUNC_TYPE_NO_CHARS((&tmp_rejoin), check_rejoin_func);
    
    if (mode_add(p, "Check Mode-> ", MODE_ID_CHECK, 0,
                 &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), "%s", 
                    " Entering check mode. Use ^Bhelp check^N for help\n"
                    " To run ^Bnormal commands^N type /<command>\n"
                    " Use '^Bend^N' to ^Bleave^N.\n");
    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter check mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }
 
 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_CHECK)]));
}

static void user_check_view_commands(player *p)
{
 user_cmds_show_section(p, "check");
}

static void check_exit_command(player *p)
{
 if (!MODE_IN_MODE(p, CHECK))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You just tried to exit check mode while not being in it?\n");
  return;
 }

 fvtell_player(NORMAL_T(p), "%s", " Leaving check mode.\n");

 mode_del(p);
}

static void user_check_wrapping(player *p, const char *str)
{
 if (p->term_width)
 {
  if (*str == '-')
  {
   fvtell_player(NORMAL_T(p), 
                 " Line wrap is on.\n Word wrap is %s.\n",
                 (p->word_wrap ? "on" : "off"));
   return;
  }
  
  fvtell_player(NORMAL_T(p), " Line wrap on, with terminal width set to %d "
                "characters.\n", p->term_width + 1);
  
  if (p->word_wrap)
    fvtell_player(NORMAL_T(p), " Word wrap is on, with biggest word size "
                  "set to %d characters.\n", p->word_wrap);
  else
    fvtell_player(NORMAL_T(p), "%s", " Word wrap is off.\n");
 }
 else
   fvtell_player(NORMAL_T(p), "%s", " Line wrap and word wrap turned off.\n");
}

static int construct_ip_name_list_do(player *scan, va_list ap)
{
 player_tree_node *from = va_arg(ap, player_tree_node *);
 player *to = va_arg(ap, player *);
 twinkle_info *info = va_arg(ap, twinkle_info *);
 int flags = va_arg(ap, int);
 time_t my_now = va_arg(ap, time_t);

 fvtell_player(ALL_T(from, to, info, flags, my_now),
               "%s%s %s logged in from %d.%d.%d.%d:^S^B%s^s\n",
               gender_choose_str(scan->gender, "", "", "The ", "The "),
               scan->saved->name,
               (scan->gender == GENDER_PLURAL) ? "are" : "is",
               (int)scan->ip_address[0], (int)scan->ip_address[1],
               (int)scan->ip_address[2], (int)scan->ip_address[3],
               scan->dns_address); 
 
 return (TRUE);
}

static void user_su_check_ip(player *p, const char *str)
{
 if (isalpha((unsigned char) *str))
 {
  player *p2 = NULL;
  
  if ((p2 = player_find_on(p, str, PLAYER_FIND_SC_EXTERN_ALL)))
    fvtell_player(NORMAL_T(p),
                  "%-*s %s logged in from %d.%d.%d.%d:^S^B%s^s.\n",
		  PLAYER_S_NAME_SZ, p2->saved->name,
                  (p2->gender == GENDER_PLURAL) ? "are" : "is",
                  (int)p->ip_address[0], (int)p->ip_address[1],
                  (int)p->ip_address[2], (int)p->ip_address[3],
                  p2->dns_address);
  return;
 }
 
 if (current_players == 1)
 {
  fvtell_player(SYSTEM_T(p), 
	       "There is only you on the program at the moment.\n");
 }
 else
 {
  ptell_mid(NORMAL_T(p),
            "There are $Current_players-Tostr people on the program", FALSE);  

  DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
                (construct_ip_name_list_do, NORMAL_WFT(1, p)));
  
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
 }
}

static void user_su_check_info(player *p, const char *str)
{
 player *p2 = NULL;
 Timer_q_node *current_timer = NULL;
 char buffer[sizeof("Banished Player %s") + PLAYER_S_NAME_SZ];
 int done = FALSE;
 
 if (!*str)
   TELL_FORMAT(p, "<player>"); 

 if (!(p2 = player_find_load(p, str, PLAYER_FIND_SC_SU_ALL |
                              PLAYER_FIND_BANISHED)))
   return;

 if (p2->saved->priv_banished)
 {
  sprintf(buffer, "Banished player %s", p2->saved->name);
  ptell_mid(NORMAL_T(p), buffer, FALSE);
 }
 else
 {
  sprintf(buffer, "Player %s", p2->saved->name);
  ptell_mid(NORMAL_T(p), buffer, FALSE);
 }

 fvtell_player(NORMAL_T(p), "In Core Flags: ");
 done = FALSE;
 P_SHOW_FLAG(p, p2->saved, 16, private_email, "Private email");
 P_SHOW_FLAG(p, p2->saved, 16, no_anonymous, "No anonymous mail");
 P_SHOW_FLAG(p, p2->saved, 16, agreed_disclaimer, "Agreed disclaimer");
 P_SHOW_FLAG(p, p2->saved, 16, hide_logon_time, "Hide logon time");
 P_SHOW_FLAG(p, p2->saved, 16, kill_logon_time, "Kill logon time");
 fvtell_player(NORMAL_T(p), "%s\n", done ? "." : ""); 

 if (!p2->passwd[0])
   ptell_mid(NORMAL_T(p), "** NO PASSWORD **", FALSE);
 
 fvtell_player(NORMAL_T(p), "Player Flags: ");
 done = FALSE;
 P_SHOW_FLAG(p, p2, 15, use_birthday_as_age, "Age is got from Birthday");
 P_SHOW_FLAG(p, p2, 15, use_24_clock, "Use 24 hour clock");
 P_SHOW_FLAG(p, p2, 15, follow_block, "Follow block");
 P_SHOW_FLAG(p, p2, 15, room_enter_brief, "Room enter Brief");
 /* FIXME: no where near all flags are here... */
 fvtell_player(NORMAL_T(p), "%s\n", done ? "." : "");
 
 fvtell_player(NORMAL_WFT(5, p),
               "Max: rooms %d, exits %d, autos %d, list %d, mails %d, "
               "aliases %d, nicknames %d.\n",
               p2->max_rooms, p2->max_exits, p2->max_autos,
               p2->max_list_entries, p2->max_mails, p2->max_aliases,
               p2->max_nicknames);
 
 fvtell_player(NORMAL_T(p), "Term: width %d, wrap %d.\n",
              p2->term_width, p2->word_wrap);

 if ((current_timer = timer_q_find_data(&scripting_timer_queue, p2)))
 {
  char buf[256];
  const struct timeval *tv = NULL;

  if (timer_q_cntl_node(current_timer, TIMER_Q_CNTL_NODE_GET_TIMEVAL, &tv))
    fvtell_player(NORMAL_T(p), " Scripting on for another %s.\n",
                  word_time_long(buf, sizeof(buf),
                                 timer_q_timeval_udiff_secs(&now_timeval, tv),
                                 WORD_TIME_ALL));
 }

 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void user_check_email(player *p)
{
 if (!p->saved->priv_base)
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You are non resident and so cannot set an email "
                "address.\n"
                " Please ask a super user to make you resident.\n");
  return;
 }
 
 if (p->email[0])
   fvtell_player(NORMAL_T(p), " Your email address is set to:\n%s\n",
                 p->email);
 else
 {
  fvtell_player(NORMAL_T(p), "%s", " You have not set an email address.\n");
  return;
 }
 
 if (p->saved->flag_private_email)
   fvtell_player(NORMAL_T(p), "%s", " Your email is private.\n");
 else
   fvtell_player(NORMAL_T(p), "%s",
                 " Your email is public for all to read.\n");
}

void cmds_init_check(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("check", check_command, RET_CHARS_SIZE_T, INFORMATION);
 CMDS_XTRA_SECTION(SETTINGS); CMDS_XTRA_SECTION(SYSTEM);

#define CMDS_SECTION_SUB CMDS_SECTION_CHECK

 CMDS_ADD_SUB("autos", user_room_check_autos, NO_CHARS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("commands", user_check_view_commands, NO_CHARS);
 CMDS_ADD_SUB("email", user_check_email, NO_CHARS);
 CMDS_ADD_SUB("end", check_exit_command, NO_CHARS);
 CMDS_PRIV(mode_check);
 
 CMDS_ADD_SUB("entry", user_list_other_player_view, PARSE_PARAMS);
 CMDS_PRIV(base);
 CMDS_ADD_SUB("follow", user_follow_check, NO_CHARS);
 CMDS_ADD_SUB("groups", user_news_list_newsgroups, CONST_CHARS);
 CMDS_ADD_SUB("information", user_su_check_info, CONST_CHARS);
 CMDS_PRIV(admin);
 CMDS_ADD_SUB("ip", user_su_check_ip, CONST_CHARS);
 CMDS_PRIV(command_trace);
 CMDS_ADD_SUB("mail", user_mail_check, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("mail_all", user_mail_check_all, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("news", user_news_list_articles, CONST_CHARS);
 CMDS_ADD_SUB("news_all", user_news_list_articles_all, CONST_CHARS);
 CMDS_ADD_SUB("room", user_room_check_flags, NO_CHARS);
 CMDS_ADD_SUB("rooms", user_room_check_all, CONST_CHARS);
 CMDS_PRIV(command_room);
 CMDS_ADD_SUB("sent", user_mail_sent_check, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("sent_all", user_mail_sent_check_all, PARSE_PARAMS);
 CMDS_PRIV(command_mail);
 CMDS_ADD_SUB("terminal", user_check_terminal, CONST_CHARS);
 CMDS_ADD_SUB("wrapping", user_check_wrapping, CONST_CHARS);
}
