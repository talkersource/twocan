#define ALARM_C
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

Timer_q_base alarm_timer_queue;

static void user_alarm(player *p, const char *str)
{
 unsigned long alarm_in = 0;
 struct timeval tv;
 char buf[256];
 parameter_holder params;
 size_t len = 0;
 
 get_parameter_init(&params);

 if (!get_parameter_parse(&params, &str, 1))
   TELL_FORMAT(p, "<period> [options]");
 
#if 0
 
 TELL_FORMAT(p, "<period|\"list\"|\"delete\"> [options]");
 
 if (!beg_strcasecmp(GET_PARAMETER_STR(&params, 1), "list"))
 {
  unsigned int count = 0;
  char head_buf[CONST_STRLEN("Used %d out of %d alarms") +
               (BUF_NUM_TYPE_SZ(int) * 2)];
  
  if (*str)
    TELL_FORMAT(p, "<\"list\">");

  gettimeofday(&tv, NULL);


  sprintf(head_buf, "Used %d out of %d alarms",
          p->alarm_num, ALARM_ALLOWED_SZ);
  
  ptell_mid(NORMAL_T(p), head_buf, FALSE);

  scan = p->alarm_start;
  while (scan)
  {
   const struct timeval *scan_tv = NULL;
   long diff_secs = 0;

   timer_q_cntl_node(scan->node, TIMER_Q_CNTL_NODE_GET_TIMEVAL, &scan_tv);
   
   diff_secs = timer_q_timeval_diff_secs(&tv, scan_tv);
   fvtell_player(NORMAL_T(p), " % 2d Alarm in ^S^B%s^s saying \"%.*s\"\n",
                 ++count, word_time_long(buf, sizeof(buf),
                                         diff_secs, WORD_TIME_DEFAULT),
                 scan->str);
   
   scan = scan->next;
  }
  assert(count == p->alarm_num);
  
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);

  return;
 }
 else if (!beg_strcasecmp(GET_PARAMETER_STR(&params, 1), "delete"))
 {
  if ((get_parameter_parse(&params, &str, 2) != 2) || *str)
   TELL_FORMAT(p, "<\"delete\"> <alarm number>");

  if (count > p->alarm_num)
  {
   fvtell_player(SYSTEM_T(p), " You only have -- ^S^B%d^s -- alarms active,"
                 " currently.\n", p->alarm_num);
   return;
  }
  
  scan = p->alarm_start;
  while (scan && (--count > 0))
    scan = scan->next;

  alarm_del(p, scan);
  
  return;
 }
#endif

 if (!*str)
   TELL_FORMAT(p, "<period> <message>");

#if 0
 if (p->alarm_num >= ALARM_ALLOWED_SZ)
 {
  
 }
#endif
 
 alarm_in = word_time_parse(GET_PARAMETER_STR(&params, 1),
                            WORD_TIME_PARSE_DEFAULT, NULL);
 
 if (!alarm_in)
 {
  if (timer_q_del_data(&alarm_timer_queue, p))
  {
   fvtell_player(NORMAL_T(p), "%s",
                 " You have removed your alarm timer.\n");
   FREE(p->alarm_str);
   p->alarm_str = NULL;
  }
  else
    fvtell_player(NORMAL_T(p), "%s",
                  " You haven't set up an alarm yet.\n");
  return;
 }

 if (timer_q_find_data(&alarm_timer_queue, p))
 {
  fvtell_player(SYSTEM_T(p), "%s",
                " You have already set up an alarm.\n");
  return;
 }
 
 len = strlen(str);
 if (!(p->alarm_str = MALLOC(len)))
   P_MEM_ERR(p);

 COPY_STR_LEN(p->alarm_str, str, len);
 
 gettimeofday(&tv, NULL);
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, alarm_in, 0);
 if (!timer_q_add_node(&alarm_timer_queue, p, &tv,
                       TIMER_Q_FLAG_NODE_DEFAULT))
   P_MEM_ERR(p);

 fvtell_player(NORMAL_T(p),
               " You set an alarm to go off in %s, with the "
               "message:\n%s ^N\n",
               word_time_long(buf, sizeof(buf),
                              alarm_in, WORD_TIME_DEFAULT), str);
}

void alarm_cleanup_player(player *p)
{
 if (timer_q_del_data(&alarm_timer_queue, p))
 {
  FREE(p->alarm_str);
  p->alarm_str = NULL;
 }
}

static void internal_alarm_timer_func(int type, void *data)
{
 player *p = data;

 log_assert(p->alarm_str);
 
 if (type == TIMER_Q_TYPE_CALL_RUN_NORM)
 { /* string comes FROM you, cause although its a system message to an
    * extent, it is something you requested and COULD have twinkles in */
  fvtell_player(NORMAL_FT(SYSTEM_INFO, p), 
                "\n^B ALARM -=> %s$Bell^N\n\n", p->alarm_str);
  FREE(p->alarm_str);
  p->alarm_str = NULL;
 }
}

void init_alarm(void)
{
 timer_q_add_static_base(&alarm_timer_queue, internal_alarm_timer_func,
                         TIMER_Q_FLAG_BASE_MALLOC_FUNC |
                         TIMER_Q_FLAG_BASE_RUN_ALL |
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY);
}

void cmds_init_alarm(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("alarm", user_alarm, CONST_CHARS, MISC);
}
