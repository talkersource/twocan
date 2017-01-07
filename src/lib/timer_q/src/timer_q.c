#define TIMER_Q_C
/*
 *  Copyright (C) 1999, 2000 James Antill
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * email: james@and.org
 */
#include "main.h"

/* ISO C magic, converts a ptr to ->next into ->next->prev */
#define TIMER_Q_CONV_PTR_NEXT_PREV(x) \
 ((Timer_q_node *)(((char *)(x)) - offsetof(Timer_q_node, next)))

#if !defined (NDEBUG) && 1

# define TIMER_Q__CHECK(base) do { \
  Timer_q_node *timer_q__local = (base)->start; \
  \
  assert(!timer_q__local || \
         !timer_q__local->has_prev || \
         !((Timer_q_double_node *) timer_q__local)->prev); \
  assert((base->start && base->end && !base->end->next) || !base->end); \
  \
  while (timer_q__local) \
  { \
   Timer_q_node *timer_q__local_next = timer_q__local->next; \
   \
   assert(!timer_q__local_next || \
          !timer_q__local->has_prev || \
          (((Timer_q_double_node *) timer_q__local_next)->prev == timer_q__local)); \
   \
   timer_q__local = timer_q__local_next; \
  } \
 } while (FALSE)
#else
# define TIMER_Q__CHECK(base) do { } while (FALSE)
#endif

int timer_q__walk_base_empty = 0;

static Timer_q_base *timer_q__start_base_norm = NULL;
static Timer_q_base *timer_q__start_base_empty = NULL;
static const struct timeval *timer_q__first_timeval = NULL;

static void timer_q__add_node(Timer_q_base *base, Timer_q_node *passed,
                              void *data, const struct timeval *tv,
                              int flags, int do_free)
{
 int has_prev = FALSE;
 int has_func = FALSE;
 Timer_q_node **scan = NULL;
 
 ++base->num;
 
 passed->tv = *tv;
 if (timer_q__first_timeval &&
     (TIMER_Q_TIMEVAL_CMP(timer_q__first_timeval, tv) > 0))
   timer_q__first_timeval = &passed->tv;
 
 if (flags & TIMER_Q_FLAG_NODE_DOUBLE)
   has_prev = TRUE;
 if (flags & TIMER_Q_FLAG_NODE_FUNC)
 {
  if (flags & TIMER_Q_FLAG_NODE_DOUBLE)
    ((Timer_q_double_func_node *)passed)->func = base->func;
  else
    ((Timer_q_func_node *)passed)->func = base->func;
  has_func = TRUE;
 }
 
 passed->has_prev = has_prev;
 passed->has_func = has_func;
 passed->do_free = do_free;
 passed->quick_del = FALSE;
 passed->data = data;

 if (!base->start)
 {
  assert(!base->end);

  if (base->move_when_empty)
    ++timer_q__walk_base_empty;
  
  base->end = base->start = passed;
  passed->next = NULL;
  if (passed->has_prev)
    ((Timer_q_double_node *) passed)->prev = NULL;
  return;
 }
 assert(base->end);

 TIMER_Q__CHECK(base);

 /* always check the end, makes life easier (and quicker) */
 if (TIMER_Q_TIMEVAL_CMP(&passed->tv, &base->end->tv) >= 0)
 {
  if (passed->has_prev)
    ((Timer_q_double_node *) passed)->prev = base->end;

  base->end->next = passed;
  base->end = passed;
  passed->next = NULL;
  return;
 }

 if (TIMER_Q_TIMEVAL_CMP(&passed->tv, &base->start->tv) < 0)
 {
  if (base->start->has_prev)
    ((Timer_q_double_node *) base->start)->prev = passed;
  if (passed->has_prev)
    ((Timer_q_double_node *) passed)->prev = NULL;

  passed->next = base->start;
  base->start = passed;
  
  return;
 }

 if (base->insert_from_end)
 {
  Timer_q_double_node *tmp = (Timer_q_double_node *)base->end;

  tmp = (Timer_q_double_node *)tmp->prev;

  assert(tmp);
  while (TIMER_Q_TIMEVAL_CMP(&tmp->s.tv, &passed->tv) > 0)
  {
   if (!tmp->s.has_prev)
     goto insert_from_beg;
   tmp = (Timer_q_double_node *)tmp->prev;
   assert(tmp);
  }

  scan = &tmp->s.next;
 }
 else
 {
 insert_from_beg:
  scan = &base->start->next;

  assert(*scan);
  while (TIMER_Q_TIMEVAL_CMP(&(*scan)->tv, &passed->tv) < 0)
  {
   scan = &(*scan)->next;
   assert(*scan);
  }
 }
 
 if ((passed->next = *scan) && passed->next->has_prev)
   ((Timer_q_double_node *) passed->next)->prev = passed;
 
 *scan = passed;
 
 if (passed->has_prev)
   ((Timer_q_double_node *) passed)->prev = TIMER_Q_CONV_PTR_NEXT_PREV(scan);

 TIMER_Q__CHECK(base);
 
 return;
}

Timer_q_node *timer_q_add_node(Timer_q_base *base,
                               void *data, const struct timeval *tv,
                               int flags)
{
 Timer_q_node *node = NULL;
 
 if (flags & (TIMER_Q_FLAG_NODE_SINGLE |
              TIMER_Q_FLAG_NODE_FUNC | TIMER_Q_FLAG_NODE_DOUBLE))
   /* nothing */ ;
 else
 {
  if (base->malloc_double)
    flags |= TIMER_Q_FLAG_NODE_DOUBLE;
  if (base->malloc_func)
    flags |= TIMER_Q_FLAG_NODE_FUNC;
 }
 
 if ((flags & (TIMER_Q_FLAG_NODE_DOUBLE | TIMER_Q_FLAG_NODE_FUNC)) == 
     (TIMER_Q_FLAG_NODE_DOUBLE | TIMER_Q_FLAG_NODE_FUNC))
   node = malloc(sizeof(Timer_q_double_func_node));
 else if ((flags & (TIMER_Q_FLAG_NODE_DOUBLE | TIMER_Q_FLAG_NODE_FUNC)) == 
          (TIMER_Q_FLAG_NODE_DOUBLE))
   node = malloc(sizeof(Timer_q_double_node));
 else if ((flags & (TIMER_Q_FLAG_NODE_DOUBLE | TIMER_Q_FLAG_NODE_FUNC)) == 
          (TIMER_Q_FLAG_NODE_FUNC))
   node = malloc(sizeof(Timer_q_func_node));
 else
   node = malloc(sizeof(Timer_q_node));
 
 if (!node)
   return (NULL);

 timer_q__add_node(base, node, data, tv, flags, TRUE);

 return (node);
}

Timer_q_node *timer_q_add_static_node(Timer_q_node *node, Timer_q_base *base,
                                      void *data, const struct timeval *tv,
                                      int flags)
{
 if (node && (flags & (TIMER_Q_FLAG_NODE_SINGLE |
                       TIMER_Q_FLAG_NODE_FUNC | TIMER_Q_FLAG_NODE_DOUBLE)))
 {
  timer_q__add_node(base, node, data, tv, flags, FALSE);
  return (node);
 }

 return (timer_q_add_node(base, data, tv, flags));
}

static Timer_q_node **timer_q__find_data(Timer_q_base *base, void *data)
{
 Timer_q_node **scan = &base->start;
 
 assert(data);

 while (*scan)
 {
  if (data == (*scan)->data)
    return (scan);
  
  scan = &(*scan)->next;
 }

 return (NULL);
}

Timer_q_node *timer_q_find_data(Timer_q_base *scan, void *data)
{
 Timer_q_node **ret = NULL;

 if ((ret = timer_q__find_data(scan, data)))
   return (*ret);
 
 return (NULL);
}

static void timer_q__del_node(Timer_q_base *base, Timer_q_node **current,
                              int just_unlink)
{
 Timer_q_node *tmp = *current;

 if (timer_q__first_timeval == &tmp->tv)
   timer_q__first_timeval = NULL;
  
 if (base->start == *current)
 {
  if (base->end == *current)
  {
   assert(!(*current)->next);
   
   base->end = NULL;
  }
  else if ((*current)->next && (*current)->next->has_prev)
    ((Timer_q_double_node *) (*current)->next)->prev = NULL;
 }
 else
 {
  Timer_q_node *prev = NULL;
  
  prev = TIMER_Q_CONV_PTR_NEXT_PREV(current);

  if ((*current)->next && (*current)->next->has_prev)
  {
   assert(!(base->end == *current));

   ((Timer_q_double_node *) (*current)->next)->prev = prev;
  }
  else if (base->end == *current)
  {
   assert(!(*current)->next);

   base->end = prev;
  }
 }
 
 *current = (*current)->next;

 --base->num;
   
 if (!just_unlink)
 {
  (*base->func)(TIMER_Q_TYPE_CALL_DEL, tmp->data);
  
  if (tmp->do_free)
    free(tmp);
 }

 TIMER_Q__CHECK(base);
}

int timer_q_del_data(Timer_q_base *base, void *data)
{
 Timer_q_node **ret = timer_q__find_data(base, data);

 if (ret)
   timer_q__del_node(base, ret, FALSE);

 return (ret != NULL);
}

void timer_q_del_node(Timer_q_base *base, Timer_q_node *current)
{
 if (base->start == current)
   timer_q__del_node(base, &base->start, FALSE);
 else if (current->has_prev)
 { /* *start would == current otherwise */
  assert(((Timer_q_double_node *) current)->prev);
  timer_q__del_node(base,
                    &((Timer_q_double_node *) current)->prev->next, FALSE);
 }
 else
 {
  Timer_q_node **scan = &base->start;

  while (*scan != current)
    scan = &(*scan)->next;
  
  timer_q__del_node(base, scan, FALSE);
 }
}

void timer_q_quick_del_node(Timer_q_node *current)
{
 current->quick_del = TRUE;
}

static void timer_q__add_base(Timer_q_base *passed,
                              void (*func)(int, void *), int flags)
{
 passed->func = func;
 
 passed->num = 0;
 passed->start = NULL;
 passed->end = NULL;
 passed->quick_del = FALSE;
 
 if (flags & TIMER_Q_FLAG_BASE_RUN_ALL)
   passed->run_all = TRUE;
 else
   passed->run_all = FALSE;

 if (flags & TIMER_Q_FLAG_BASE_MALLOC_DOUBLE)
   passed->malloc_double = TRUE;
 else
   passed->malloc_double = FALSE;

 if (flags & TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY)
 {
  passed->next = timer_q__start_base_norm;
  timer_q__start_base_norm = passed;
  
  passed->move_when_empty = TRUE;
 }
 else
 {
  passed->next = timer_q__start_base_empty;
  timer_q__start_base_empty = passed;
  
  passed->move_when_empty = FALSE;
 }
}

Timer_q_base *timer_q_add_base(void (*func)(int, void *), int flags)
{
 Timer_q_base *base = malloc(sizeof(Timer_q_base));
 
 if (!base)
   return (NULL);

 timer_q__add_base(base, func, flags);

 return (base);
}

Timer_q_base *timer_q_add_static_base(Timer_q_base *base,
                                      void (*func)(int, void *), int flags)
{
 assert(base);

 timer_q__add_base(base, func, flags);

 return (base);
}

static void timer_q__del_base(Timer_q_base **base)
{
 Timer_q_base *tmp = *base;
 
 *base = (*base)->next;

 while (tmp->start)
   timer_q__del_node(tmp, &tmp->start, FALSE);

 if (tmp->do_free)
   free(tmp);
 else
   tmp->quick_del = FALSE;
}

void timer_q_del_base(Timer_q_base *base)
{
 Timer_q_base **scan = NULL;

 if (timer_q__walk_base_empty || (base->move_when_empty && !base->start))
 {
  scan = &timer_q__start_base_empty;
  while (*scan && (*scan != base))
    scan = &(*scan)->next;

  if (*scan)
  {
   timer_q__del_base(scan);
   return;
  }
 }

 scan = &timer_q__start_base_norm;
 while (*scan != base)
   scan = &(*scan)->next;
 
 timer_q__del_base(scan);
}

void timer_q_quick_del_base(Timer_q_base *base)
{
 base->quick_del = TRUE;
}

static unsigned int timer_q__run_norm(Timer_q_base *base,
                                      const struct timeval *tv)
{
 Timer_q_node *scan = base->start;
 int called = 0;

 TIMER_Q__CHECK(base);
 
 while (scan && (scan->quick_del || (TIMER_Q_TIMEVAL_CMP(&scan->tv, tv) <= 0)))
 {
  assert(!scan->next || !scan->next->has_prev ||
         (((Timer_q_double_node *) scan->next)->prev == scan));
  assert(base->start == scan);
  
  if (scan->quick_del)
    timer_q__del_node(base, &base->start, FALSE);
  else
  {
   void (*func)(int, void *);
   
   timer_q__del_node(base, &base->start, TRUE);
  
   ++called;
   
   if (scan->has_func)
     if (scan->has_prev)
       func = ((Timer_q_double_func_node *)scan)->func;
     else
       func = ((Timer_q_func_node *)scan)->func;
   else
     func = base->func;

   (*func)(TIMER_Q_TYPE_CALL_RUN_NORM, scan->data);

   if (scan->do_free)
     free(scan);
  }

  scan = base->start;
 }

 TIMER_Q__CHECK(base);
 
 if (scan && (!timer_q__first_timeval ||
              (TIMER_Q_TIMEVAL_CMP(timer_q__first_timeval, &scan->tv) > 0)))
   timer_q__first_timeval = &scan->tv;

 return (called);
}

static unsigned int timer_q__run_all(Timer_q_base *base)
{ /* ignore timestamps */
 Timer_q_node *scan = base->start;
 int called = 0;

 TIMER_Q__CHECK(base);
 
 while (scan)
 {
  Timer_q_node *scan_next = scan->next;
  
  assert(!scan_next || !scan_next->has_prev ||
         (((Timer_q_double_node *) scan_next)->prev == scan));
  
  if (!scan->quick_del)
  {
   void (*func)(int, void *);
   
   ++called;
   
   if (scan->has_func)
     if (scan->has_prev)
       func = ((Timer_q_double_func_node *)scan)->func;
     else
       func = ((Timer_q_func_node *)scan)->func;
   else
     func = base->func;
   
   (*func)(TIMER_Q_TYPE_CALL_RUN_ALL, scan->data);
  }
  
  scan = scan_next;
 }

 return (called);
}

static void timer_q__get_empties(void)
{
 Timer_q_base **scan = &timer_q__start_base_empty;
 
 /* It's possible that timer_q__walk_base_empty > number of elements,
  * Eg. call SET_NO_MOVE_WHEN_EMPTY with 0, then 1 repeatedly */
 while (timer_q__walk_base_empty && *scan)
 {
  if ((*scan)->start)
  {
   Timer_q_base *tmp = *scan;

   *scan = (*scan)->next;

   tmp->next = timer_q__start_base_norm;
   timer_q__start_base_norm = tmp;

   --timer_q__walk_base_empty;
  }
  else  
    scan = &(*scan)->next;
 }
 timer_q__walk_base_empty = 0;
}

unsigned int timer_q_run_norm(const struct timeval *tv)
{
 Timer_q_base **scan = &timer_q__start_base_norm;
 int num = 0;

 if (timer_q__first_timeval &&
     (TIMER_Q_TIMEVAL_CMP(timer_q__first_timeval, tv) > 0))
   return (0);
 
 timer_q__get_empties();
 
 timer_q__first_timeval = NULL;

 scan = &timer_q__start_base_norm;
 while (*scan)
 {
  if (!(*scan)->quick_del)
    num += timer_q__run_norm(*scan, tv);

  if ((*scan)->quick_del)
  {
   timer_q__del_base(scan);
  }
  else if (!(*scan)->start && (*scan)->move_when_empty)
  {
   Timer_q_base *tmp = *scan;

   *scan = (*scan)->next;

   tmp->next = timer_q__start_base_empty;
   timer_q__start_base_empty = tmp;
  }
  else  
    scan = &(*scan)->next;
 }

 return (num);
}

unsigned int timer_q_run_all(void)
{
 Timer_q_base *scan = timer_q__start_base_norm;
 int num = 0;

 timer_q__get_empties();
 
 while (scan)
 {
  if (scan->run_all && !scan->quick_del)
    num += timer_q__run_all(scan);

  scan = scan->next;
 }

 return (num);
}

const struct timeval *timer_q_first_timeval(void)
{
 struct timeval dummy = {0, 0};
 
 if (!timer_q__first_timeval)
   timer_q_run_norm(&dummy);
 
 return (timer_q__first_timeval);
}

