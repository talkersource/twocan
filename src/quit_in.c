#define QUIT_IN_C
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

Timer_q_base quit_in_timer_queue;

static void user_quit_in(player *p, const char *str)
{
 unsigned long quit_in = 0;
 struct timeval tv;
 char buf[256];
 parameter_holder params;

 get_parameter_init(&params);

 if (!get_parameter_parse(&params, &str, 1))
   TELL_FORMAT(p, "<period>");

 quit_in = word_time_parse(GET_PARAMETER_STR(&params, 1),
                           WORD_TIME_PARSE_DEFAULT, NULL);
 
 if (!quit_in)
 {
  if (timer_q_del_data(&quit_in_timer_queue, p))
    fvtell_player(NORMAL_T(p), "%s",
                  " You have removed your quit in timer.\n");
  else
    fvtell_player(NORMAL_T(p), "%s",
                  " You haven't set up a time to quit in yet.\n");
  return;
 }
 
 if (timer_q_find_data(&quit_in_timer_queue, p))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You have already set up a quit in timer.\n");
  return;
 }

 gettimeofday(&tv, NULL);
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, quit_in, 0);
 if (!timer_q_add_node(&quit_in_timer_queue, p, &tv,
                       TIMER_Q_FLAG_NODE_DEFAULT))
   P_MEM_ERR(p);

 fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p),
               " You will quit $Talker-Name in ^S^B%s^s.\n",
               word_time_long(buf, sizeof(buf),
                              quit_in, WORD_TIME_DEFAULT));
}

void quit_in_cleanup_player(player *p)
{
 timer_q_del_data(&quit_in_timer_queue, p);
}

static void internal_quit_in_timer_func(int type, void *data)
{
 player *p = data;
 
 if (type == TIMER_Q_TYPE_CALL_RUN_NORM)
   user_logoff(p, ""); /* so you can't put quit_in 1 in your aliases either */
}

void init_quit_in(void)
{
 timer_q_add_static_base(&quit_in_timer_queue, internal_quit_in_timer_func,
                         TIMER_Q_FLAG_BASE_MALLOC_FUNC |
                         TIMER_Q_FLAG_BASE_RUN_ALL |
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY);
}

void cmds_init_quit_in(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("quit_in", user_quit_in, CONST_CHARS, SYSTEM);
 CMDS_FLAG(no_expand);
 CMDS_XTRA_MISC(RESTRICTED); /* should probably let them quit_in too
                              * (and stop a quit_in) */
 CMDS_PRIV(base);
}
