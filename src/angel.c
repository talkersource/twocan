#define ANGEL_C
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

int total_crashes;
time_t angel_started;

static Timer_q_func_node angel_timer_node;

static child_com the_parent = CHILD_COM_INIT(CHILD_COM_DEFAULT_SIZE);
static child_com *parent = NULL;

static void angel_close_parent(void)
{
 wlog("error", " angel parent killed.\n");
 socket_poll_del(parent->io_indicator);
 child_com_close(parent);
 parent = NULL;
}

static void timed_angel_ping(int timed_type, void *data)
{
 struct timeval tv;
 
 IGNORE_PARAMETER(data);

 printf("abcd\n");
 
 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 if (timed_type == TIMER_Q_TYPE_CALL_RUN_ALL)
   return;

 if (!parent)
   return;
 
 child_com_send(parent, "%s", "b");
 if (parent->bad || !child_com_flush(parent))
 {
  angel_close_parent();
  return;
 }
 
 gettimeofday(&tv, NULL);
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, 4, 0);
 timer_q_add_static_node(&angel_timer_node.s, &timer_queue_global,
                         &angel_timer_node, &tv, TIMER_Q_FLAG_NODE_FUNC);
 timer_q_cntl_node(&angel_timer_node.s, TIMER_Q_CNTL_NODE_SET_FUNC,
                   timed_angel_ping);
}

void angel_socket_do(void)
{
 if (!parent)
   return;
 
 child_com_recv(parent, NULL, 512);

 if (parent->bad || child_com_waiting_input(parent, 200))
   angel_close_parent();
}

void init_angel(void)
{
 size_t header_size = 0;
 time_t start = now = time(NULL);

 parent = &the_parent;
 child_com_open(parent, fileno(stdin), stdout, getppid());
 parent->io_indicator = socket_poll_add(fileno(stdin));
 SOCKET_POLL_INDICATOR(parent->io_indicator)->events |= POLLIN;

 start = now;
 socket_update_indicators(1);
 while (((header_size = child_com_gather(parent, '\n')) <= 0) &&
        (difftime(now, start) < ANGEL_TALKER_WAIT_BOOT))
 {
  socket_update_indicators(ANGEL_TALKER_POLL_BOOT);
  now = time(NULL);
 }
 
 if ((header_size = child_com_gather(parent, '\n')) > 0)
 {
  char buffer[1024];
  struct timeval tv;

  if (child_com_recv(parent, buffer, header_size) == header_size)
    buffer[header_size - 1] = 0;
  else
  {
   assert(FALSE);
  }

  if (!BEG_CONST_STRCMP("total_crashes:", buffer))
  {
   total_crashes = atoi(buffer + CONST_STRLEN("total_crashes:"));
  }

  if ((header_size = child_com_gather(parent, '\n')) > 0)
  {
   if (child_com_recv(parent, buffer, header_size) == header_size)
     buffer[header_size - 1] = 0; /* remove return */
   else
   {
    assert(FALSE);
   }

   if (!BEG_CONST_STRCMP("angel_started:", buffer))
   {
    angel_started = disp_time_string_cmp(buffer +
                                         CONST_STRLEN("angel_started:"));
   }
  }

  gettimeofday(&tv, NULL);
  TIMER_Q_TIMEVAL_ADD_SECS(&tv, 0, 500);
  timer_q_add_static_node(&angel_timer_node.s, &timer_queue_global,
                          &angel_timer_node, &tv, TIMER_Q_FLAG_NODE_FUNC);
  timer_q_cntl_node(&angel_timer_node.s, TIMER_Q_CNTL_NODE_SET_FUNC,
                    timed_angel_ping);
 }
 else
 {
  fprintf(stderr, "\n No angel.\n");
  socket_poll_del(parent->io_indicator);
  /* child_com_delete(parent); */
  /* don't call above or it'll kill gdb */

  parent = NULL; /* it's not the angel */
 }
}

void user_su_shutdown_angel(player *p, const char *str)
{
 int angel_signal = 0;
 
 /* 4 = TERM, 9 = KILL */
 if (!strcmp("soft", str))
   angel_signal = SIGTERM;
 else
   if (!strcmp("hard", str))
     angel_signal = SIGKILL;
   else
     
   {
    fvtell_player(NORMAL_T(p),
                  " This command will ^4^B_shutdown_^N the angel.\n"
                  " Use only if you are going to stop the program "
                  "for more than a second.\n");
    TELL_FORMAT(p, "[ soft | hard ]");
   }
 
 if (kill(getppid(), angel_signal) > -1)
   fvtell_player(NORMAL_T(p), "%s", " Successfully killed the angel.\n");
 else
   switch(errno)
   {
  case EINVAL:
    fvtell_player(SYSTEM_T(p), "%s", " That is not a valid signal.\n");
    break;
    
  case ESRCH:
    fvtell_player(SYSTEM_T(p), "%s", " The angel is not running.\n");
    break;

  case EPERM:
    fvtell_player(SYSTEM_T(p), "%s",
                  " The talker does not seem to have the privs to "
                  "kill the angel.\n");
    break;

  default:
    assert(FALSE);
    fvtell_player(SYSTEM_T(p), "%s", " This should not happen.\n");
   }
}

void cmds_init_angel(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("angel_shutdown", user_su_shutdown_angel, CONST_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(coder_admin);
}
