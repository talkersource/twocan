#define NUKE_C
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

Timer_q_base nuke_timer_queue;


static void timed_nuke(int timed_type, void *passed_player_tree_node)
{
 player_tree_node *sp = passed_player_tree_node;

 TCTRACE("timed_nuke_do");

 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return; 

 timer_q_del_data(&timer_queue_player_jail, sp);
 timer_q_del_data(&timer_queue_player_no_logon, sp);
 timer_q_del_data(&timer_queue_player_no_move, sp);
 timer_q_del_data(&timer_queue_player_no_shout, sp);

 while (sp->mail_recieved_start)
   mail_del_mail_recipient(sp->mail_recieved_start->mail,
                           sp->mail_recieved_start);
 
 while (sp->mail_sent_start)
   mail_destroy_mail_sent(sp, sp->mail_sent_start);
 
 while (sp->rooms_start)
   room_del(sp->rooms_start);
 
 list_room_glob_load_cleanup(sp);
 
 if (P_IS_AVL(sp))
 {
  assert(timer_q_find_data(&load_player_timer_queue, sp));
  timer_q_del_node(&load_player_timer_queue, &sp->load_timer);
 
  player_load_cleanup(sp->player_ptr);
  
  sp->player_ptr = NULL;
 }

 spodlist_remove_player(sp->lower_name);
 
 if (PRIV_STAFF(sp))
   player_list_perm_staff_del(sp);
 
 if (sp->priv_spod)
   player_list_spod_del(sp);

 multis_destroy_for_player(sp);

 while (sp->channels_start)
   channels_del_system(sp->channels_start->base->name, sp);
 
 player_file_remove(sp->lower_name);
 
 no_of_resis--;
 
 XFREE(sp, PLAYER_TREE_NODE);
}

static void user_su_nuke_player(player *p, const char *str)
{
 struct timeval tv;
 player_tree_node *sp = NULL;
 parameter_holder params;
 
 get_parameter_init(&params);
 
 if (!get_parameter_parse(&params, &str, 1) ||
     (!*str && !p->saved->priv_admin))
   TELL_FORMAT(p, "<player> <reason>");
 
 CHECK_DUTY(p);

 lower_case(GET_PARAMETER_STR((&params), 1));
 
 if (!(sp = player_find_all(p, GET_PARAMETER_STR(&params, 1),
                            PLAYER_FIND_DEFAULT)))
 { /* player being nuked is not a saved player */
  if ((sp = player_newbie_find_exact(GET_PARAMETER_STR((&params), 1))))
  {
   fvtell_player(NORMAL_T(p), " -=> %s is a ^S^Bnewbie^s.\n", sp->name);
   fvtell_player(SYSTEM_T(sp->player_ptr), "%s",
                 " You are being removed from the program.\n"
                 " Be nice next time.\n");
   stats_log_event(sp->player_ptr, STATS_RESI_OFF, STATS_RESI_OFF_FORCED);
   user_logoff(sp->player_ptr, NULL);
  }
  else
  {
   fvtell_player(NORMAL_T(p), " The string -- ^S^B%s^s -- does not match the"
                 " start of any players name.\n",
                 GET_PARAMETER_STR(&params, 1));
  }
  
  return;
 }

 if (timer_q_find_data(&nuke_timer_queue, sp))
 {
  fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- is system banished, "
                "which means you can't nuke them.\n",
                sp->name);
  return;
 }

 if (!priv_test_user_check(p->saved, sp, "nuke", 1))
   return;

 gettimeofday(&tv, NULL);

 TIMER_Q_TIMEVAL_ADD_SECS(&tv, NUKE_PLAYER_TIME, 0);
                          
 if (timer_q_add_node(&nuke_timer_queue, sp, &tv, TIMER_Q_FLAG_NODE_DEFAULT))
 {
  P_MEM_ERR(p);  
  return;
 }

 if (!P_IS_ON(sp))
 {
  player_load(sp);

  fvtell_player(NORMAL_T(p), "%s", " Player not logged on presently.\n"
                " Please email them if possible and appropriate.\n");
 }
 else
 {
  stats_log_event(sp->player_ptr, STATS_RESI_OFF, STATS_RESI_OFF_FORCED);
  
  fvtell_player(SYSTEM_T(sp->player_ptr), "%s",
                "\n$Bell\n ^B-=> You've just been nuked.^b\n"
                "     You have lost residency.\n"
                "     All your character information and login time has "
                "been removed.\n"
                "     Goodbye.\n$Bell\n");
 }
 
 sp->flag_tmp_player_needs_saving = TRUE;
 sp->priv_banished = TRUE;
 
 logoff_all(sp->lower_name);

 /* these messages aren't true... but fuck it */
 channels_wall("staff", 3, NULL, " -=> %s nuked %s -- %s",
               p->saved->name, sp->name, str);

 vwlog("nuke", "%s nuked %s -- %s\n", p->saved->name, sp->name, str); 
}

void init_nuke(void)
{
 timer_q_add_static_base(&nuke_timer_queue, timed_nuke,
                         TIMER_Q_FLAG_BASE_DEFAULT);
}

void cmds_init_nuke(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("nuke", user_su_nuke_player, CONST_CHARS, SU);
 CMDS_FLAG(no_expand); CMDS_PRIV(normal_su);
}
