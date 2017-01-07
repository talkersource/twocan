#define TIMER_PLAYER_C
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

Timer_q_base timer_queue_player_no_logon;
Timer_q_base timer_queue_player_no_move;
Timer_q_base timer_queue_player_no_shout;
Timer_q_base timer_queue_player_jail;

static int internal_timer_player_setup(player *p, const char *str,
                                       Timer_q_base *timer_base,
                                       const char *timer_type,
                                       player_tree_node **ret)
{
 unsigned long seconds = 0;
 Timer_q_node *current_timer = NULL;
 parameter_holder params;
 player_tree_node *ret_dummy = NULL;
 char buf[256];
 struct timeval tv;
 
 get_parameter_init(&params);
 
 get_parameter_parse(&params, &str, 3);

 if (!ret)
   ret = &ret_dummy;
 
 *ret = NULL;
 
 switch (params.last_param)
 {
  default:
    TELL_FORMAT_NO_RETURN(p, "<player> [period]");
    return (0);
    
  case 1:
    seconds = MK_MINUTES(1);
    break;

  case 2:
  {
   int wt_err;
   
   seconds = word_time_parse(GET_PARAMETER_STR(&params, 2),
                             WORD_TIME_PARSE_ERRORS, &wt_err);
   
   if (wt_err)
   {
    TELL_FORMAT_NO_RETURN(p, "<player> [period]");
    return (0);
   }

   if ((seconds > MK_MINUTES(10)) && !p->saved->priv_admin)
   {
    fvtell_player(NORMAL_T(p), "%s",
                  " That amount of time is too harsh, set to 10 mins.\n");
    seconds = MK_MINUTES(10);
   }
  }
  break;
 }

 if (!(*ret = player_find_all(p, GET_PARAMETER_STR(&params, 1),
                              PLAYER_FIND_SC_SU_ALL)))
   return (0);

 if (!seconds)
 { /* anyone is allowed to stop it from anyone */
  if (!(current_timer = timer_q_find_data(timer_base, *ret)))
  {
   fvtell_player(SYSTEM_T(p),
                 " The player -- ^S^B%s^s -- isn't under a %s timer.\n",
                 (*ret)->name, timer_type);
   return (0);
  }
  
  if (P_IS_ON(*ret))
  { /* use says, instead ? */
   vtell_room_wall((*ret)->player_ptr->location, (*ret)->player_ptr,
                   " -=> %s%s %s now ^S^B_not_^s under a ^S^B%s^s timer.\n",
                   gender_choose_str((*ret)->player_ptr->gender, "", "",
                                     "The ", "The "),
                   (*ret)->player_ptr->saved->name,
                   ((*ret)->player_ptr->gender == GENDER_PLURAL) ?
                   "are" : "is", timer_type);
   
   fvtell_player(SYSTEM_T((*ret)->player_ptr),
                 " -=> Someone removes a ^S^B%s^s timer from you.\n",
                 timer_type);
  }
  
  timer_q_quick_del_node(current_timer);
  
  vwlog("timer_player", " -=> %s removes a %s timer from %s.\n", 
        p->saved->name, timer_type, (*ret)->name);
  channels_wall("staff", 3, NULL,
                " -=> %s removes a ^S^B%s^s timer from %s.",
                p->saved->name, timer_type, (*ret)->name);
  return (0);
 }
 
 if (priv_test_check(p->saved, *ret) < 1)
 {
  fvtell_player(SYSTEM_T(p),
                " The player -- ^S^B%s^s -- has enough "
                "privileges that you cannot put a -- ^S^B%s^s -- timer "
                "on them.\n", (*ret)->name, timer_type);
  
  if (P_IS_ON(*ret))
    fvtell_player(SYSTEM_T((*ret)->player_ptr),
                  " -=> %s%s tried to put a ^S^B%s^s timer on you.\n",
                  gender_choose_str(p->gender, "", "", "The ", "The "),
                  p->saved->name, timer_type);
  
  vwlog("priv_test", "%s failed to put a %s timer on %s",
        p->saved->name, timer_type, (*ret)->name);
  *ret = NULL;
  return (0);
 }
    
 timer_q_del_data(timer_base, *ret); /* in case we are doing it again */

 gettimeofday(&tv, NULL);

 TIMER_Q_TIMEVAL_ADD_SECS(&tv, seconds, 0);
 
 if (!timer_q_add_node(timer_base, *ret, &tv,
                       TIMER_Q_FLAG_NODE_DEFAULT))
 {
  P_MEM_ERR(p);
  *ret = NULL;
  return (0);
 }

 if (P_IS_ON(*ret))
 { /* use says, instead ? */
  vtell_room_wall((*ret)->player_ptr->location, (*ret)->player_ptr,
                  " -=> %s%s %s now under a ^S^B%s^s timer for %s.\n",
                  gender_choose_str((*ret)->player_ptr->gender, "", "",
                                    "The ", "The "),
                  (*ret)->player_ptr->saved->name,
                  ((*ret)->player_ptr->gender == GENDER_PLURAL) ? "are" : "is",
                  timer_type, word_time_long(buf, sizeof(buf),
                                             seconds, WORD_TIME_DEFAULT));
  
  fvtell_player(SYSTEM_T((*ret)->player_ptr),
                " You are now under a ^S^B%s^s timer for %s.\n", timer_type,
                word_time_long(buf, sizeof(buf),
                               seconds, WORD_TIME_DEFAULT));
 }
  
 channels_wall("staff", 3, NULL,
               " -=> %s put %s under a ^S^B%s^s timer for %s.",
               p->saved->name, (*ret)->name, timer_type,
               word_time_long(buf, sizeof(buf),
                              seconds, WORD_TIME_DEFAULT));
 
 vwlog("timer_player", "%s put %s under a %s timer for %s.\n",
       p->saved->name, (*ret)->name, timer_type,
       word_time_long(buf, sizeof(buf),
                      seconds, WORD_TIME_DEFAULT));

 return (seconds);
}

static void timed_remove_no_logon(int timed_type,
                                  void *passed_player_tree_node)
{
 player_tree_node *current = passed_player_tree_node;
 
 IGNORE_PARAMETER(current);
 
 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
}

static void user_su_timer_no_logon(player *p, const char *str)
{
 player_tree_node *togo = NULL;

 CHECK_DUTY(p);

 if (internal_timer_player_setup(p, str, &timer_queue_player_no_logon,
                                 "no logon", &togo) &&
     P_IS_ON(togo))
   logoff_all(togo->lower_name);
}

static void timed_remove_no_move(int timed_type,
                                 void *passed_player_tree_node)
{
 player_tree_node *current = passed_player_tree_node;

 IGNORE_PARAMETER(current);

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
}

static void user_su_timer_no_move(player *p, const char *str)
{
 CHECK_DUTY(p);
 
 internal_timer_player_setup(p, str, &timer_queue_player_no_move,
                             "no move", NULL);
}

static void timed_remove_no_shout(int timed_type,
                                  void *passed_player_tree_node)
{
 player_tree_node *current = passed_player_tree_node;

 IGNORE_PARAMETER(current);
 
 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;
}

static void user_su_timer_no_shout(player *p, const char *str)
{
 CHECK_DUTY(p);
 
 internal_timer_player_setup(p, str, &timer_queue_player_no_shout,
                             "no shout", NULL);
}

static void timed_remove_jail(int timed_type, void *passed_player_tree_node)
{
 player_tree_node *current = passed_player_tree_node;
 
 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 if (!P_IS_ON(current))
   return;
 
 if (MODE_IN_MODE(current->player_ptr, JAIL))
 {
  fvtell_player(SYSTEM_FT(HILIGHT, current->player_ptr), "%s",
                " After serving your sentence you are flung out"
                " into society again.\n");
  
  mode_del(current->player_ptr);
 }
 else
 {
  assert(FALSE);
 }
}

static void user_su_timer_jail(player *p, const char *str)
{
 player_tree_node *togo = NULL;
 
 CHECK_DUTY(p);
 
 if (internal_timer_player_setup(p, str, &timer_queue_player_jail,
                                 "jail", &togo) &&
     P_IS_ON(togo))
   room_enter_jail(togo->player_ptr, ROOM_TRANS_DEFAULT);
}

void init_timer_player(void)
{
 timer_q_add_static_base(&timer_queue_player_no_logon, timed_remove_no_logon,
                         TIMER_Q_FLAG_BASE_INSERT_FROM_END |
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY |
                         TIMER_Q_FLAG_BASE_MALLOC_DOUBLE);
 timer_q_add_static_base(&timer_queue_player_no_move, timed_remove_no_move,
                         TIMER_Q_FLAG_BASE_INSERT_FROM_END |
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY |
                         TIMER_Q_FLAG_BASE_MALLOC_DOUBLE);
 timer_q_add_static_base(&timer_queue_player_no_shout, timed_remove_no_shout,
                         TIMER_Q_FLAG_BASE_INSERT_FROM_END |
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY |
                         TIMER_Q_FLAG_BASE_MALLOC_DOUBLE);
 timer_q_add_static_base(&timer_queue_player_jail, timed_remove_jail,
                         TIMER_Q_FLAG_BASE_INSERT_FROM_END |
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY |
                         TIMER_Q_FLAG_BASE_MALLOC_DOUBLE);
}

void cmds_init_timer_player(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("drag", user_su_timer_no_logon, CONST_CHARS, SU);
 CMDS_FLAG(no_expand); CMDS_PRIV(normal_su);
 CMDS_ADD("jail", user_su_timer_jail, CONST_CHARS, SU);
 CMDS_PRIV(normal_su);
 CMDS_ADD("rm_move", user_su_timer_no_move, CONST_CHARS, ADMIN);
 CMDS_PRIV(admin);
 CMDS_ADD("rm_shout", user_su_timer_no_shout, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);
}
