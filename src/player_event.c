#define PLAYER_EVENT_C
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


static void player_event_do_real_quit(player *p)
{
 logoff_real(p);
 player_destroy(p);
}

static void player_event_do_quit_msgs(player *p)
{
 const char *disconnect_msg = p->disconnect_msg;
 
 log_assert(P_IS_ON_P(p->saved, p));

 if (!disconnect_msg[0])
   disconnect_msg = configure.room_disconnect_msg;
 
 vtell_room_movement(p->location, p, " << %s%s%s%.*s^N >>\n",
                     gender_choose_str(p->gender, "", "", "The ", "The "),
                     p->saved->name, isits1(disconnect_msg), 
                     OUT_LENGTH_CONNECT_MSGS, isits2(disconnect_msg));
 
 logon_inform(p, "[$F-Name $F-Gender(plural(are) def(is)) no longer "
              "in $Talker-Name] $F-Dns_address");
}

static void player_event_do_run_command(player *p)
{
 int ret = 0;
 int had_idle_msg = FALSE;
 
 cmds_last_ran = &cmds_dummy_input_to;
 cmds_last_ran->func = MODE_CURRENT(p).cmd_func;

 if (p->idle_msg[0])
   had_idle_msg = TRUE;
 
 ret = cmds_run_func(&MODE_CURRENT(p).cmd_func,
                     p, p->input_start->input, p->input_start->length);
 cmds_last_ran = NULL;
 
 if (p->event == PLAYER_EVENT_RECONNECTION)
   return;

 if (ret)
 {
  idle_check_increment_total(p);
  if (had_idle_msg)
    p->idle_msg[0] = 0;
  p->flag_tmp_idle_had_warn = FALSE;
  p->last_command_timestamp = now;
 }
 
 if (p->input_start->comp_generated)
   ++p->input_comp_count;
 else
   p->input_comp_count = 0;
 input_del(p, p->input_start);
 
 if (MODE_CURRENT(p).prompt)
   prompt_update(p, MODE_CURRENT(p).prompt);

 if (((p->input_comp_count > 8) && !p->flag_input_keep_going) ||
     (p->input_comp_count > 400) ||
     (p->input_node_count > INPUT_NODE_QUEUE_SOFT_SZ))
   input_del_all_comp(p);
}

static int player_event_do_output(player *scan)
{
 int more_left = 0;
 
 more_left = output_for_player(scan);
 
 output_for_player_cleanup(scan);
 
 if (!more_left)
  player_list_io_del(scan);

 return (more_left);
}

int player_event_do(player_linked_list *passed_scan,
                    va_list ap __attribute__ ((unused)))
{
 player *scan = PLAYER_LINK_GET(passed_scan);
 
 STRACE("player_event_do");

 current_player = scan;
 
 if (!scan->io_indicator ||
     (SOCKET_POLL_INDICATOR(scan->io_indicator)->fd == -1) ||
     (SOCKET_POLL_INDICATOR(scan->io_indicator)->revents &
      (POLLERR | POLLHUP | POLLNVAL)))
 { /* NVAL is bad fd -- shouldn't happen */
  assert(!scan->io_indicator ||
         !(SOCKET_POLL_INDICATOR(scan->io_indicator)->revents & POLLNVAL));
  
  PLAYER_EVENT_UPGRADE(scan, BAD_FD);
  user_logoff(scan, NULL);
 }

 switch (scan->event)
 {
  case PLAYER_EVENT_CHOOSE:
    assert(scan->io_indicator);
    
    if (scan->input_start->ready && scan->allow_run_commands &&
        schedule_can_go(scan))
      PLAYER_EVENT_UPGRADE(scan, RUN_COMMAND);
        
    if (SOCKET_POLL_INDICATOR(scan->io_indicator)->revents & POLLIN)
      PLAYER_EVENT_UPGRADE(scan, INPUT);

    if (SOCKET_POLL_INDICATOR(scan->io_indicator)->revents & POLLOUT)
    {
     assert(player_list_io_find(scan));
     PLAYER_EVENT_UPGRADE(scan, OUTPUT);
    }
    break;
    
  case PLAYER_EVENT_INPUT:
    assert(scan->io_indicator);

    scan->event = PLAYER_EVENT_CHOOSE;
    
    socket_player_input(scan);
    break;

  case PLAYER_EVENT_OUTPUT:
    assert(scan->io_indicator);
    
    scan->event = PLAYER_EVENT_CHOOSE;
    
    player_event_do_output(scan);
    break;
    
  case PLAYER_EVENT_RUN_COMMAND:
  {
   struct timeval storage;
   
   assert(scan->io_indicator);

   scan->event = PLAYER_EVENT_CHOOSE;
   
   schedule_timer_start(scan, &storage);
   
   player_event_do_run_command(scan);
   
   schedule_timer_end(scan, &storage);
  }
  break;

  case PLAYER_EVENT_LOGOFF:
    if ((difftime(now, scan->logoff_started_timestamp) < LOGOFF_TIMEOUT) &&
        player_list_io_find(scan))
      {
       if (!player_event_do_output(scan) && scan->io_indicator)
         shutdown(SOCKET_POLL_INDICATOR(scan->io_indicator)->fd, SHUT_RD);
       break;
      }
    
    /* FALLTHROUGH */
  case PLAYER_EVENT_BAD_FD:
    if (scan->is_fully_on && !scan->flag_tmp_dont_do_normal_msgs)
      player_event_do_quit_msgs(scan);
    /* FALLTHROUGH */
  case PLAYER_EVENT_RECONNECTION:
    player_event_do_real_quit(scan);
    break;
  
  default:
    log_assert(FALSE);
 }
 
 current_player = NULL;

 STRACE("player_event_do");
 
 return (TRUE);
}

