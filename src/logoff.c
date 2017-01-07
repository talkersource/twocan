#define LOGOFF_C
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




/* fucntions... */


static void logoff_player_generic(player_tree_node *current)
{
 assert(MALLOC_VALID(current, sizeof(player_tree_node), PLAYER_TREE_NODE));
 
 assert(P_IS_ON(current));
 assert(current->player_ptr->location);

 current->player_ptr->is_fully_on = FALSE;

 alarm_cleanup_player(current->player_ptr);
 
 room_player_del(current->player_ptr, current->player_ptr->location);
 
 stats_log_event(current->player_ptr, STATS_RESI_OFF, STATS_NO_EXTRA);
 session_player_leave(current->player_ptr);
 --current_players;
 player_list_alpha_del(current);

 if (current->player_ptr->assisted_player)
 {
  player *p2 = current->player_ptr->assisted_player;
  p2->assisted_player = NULL;
  current->player_ptr->assisted_player = NULL;
 }

 if (PRIV_STAFF(current)) /* EW compatability */
   channels_add_system("staff", current);
 
 list_load_cleanup(LIST_TYPE_SELF, current->player_ptr->list_self_tmp_start);
 current->player_ptr->list_self_tmp_start = NULL;
 
 list_load_cleanup(LIST_TYPE_COMS, current->player_ptr->list_coms_tmp_start);
 current->player_ptr->list_coms_tmp_start = NULL;
 
 current->player_ptr->allow_run_commands = FALSE;
 current->player_ptr->schedule_time = 0;
 
 /* kill_sps_played(p); checks if there has been a game*/
 
 /* checks if they are watching first */
 draughts_stop_watching_game(current->player_ptr, 0);
 /* will check if there's even a game */
 draughts_lose_game(current->player_ptr, DRAUGHTS_LOST_QUIT);
 
 if (current->player_ptr->ttt_playing)
   user_ttt_quit(current->player_ptr);

 if (current->player_ptr->hangman)
   hangman_clear(current->player_ptr);
 
 idle_timer_stop(current->player_ptr);
 
 MODE_DEL_ALL(current->player_ptr);
}

void logoff_player_update(player *p)
{
 int login_time = difftime(now, p->logon_timestamp) - p->idle_logon;
 
 qstrcpy(p->saved->last_dns_address, p->dns_address);
 memcpy(p->saved->last_ip_address, p->ip_address, 4);
 
 if (login_time > system_data.longest_spod_time)
 {
  qstrcpy(system_data.longest_spod_name, p->saved->name);
  system_data.longest_spod_time = login_time;
 }

 p->saved->logoff_timestamp = now;
  
 p->saved->flag_tmp_player_needs_saving = TRUE;

 idle_check_increment_total(p->saved->player_ptr);
 p->saved->total_logon += difftime(now, p->logon_timestamp);
 p->saved->total_idle_logon += p->idle_logon;
 p->logon_timestamp = now;
 p->idle_logon = 0;
}

static void logoff_player_resident(player *p)
{
 player_tree_node *current = p->saved;

 BTRACE("logoff_player_resident");
 
 assert(MALLOC_VALID(current, sizeof(player_tree_node), PLAYER_TREE_NODE));
 assert(MALLOC_VALID(p, sizeof(player), PLAYER));
 assert(p && player_tree_find_exact(p->saved->lower_name));
 
 if (P_IS_ON_P(current, p)) /* check for reconnections etc... */
 {
  timer_q_del_data(&scripting_timer_queue, p);

  quit_in_cleanup_player(p);
  
  mail_load_cleanup_all(current); /* FIXME: ? */
  
  spodlist_check_order(current, SPOD_CHECK_EXTERNAL, SPOD_CHECK_DEF_LEVEL);
  
  logoff_player_update(p);

  last_logon_add(p->saved->lower_name);

  logoff_player_generic(current);
  
  if (PRIV_STAFF(current))
    player_list_logon_staff_del(current);
 }
 else
   p->saved = NULL;
}

static void logoff_player_newbie(player *p)
{
 player_tree_node *current = p->saved;
  
 BTRACE("logoff_player_newbie");

 assert(MALLOC_VALID(current, sizeof(player_tree_node), PLAYER_TREE_NODE));
 
 assert(!player_tree_find_exact(current->lower_name));

 assert(!timer_q_find_data(&logon_timer_queue, p));
 assert(!timer_q_find_data(&scripting_timer_queue, p));
 assert(!timer_q_find_data(&load_player_timer_queue, current));
 
 timer_q_del_data(&priv_resident_timer_queue, p);

 if (current)
 {
  timer_q_del_data(&timer_queue_player_jail, current);
  timer_q_del_data(&timer_queue_player_no_logon, current);
  timer_q_del_data(&timer_queue_player_no_move, current);
  timer_q_del_data(&timer_queue_player_no_shout, current);

  if (P_IS_ON_P(p->saved, p))
  {
   logoff_player_generic(current);
   
   player_newbie_del(current);
  }
  
  multis_destroy_for_player(current);
  
  XFREE(current, PLAYER_TREE_NODE);
 }
 
 p->saved = NULL;
}

void logoff_real(player *p)
{ /* logoff_generic that's needed even if !p->saved */
 BTRACE("logoff_real");
 
 while (p->input_start)
   input_del(p, p->input_start);
 
 timer_q_del_data(&logon_timer_queue, p);

#ifdef HAVE_ZLIB_H
 if (p->output_compress_lib)
   output_compress_stop_2(p);
#endif
 
 if (p->saved && !p->saved->priv_base)
 {
  if (!p->is_fully_on)
  {
   if (logging_onto_count > 0)
     --logging_onto_count; /* they have not logged on properly */
   else
     log_assert(FALSE);
  }

  logoff_player_newbie(p);
 }
 else if (!p->is_fully_on)
 {
  assert(!p->location);
  assert(logging_onto_count > 0);

  p->saved = NULL;
  
  if (logging_onto_count > 0)
    --logging_onto_count; /* they have not logged on properly */
  else
    log_assert(FALSE);
 }
 else
 {
  assert(p->location);
  assert(p->saved);

  log_assert(p->saved->priv_base);

  logoff_player_resident(p);
 }
}

void player_destroy(player *p)
{ 
 BTRACE("player_destroy");
 
 player_list_cron_del(p);

 if (player_list_io_find(p)) /* output stuff should have gone
                              * via. player_event loop */
   player_list_io_del(p);
 
 output_list_cleanup(&p->output_start);
 output_list_cleanup(&p->output_buffer_tmp);
 
 if (gettimeofday(&last_entered_left, NULL))
 {
  assert(FALSE);
 }
 
 if (p->io_indicator)
 {
  int fd = SOCKET_POLL_INDICATOR(p->io_indicator)->fd;
  socket_poll_del(p->io_indicator);
  close(fd);
 }
 p->io_indicator = 0;
 
 if (p->saved)
 {
  assert(!p->location);
   
  p->saved->flag_tmp_player_needs_saving = TRUE;
  
  player_save_all(p);
 }
 else
 { /* newbie */
  player_load_cleanup(p);
  XFREE(p, PLAYER);
 }
}

static int internal_logoff_all(player_linked_list *passed_scan, va_list ap)
{
 const char *name = va_arg(ap, const char *);
 int *count = va_arg(ap, int *);
 player *scan = PLAYER_LINK_GET(passed_scan);

 if (!scan->saved || strcmp(scan->saved->lower_name, name))
   return (TRUE);

 user_logoff(scan, NULL);
 if (!P_IS_ON_P(scan->saved, scan))
   scan->saved = NULL;
 
 ++*count;

 return (TRUE);
}

int logoff_all(const char *name)
{
 int count = 0;

 do_order_misc_on_all(internal_logoff_all, player_list_cron_start(),
                      name, &count);

 return (count);
}

void user_logoff(player *p, const char *str)
{
 IGNORE_PARAMETER(str);
 
 if (str)
 { /* don't let them autoexec quit */
  BTRACE("user_logoff");
  
  if (!p->typed_commands)
  {
   fvtell_player(SYSTEM_T(p), " If I let you do that you would"
                 " autoexec quit!\n");
   return;
  }

  if (p->saved)
  {
   if (p->saved->priv_base)
     fvtell_player(NORMAL_T(p), 
                   "\n Thank you for using...\n\n%s"
                   "\n\n          We hope to see you again, %s.\n\n",
                   configure.name_ascii_art,
                   p->saved->name);
   else
     fvtell_player(NORMAL_T(p), 
                   "\n Thank you for using...\n\n%s"
                   "\n\n         %s you can email $Talker-Email if you want "
                   "to get a saved player.\n\n",
                   configure.name_ascii_art,
                   p->saved->name);
  }

  PLAYER_EVENT_UPGRADE(p, LOGOFF);
 }
 else
 {
  BTRACE("user_logoff");
  PLAYER_EVENT_UPGRADE(p, LOGOFF);
 }
 
 if (p->output_compress_do)
   telopt_ask_compress_stop(p);
 
 if (p->io_indicator)
 {
  SOCKET_POLL_INDICATOR(p->io_indicator)->events &= ~POLLIN;
  socket_update_player(p);
 }

 p->logoff_started_timestamp = now;
}

void cmds_init_logoff(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("quit", user_logoff, CONST_CHARS, SYSTEM);
 CMDS_FLAG(no_expand);
 CMDS_XTRA_MISC(RESTRICTED); /* should probably let them quit :) */
}
