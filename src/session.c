#define SESSION_C
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


static char session[SESSION_MSG_SZ];
static char sess_name[PLAYER_S_NAME_SZ];

static int session_allow_set = TRUE;

static unsigned int session_comments = 0;
static time_t session_timestamp;

static Timer_q_base session_timer_base;
static Timer_q_node session_timer_node;


static int internal_session_set(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 
 *scan->comment = 0;

 if (p == scan)
   return (TRUE);
 
 LIST_COMS_CHECK_FLAG_START(scan, p->saved);
 if (LIST_COMS_CHECK_FLAG_DO(sessions))
   return (TRUE);
 LIST_COMS_CHECK_FLAG_END();

 fvtell_player(TALK_TP(scan), " -=> %s%s set%s the session to be: %.57s^N\n",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               (p->gender == GENDER_PLURAL) ? "" : "s", session);
 
 return (TRUE);
}

static int internal_session_comment_set(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);

 if (p == scan)
   return (TRUE);

 LIST_COMS_CHECK_FLAG_START(scan, p->saved);
 if (LIST_COMS_CHECK_FLAG_DO(comments))
   return (TRUE);
 LIST_COMS_CHECK_FLAG_END();

 fvtell_player(TALK_TP(scan), " -=> %s%s set%s %s comment to be: %.57s^N\n",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               "$If( ==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))",
               (p->gender == GENDER_PLURAL) ? "" : "s",
               gender_choose_str(p->gender, "his", "her", "their", "its"), 
               p->comment);

 return (TRUE);
}

static void session_set(player *p, const char *str, int force)
{
 int seconds_left = 0;
 char buf[256];
 struct timeval real_tv;
 struct timeval *tv;

 assert(session_timer_base.num == !session_allow_set);
 
 if (!force)
 {
  if (!session_allow_set)
    if (timer_q_cntl_node(&session_timer_node,
                          TIMER_Q_CNTL_NODE_GET_TIMEVAL, &tv))
    {
     gettimeofday(&real_tv, NULL);
     seconds_left = timer_q_timeval_udiff_secs(tv, &real_tv);
    }
 }

 if (!*str)
 {
  ptell_mid(NORMAL_T(p), "Session message", FALSE);
  fvtell_player(NORMAL_T(p),
                " Current Message: %.57s^N\n"
                " Set by: %s\n"
                " Set at: %s\n"
                " Can be changed in: %s\n%s",
                session, sess_name,
                DISP_TIME_P_STD(session_timestamp, p),
                word_time_long(buf, sizeof(buf), seconds_left, WORD_TIME_ALL),
                DASH_LEN);
  return;
 }
 
 if (!force && (seconds_left > 0) &&
     strcasecmp(p->saved->lower_name, sess_name))
 {
  fvtell_player(SYSTEM_T(p), " Session can be changed in %s\n",
                word_time_long(buf, sizeof(buf), seconds_left, WORD_TIME_ALL));
  return;
 }

 COPY_STR(session, str, SESSION_MSG_SZ);
  
 fvtell_player(NORMAL_T(p),
               " You change the ^S^Bsession message^s to be '%.57s^N'\n",
               session);
 
 do_inorder_logged_on(internal_session_set, p);

 session_comments = 0;

 timer_q_del_data(&session_timer_base, &session_timer_node);
 if (!force)
 {
  gettimeofday(&real_tv, NULL);

  TIMER_Q_TIMEVAL_ADD_MINS(&real_tv, 15, 0);

  timer_q_add_static_node(&session_timer_node, &session_timer_base, 
                          &session_timer_node, &real_tv,
                          TIMER_Q_FLAG_NODE_SINGLE);
  session_allow_set = FALSE;
 }
 
 qstrcpy(sess_name, p->saved->name); 

 vwlog("session", "%s- %s^N", p->saved->name, session);
}

static void user_su_session_reset(player *p, const char *str)
{
 if (*str)
   session_set(p, str, TRUE);
 else
 {
  fvtell_player(NORMAL_T(p), "%s", " You reset the time on the session "
                "message, to zero.\n");
  if (session_allow_set)
    timer_q_del_node(&session_timer_base, &session_timer_node);
 }
}

static void user_session_set(player *p, const char *str)
{
 session_set(p, str, FALSE);
}

void session_player_leave(player *p)
{
 if (*p->comment)
   --session_comments;

 assert(!session_timer_base.num || (session_timer_base.num == 1));
 
 if (session_timer_base.num && !strcasecmp(p->saved->name, sess_name))
   timer_q_del_node(&session_timer_base, &session_timer_node);
}

static void user_session_comment_set(player *p, const char *str)
{
 if (!*str)
 {
  if (*p->comment)
  {
   fvtell_player(NORMAL_T(p), "%s",
                 " You blank your ^S^Bsession comment^s.\n");
   *p->comment = 0;
   --session_comments; 
   return;
  }
  TELL_FORMAT(p, "<msg>");
 }

 COPY_STR(p->comment, str, PLAYER_S_COMMENT_SZ);
 
 ++session_comments;
 
 fvtell_player(NORMAL_T(p),
               " You change your ^S^Bsession comment^s to be '%.57s^N'\n",
               p->comment);

 do_inorder_logged_on(internal_session_comment_set, p);
}

static int construct_session_name_list_do(player *scan, va_list ap)
{
 const char *temp_sessname = va_arg(ap, const char *);
 int only_comments = va_arg(ap, int);
 int comment_length = va_arg(ap, int);
 unsigned int *count = va_arg(ap, unsigned int *);
 /* params */
 player_tree_node *from = va_arg(ap, player_tree_node *);
 player *to = va_arg(ap, player *);
 twinkle_info *info = va_arg(ap, twinkle_info *);
 int flags = va_arg(ap, int);
 time_t my_now = va_arg(ap, time_t);

 if (!only_comments || *scan->comment)
 {
  if (to->flag_no_info_in_who)
    if (!strcmp(temp_sessname, scan->saved->lower_name))
      fvtell_player(ALL_T(from, to, info, flags, my_now),
                    "%-20s* ", scan->saved->name);
    else
      fvtell_player(ALL_T(from, to, info, flags, my_now),
                    "%-20s- ", scan->saved->name);
  else
    if (!strcmp(temp_sessname, scan->saved->lower_name))
      fvtell_player(ALL_T(from, to, info, flags, my_now),
                    "%-20s* ", scan->saved->name);
    else
      if (scan->flag_just_normal_hilight)
        fvtell_player(ALL_T(from, to, info, flags, my_now),
                      "%-20s- ", scan->saved->name);
      else
        if (scan->flag_no_specials_from_others)
          if (scan->flag_no_colour_from_others)
            fvtell_player(ALL_T(from, to, info, flags, my_now),
                          "%-20sS ", scan->saved->name);
          else
            fvtell_player(ALL_T(from, to, info, flags, my_now),
                          "%-20sC ", scan->saved->name);
        else
          if (scan->flag_no_colour_from_others)
            fvtell_player(ALL_T(from, to, info, flags, my_now),
                          "%-20sW ", scan->saved->name);
          else
            fvtell_player(ALL_T(from, to, info, flags, my_now),
                         "%-20sB ", scan->saved->name);
  
  fvtell_player(ALL_T(scan->saved, to, info, flags, my_now), "%.*s%s\n",
                comment_length, scan->comment, "^N");
  
  ++*count;
 }
 
 return (TRUE);
}

static void internal_session_show(player *p, int only_comments)
{
 int comment_length = 57;
 unsigned int count = 0;
 char lowered_name[PLAYER_S_NAME_SZ];
 
 if (!session_comments && only_comments)
 {
  fvtell_player(NORMAL_T(p), "%s", " There are ^S^Bno comments^s, at "
                "the moment.\n");
  return;
 }
 
 qstrcpy(lowered_name, sess_name);
 lower_case(lowered_name);

 ptell_mid(NORMAL_T(p), session, TRUE);

 if (p->see_raw_twinkles)
   comment_length = INT_MAX;

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_session_name_list_do,
                lowered_name, only_comments, comment_length, &count,
                NORMAL_T(p)));
 
 fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p), "%s", DASH_LEN);
 
 assert(count);
 if (only_comments)
 {
  assert(count == session_comments);
  session_comments = count;
 }
 
 pager(p, PAGER_DEFAULT);
}

void user_session_show(player *p)
{
 internal_session_show(p, FALSE);
}

static void user_session_comments_show(player *p)
{
 internal_session_show(p, TRUE);
} 

static void user_toggle_session_in_who(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_session_in_who, TRUE,
                       " You will "
                       "%ssee sessions with the ^S^Bwho^s command.\n",
                       " You will not "
                       "%ssee sessions with the ^S^Bwho^s command.\n", TRUE);
}

static void session_timer_func(int type, void *data)
{
 IGNORE_PARAMETER(data);
 
 if (type == TIMER_Q_TYPE_CALL_DEL)
   return;
 
 session_allow_set = TRUE;
}

void init_session(void)
{
 session_timestamp = now;

 timer_q_add_static_base(&session_timer_base, session_timer_func,
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY);
 
}

void cmds_init_session(void)
{
 CMDS_BEGIN_DECLS();
 
 sprintf(session, "Welcome to %.*s",
         SESSION_MSG_SZ - (int)sizeof("Welcome to "), configure.name_long);
 sprintf(sess_name, "%.*s", PLAYER_S_NAME_SZ, configure.player_name_admin);

 CMDS_ADD("comment", user_session_comment_set, CONST_CHARS, PERSONAL_INFO);
 CMDS_FLAG(no_beg_space); CMDS_XTRA_SECTION(SETTINGS);
 CMDS_ADD("comments", user_session_comments_show, NO_CHARS, INFORMATION);

 CMDS_ADD("session_show", user_session_show, NO_CHARS, SETTINGS);
 CMDS_ADD("session_in_who", user_toggle_session_in_who, CONST_CHARS, SETTINGS);

 CMDS_ADD("session", user_session_set, CONST_CHARS, INFORMATION);
 CMDS_PRIV(command_session);
 CMDS_ADD("session_reset", user_su_session_reset, CONST_CHARS, SU);
 CMDS_PRIV(normal_su);
}
