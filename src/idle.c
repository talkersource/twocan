#define IDLE_C
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

static Timer_q_base idle_timer_queue;

void idle_check_increment_total(player *p)
{
 if (p->typed_commands) /* make sure they've DONE something */
 {
  if (difftime(now, p->last_command_timestamp) >= IDLE_TIME_PERMITTED)
    p->idle_logon += difftime(now, p->last_command_timestamp);
 }
 else if (p->is_fully_on)
      /* check if they've just logged on and idled */
   p->idle_logon += difftime(now, p->logon_timestamp);
}

static void timed_idle_func(int timed_type, void *passed_player)
{
 player *p = passed_player;
 int secs = 0;
 int after = 0;
 struct timeval tv;
 
 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
 {
  p->idle_in_queue = FALSE;
  return;
 }

 secs = floor(difftime(now, p->last_command_timestamp));

 if (difftime(now, p->prompt_last_output) > 64)
   if (MODE_CURRENT(p).prompt)
     prompt_update(p, MODE_CURRENT(p).prompt);
 
 if (p->flag_tmp_idle_had_warn)
 {
  after = (p->idle_msg[0] ? IDLE_TIME_IDLE_KICK : IDLE_TIME_NOIDLE_KICK);
  
  if (secs >= after)
  {
   p->idle_in_queue = FALSE;

   fvtell_player(SYSTEM_FT(HILIGHT, p), "%s",
                 " -=> You've been idle for far too long; don't say "
                 "we didnt warn you.$Bell\n");
   
   channels_wall("staff", 3, p,
                 " -=> %s has idled into oblivion with%s an idlemsg.", 
                 p->saved->name, p->idle_msg[0] ? "" : "out");
   vwlog("idleouts", " -=> %s has idled into oblivion "
         "with%s an idlemsg.\n", 
         p->saved->name, p->idle_msg[0] ? "" : "out");
   
   stats_log_event(p, STATS_RESI_OFF, STATS_RESI_OFF_IDLEOUT);
   
   user_logoff(p, NULL);
  }
  else
  {
   int nxt_time = (after - secs) + 1;
   if (nxt_time > IDLE_TIME_STEP_MAX)
     nxt_time = IDLE_TIME_STEP_MAX;
  
   gettimeofday(&tv, NULL);
   TIMER_Q_TIMEVAL_ADD_SECS(&tv, nxt_time, 0);
   timer_q_add_static_node(&p->idle_timer.s, &idle_timer_queue, p, &tv,
                           TIMER_Q_FLAG_NODE_DOUBLE);
  }
 }
 else
 {
  after = (p->idle_msg[0] ? IDLE_TIME_IDLE_WARN : IDLE_TIME_NOIDLE_WARN);
  
  if (secs >= after)
  {
   p->flag_tmp_idle_had_warn = TRUE;
   fvtell_player(SYSTEM_FT(HILIGHT | OVERRIDE_RAW_OUTPUT_VARIABLES, p),
                 " -=> Please stop idling, otherwise you'll be removed "
                 "from $Talker-Name in 10 minutes.$Bell\n");
   
   gettimeofday(&tv, NULL);
   TIMER_Q_TIMEVAL_ADD_SECS(&tv, 10, 0);
   timer_q_add_static_node(&p->idle_timer.s, &idle_timer_queue, p, &tv,
                           TIMER_Q_FLAG_NODE_DOUBLE);
  }
  else
  {
   int nxt_time = (after - secs) + 1;
   if (nxt_time > IDLE_TIME_STEP_MAX)
     nxt_time = IDLE_TIME_STEP_MAX;
  
   gettimeofday(&tv, NULL);
   TIMER_Q_TIMEVAL_ADD_SECS(&tv, nxt_time, 0);
   timer_q_add_static_node(&p->idle_timer.s, &idle_timer_queue, p, &tv,
                           TIMER_Q_FLAG_NODE_DOUBLE);
  }
 }
}

void idle_timer_start(player *p)
{
 struct timeval tv;
 
 if (/* p->saved->priv_no_timeout || */ p->idle_in_queue)
 {
  assert_log(!p->idle_in_queue);
  return; /* removing someone's no timeout isn't a problem */
 }
 
 gettimeofday(&tv, NULL);
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, IDLE_TIME_NOIDLE_WARN, 0);
 timer_q_add_static_node(&p->idle_timer.s, &idle_timer_queue, p, &tv,
                         TIMER_Q_FLAG_NODE_DOUBLE);

 p->idle_in_queue = TRUE;
}

void idle_timer_stop(player *p)
{
 if (p->idle_in_queue)
   timer_q_del_node(&idle_timer_queue, &p->idle_timer.s);
 assert_log(!p->idle_in_queue);
}

/* user commands */
static int internal_construct_idle_name_list_do(player *scan, 
                                                player *p,
                                                const char *str)
{
 tmp_output_list_storage tmp_save;
 output_node *tmp = NULL;
 int string_length = (p->term_width ? p->term_width : 79) - 9;
 
 save_tmp_output_list(p, &tmp_save);

 fvtell_player(INFO_FT(OUTPUT_BUFFER_TMP, scan->saved, p), "%s",
               "$If(==(1(N) 2($R-Set-Ign_prefix))"
               " t($F-Name_full) f($F-Name))");

 if (*str == '-')
   fvtell_player(INFO_FT(OUTPUT_BUFFER_TMP, scan->saved, p), "%-*s",
                 string_length, "");
 else
   fvtell_player(INFO_FT((RAW_OUTPUT | OUTPUT_BUFFER_TMP), scan->saved, p),
                 "%s%-*s", isits1(scan->idle_msg),
                 string_length - 1, isits2(scan->idle_msg));

 tmp = output_list_grab(p);

 load_tmp_output_list(p, &tmp_save);
 
 output_list_linkin(p, 3, &tmp, string_length);
 
 if (difftime(now, scan->last_command_timestamp))
   fvtell_player(INFO_T(scan->saved, p),
                 "[%.0f:%02.0f]\n",
                 floor(difftime(now, scan->last_command_timestamp) / 60.0),
                 fmod(difftime(now, scan->last_command_timestamp), 60.0));
 else
   fvtell_player(INFO_T(scan->saved, p),
                 "[%d:%.2d]\n", 0, 0);
 
 return (TRUE);
}

static int construct_idle_name_list_do(player *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str = va_arg(ap, const char *);

 return (internal_construct_idle_name_list_do(scan, p, str));
}

static int construct_idle_name_list_do_multi(multi_node *scan, va_list ap)
{
 player *p = va_arg(ap, player *);
 const char *str = va_arg(ap, const char *);
 
 assert(P_IS_ON(scan->parent));
 
 if ((scan->parent != p->saved) ||
     multi_check_node_for_flag(scan, MULTI_TO_SELF))
   return (internal_construct_idle_name_list_do(scan->parent->player_ptr,
                                                p, str));

 return (TRUE);
}

static int count_not_idle(player *scan, va_list ap)
{
 int *not_idle = va_arg(ap, int *);
 
 if (difftime(now, scan->last_command_timestamp) < 300)
   ++*not_idle;

 return (TRUE);
}

static void user_idle_check(player *p, const char *str)
{
 char middle[sizeof("There are %d people here, %d %s awake") +
            (2 * BUF_NUM_TYPE_SZ(int)) + sizeof("are")];
 int not_idle = 0;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (get_parameter_parse(&params, &str, 3))
   TELL_FORMAT(p, "[player(s)] [-]");

 if (params.last_param && strcmp(GET_PARAMETER_STR(&params, 1), "-"))
 {
  multi_return *values = NULL;

  values = multi_add(p->saved, GET_PARAMETER_STR(&params, 1),
                     MULTI_VERBOSE |
                     MULTI_NO_DO_MATCH |
                     MULTI_NO_COMS_CHECKS |
                     MULTI_LIVE_ON_SMALL |
                     MULTI_MUST_CREATE |
                     MULTI_DESTROY_CLEANUP, PLAYER_FIND_SC_INTERN_ALL);
  
  if (values->multi_number)
  {
   if ((values->players_added == 1) &&
       !multi_check_for_flag(values->multi_number, MULTI_TO_SELF))
   {
    multi_cleanup(values->multi_number, MULTI_DEFAULT);
    return;
   }
   
   get_parameter_shift(&params, 1);

   ptell_mid(INFO_TP(p), "Idle list", FALSE);
   
   do_inorder_multi(construct_idle_name_list_do_multi,
                    values->multi_number, MULTI_DEFAULT, p,
                    params.last_param ? GET_PARAMETER_STR(&params, 1) : "");
   
   multi_cleanup(values->multi_number, MULTI_DEFAULT);

   fvtell_player(INFO_TP(p), "%s", DASH_LEN);
   
   pager(p, PAGER_DEFAULT);
  }
  else
    TELL_FORMAT(p, "[player(s)]");

  pager(p, PAGER_DEFAULT);
  return;
 }
 
 do_inorder_logged_on(count_not_idle, &not_idle);
  
 if (current_players == 1)
   CONST_COPY_STR_LEN(middle, "There is only you on, at the moment");
 else
   sprintf(middle, "There are %d people here, %d %s awake",
           current_players, not_idle, (not_idle == 1) ? "is" : "are");
 
 ptell_mid(INFO_TP(p), middle, FALSE);

 DO_ORDER_TEST(logged_on, p->flag_list_show_inorder, in, cron,
               (construct_idle_name_list_do, p, str));
 
 fvtell_player(INFO_TP(p), "%s", DASH_LEN);

 pager(p, PAGER_DEFAULT);
}

static void user_idle_msg(player *p, const char *str)
{
 if (p->idle_msg[0])
 {
  fvtell_player(NORMAL_T(p), "%s", " Your ^S^Bidle message^s has been "
                "automatically reset.\n");
  return;
 }
 
 COPY_STR_BUILD_FUNC_ISITS_RAW(p->idle_msg, str, "idle message",
                               PLAYER_S_IDLE_MSG_SZ);
 fvtell_player(NORMAL_T(p), "%s", " -=> This will be automatically reset, "
               "when you run a command.\n");
}

void init_idle(void)
{
 timer_q_add_static_base(&idle_timer_queue, timed_idle_func,
                         TIMER_Q_FLAG_BASE_DEFAULT |
                         TIMER_Q_FLAG_BASE_INSERT_FROM_END);
}

void cmds_init_idle(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("idle", user_idle_check, CONST_CHARS, INFORMATION);
 CMDS_ADD("idlemsg", user_idle_msg, CONST_CHARS, PERSONAL_INFO);
}
