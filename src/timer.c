#define TIMER_C
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

#if 1

Timer_q_base timer_queue_global;
struct timeval now_timeval;
time_t now;

global_timers glob_timer =
{
 0, 0,
};

#else
/* _don't_ forget nuke if you add timers */
timer_node *timer_queue_player_idle = NULL;

timer_node *timer_queue_player_jail = NULL;
timer_node *timer_queue_player_logon = NULL;
timer_node *timer_queue_player_no_logon = NULL;
timer_node *timer_queue_player_no_move = NULL;
timer_node *timer_queue_player_no_shout = NULL;
timer_node *timer_queue_player_nuke = NULL;
timer_node *timer_queue_player_res = NULL;
timer_node *timer_queue_player_scripting = NULL;

timer_node *timer_queue_global = NULL;

timer_node *timer_queue_room_autos = NULL;
timer_node *timer_queue_auth_player = NULL;

timer_node *timer_queue_list_cleanup = NULL;
timer_node *timer_queue_mail_cleanup = NULL;
timer_node *timer_queue_news_cleanup = NULL;
timer_node *timer_queue_player_cleanup = NULL;
timer_node *timer_queue_room_cleanup = NULL;

timer_node *timer_queue_channels = NULL;

global_timers glob_timer =
{
 0, 0,
 {{NULL, NULL, NULL, 0, 0, FALSE, FALSE}, NULL}, /* anoying egcs warnings */
 {{NULL, NULL, NULL, 0, 0, FALSE, FALSE}, NULL}, /* anoying egcs warnings */
 {{NULL, NULL, NULL, 0, 0, FALSE, FALSE}, NULL}, /* anoying egcs warnings */
 {{NULL, NULL, NULL, 0, 0, FALSE, FALSE}, NULL}, /* anoying egcs warnings */
};


/* NOTE: doesn't init next or prev */
timer_node *timer_init_node(timer_node *passed,
                            void (*func)(int, void *), void *data,
                            int have_prev)
{
 if (passed)
   passed->do_free = FALSE;
 else
 {
  if (have_prev)
    passed = XMALLOC(sizeof(timer_double_node), TIMER_DOUBLE_NODE);
  else
    passed = XMALLOC(sizeof(timer_node), TIMER_NODE);
  
  if (!passed)
    return (NULL);
  
  passed->do_free = TRUE;
 }

 passed->has_prev = have_prev;
 passed->func = func;
 passed->data = data;
 
 return (passed);
}

static timer_node **internal_timer_find_data(timer_node **scan, void *data)
{
 assert(data);

 while (*scan)
 {
  if (data == (*scan)->data)
    return (scan);
  
  scan = &(*scan)->next;
 }

 return (NULL);
}

timer_node *timer_find_data(timer_node *scan, void *data)
{
 timer_node **ret = NULL;

 if ((ret = internal_timer_find_data(&scan, data)))
   return (*ret);
 
 return (NULL);
}

void timer_add_node(timer_node **start, timer_node *addto)
{
 timer_node **tmp = NULL;

 assert(addto && start);

 tmp = start;

 if (!*tmp)
 {
  *start = addto;
  addto->next = NULL;
  if (addto->has_prev)
    ((timer_double_node *) addto)->prev = NULL;

#ifndef NDEBUG
  {
   timer_node *scan = *start;
   
   assert(!scan || !scan->has_prev || !((timer_double_node *) scan)->prev);
   
   while (scan)
   {
    timer_node *scan_next = scan->next;
    
    assert(!scan_next || !scan_next->has_prev ||
           (((timer_double_node *) scan_next)->prev == scan));
    
    scan = scan_next;
   }
  }
#endif
  return;
 }
 
 if (TIMER_TOGO(*tmp, timer) > TIMER_TOGO(addto, timer))
 {
  if ((addto->next = *tmp) && addto->next->has_prev)
  {
   assert(!((timer_double_node *) addto->next)->prev);
   ((timer_double_node *) addto->next)->prev = addto;
  }
  
  if (addto->has_prev)
    ((timer_double_node *) addto)->prev = NULL;
  
  *tmp = addto;

#ifndef NDEBUG
  {
   timer_node *scan = *start;
   
   assert(!scan || !scan->has_prev || !((timer_double_node *) scan)->prev);
   
   while (scan)
   {
    timer_node *scan_next = scan->next;
    
    assert(!scan_next || !scan_next->has_prev ||
           (((timer_double_node *) scan_next)->prev == scan));
    
    scan = scan_next;
   }
  }
#endif
  return;
 }
 
 while ((*tmp)->next &&
        (TIMER_TOGO((*tmp)->next, timer) < TIMER_TOGO(addto, timer)))
   tmp = &(*tmp)->next;
 
 if ((addto->next = (*tmp)->next) && addto->next->has_prev)
   ((timer_double_node *) addto->next)->prev = addto;
 
 (*tmp)->next = addto;
 
 if (addto->has_prev)
   ((timer_double_node *) addto)->prev = *tmp;

#ifndef NDEBUG
 {
  timer_node *scan = *start;
  
  assert(!scan || !scan->has_prev || !((timer_double_node *) scan)->prev);
  
  while (scan)
  {
   timer_node *scan_next = scan->next;
   
   assert(!scan_next || !scan_next->has_prev ||
          (((timer_double_node *) scan_next)->prev == scan));
   
   scan = scan_next;
  }
 }
#endif
}

static void internal_timer_del_node(timer_node **start, timer_node **current,
                                    int just_unlink)
{
 timer_node *tmp = *current;

 if (start == current)
 {
  if ((*current)->next && (*current)->next->has_prev)
    ((timer_double_node *) (*current)->next)->prev = NULL;
 }
 else
 {
  timer_node *prev = NULL;
  
  /* ANSI magic */
  prev = (timer_node *)(((char *)current) - offsetof(timer_node, next));

  if ((*current)->next && (*current)->next->has_prev)
    ((timer_double_node *) (*current)->next)->prev = prev;
 }
 
 *current = (*current)->next;

 if (!just_unlink)
 {
  (*tmp->func)(TIMER_TYPE_DEL, tmp->data);
  
  if (tmp->do_free)
    XFREE(tmp, TIMER_NODE);
 }

#ifndef NDEBUG
 {
  timer_node *scan = *start;
  
  assert(!scan || !scan->has_prev || !((timer_double_node *) scan)->prev);
  
  while (scan)
  {
   timer_node *scan_next = scan->next;
   
   assert(!scan_next || !scan_next->has_prev ||
          (((timer_double_node *) scan_next)->prev == scan));
   
   scan = scan_next;
  }
 }
#endif
}

void timer_del_data(timer_node **start, void *data)
{
 timer_node **ret = internal_timer_find_data(start, data);

 if (ret)
   internal_timer_del_node(start, ret, FALSE);
}

void timer_del_node(timer_node **start, timer_node *current)
{
 if (*start == current)
   internal_timer_del_node(start, start, FALSE);
 else if (current->has_prev)
 { /* *start would == current otherwise */
  assert(((timer_double_node *) current)->prev);
  internal_timer_del_node(start, &((timer_double_node *) current)->prev->next,
                          FALSE);
 }
 else
   timer_del_data(start, current->data);
}

int timer_run_q(timer_node **start)
{
 timer_node *scan = NULL;
 int called = 0;
 
 assert(start);
 if (!start)
   return (0);
 
 scan = *start;

#ifndef NDEBUG
 assert(!scan || !scan->has_prev || !((timer_double_node *) scan)->prev);

 while (scan)
 {
  timer_node *scan_next = scan->next;

  assert(!scan_next || !scan_next->has_prev ||
         (((timer_double_node *) scan_next)->prev == scan));
  
  scan = scan_next;
 }
 scan = *start;
#endif
 
 while (scan && TIMER_IS_DONE(scan, timer))
 {
  timer_node *scan_next = scan->next;

  assert(!scan_next || !scan_next->has_prev ||
         (((timer_double_node *) scan_next)->prev == scan));
  assert(*start == scan);
  
  internal_timer_del_node(start, start, TRUE);
  ++called;

  BTRACE("timer_run_func");
  (*scan->func)(TIMER_TYPE_RUN, scan->data);

  if (scan->do_free)
    XFREE(scan, TIMER_NODE);
  
  scan = scan_next;
 }

 return (called);
}

int timer_exec_run_q(timer_node **start) /* ignore timestamps */
{
 timer_node *scan = NULL;
 int called = 0;
 
 assert(start);
 if (!start)
   return (0);
 
 scan = *start;
 
 while (scan)
 {
  timer_node *scan_next = scan->next;

  ++called;

  BTRACE("timer_exec_run_func");
  (*scan->func)(TIMER_TYPE_EXEC_RUN, scan->data);
  
  scan = scan_next;
 }

 return (called);
}
#endif

void timer_run_do(void)
{
 now = time(NULL);
 gettimeofday(&now_timeval, NULL);
 
 timer_q_run_norm(&now_timeval);

#if 0
 timer_run_q(&timer_queue_player_idle);

 timer_run_q(&timer_queue_player_jail);
 timer_run_q(&timer_queue_player_logon);
 timer_run_q(&timer_queue_player_no_logon);
 timer_run_q(&timer_queue_player_no_move);
 timer_run_q(&timer_queue_player_no_shout);
 timer_run_q(&timer_queue_player_nuke);
 timer_run_q(&timer_queue_player_res);
 timer_run_q(&timer_queue_player_scripting);
 
 timer_run_q(&timer_queue_global);

 timer_run_q(&timer_queue_room_autos);

 timer_run_q(&timer_queue_auth_player);

 timer_run_q(&timer_queue_list_cleanup);
 timer_run_q(&timer_queue_mail_cleanup);
 timer_run_q(&timer_queue_news_cleanup);
 timer_run_q(&timer_queue_player_cleanup);
 timer_run_q(&timer_queue_room_cleanup);

 timer_run_q(&timer_queue_channels);
#endif
 
 /* special timer */
 shutdown_do_timer();
}

void timer_exec_run_do(void)
{
 timer_q_run_all();

#if 0
 timer_exec_run_q(&timer_queue_global);

 timer_exec_run_q(&timer_queue_list_cleanup);
 timer_exec_run_q(&timer_queue_mail_cleanup);
 timer_exec_run_q(&timer_queue_news_cleanup);
 timer_exec_run_q(&timer_queue_player_cleanup);
 timer_exec_run_q(&timer_queue_room_cleanup);

 timer_exec_run_q(&timer_queue_channels);
#endif
}

static void timer_broken_call_back(int type, void *data)
{
 IGNORE_PARAMETER(type && data);
 
 assert_log(FALSE);
}

void init_timer(void)
{
 timer_q_add_static_base(&timer_queue_global, timer_broken_call_back,
                         TIMER_Q_FLAG_BASE_MALLOC_FUNC |
                         TIMER_Q_FLAG_BASE_RUN_ALL |
                         TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY);
}
