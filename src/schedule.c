#define SCHEDULE_C
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


void schedule_timer_start(player *p, struct timeval *storage)
{
 IGNORE_PARAMETER(p);
 
 if (gettimeofday(storage, NULL))
 {
  assert(FALSE);
 }
}

void schedule_timer_end(player *p, struct timeval *storage)
{
 unsigned long tmp = 0;
 
 if (gettimeofday(&now_timeval, NULL))
 {
  assert(FALSE);
 }
 if (!p->schedule_time)
   p->schedule_can_go = now_timeval;
 
 tmp = timeval_diff_time(&now_timeval, storage);

 DEBUG_START(SCHED_DEBUG);
 fprintf(stderr, "%lu", tmp);
 DEBUG_END();

 p->schedule_time += tmp;
 
 timeval_add_useconds(&p->total_cpu, tmp);
 timeval_add_useconds(&p->this_cpu, tmp);
}

int schedule_can_go(player *p)
{
 unsigned long sched_pass_limit = SCHEDULE_NEWBIE_PASS;
 unsigned long tmp = 0;

 if (!p->schedule_time)
   return (TRUE);

 if (!p->is_fully_on)
   return (TRUE);
 
 if (p->saved && p->saved->priv_admin)
   sched_pass_limit = SCHEDULE_ADMIN_PASS;
 else if (p->saved && p->saved->priv_pretend_su)
   sched_pass_limit = SCHEDULE_STAFF_PASS;
 else if (p->saved && p->saved->priv_spod)
   sched_pass_limit = SCHEDULE_SPOD_PASS;
 else if (p->saved && p->saved->priv_base)
 {
  if (difftime(now, p->last_command_timestamp) < 1)
    return (FALSE);
  sched_pass_limit = SCHEDULE_RESIDENT_PASS;
 }
 /* do we want this to be lower ?*/
 else if (difftime(now, p->last_command_timestamp) < 2)
   return (FALSE);
 
 if (gettimeofday(&now_timeval, NULL))
 {
  assert(FALSE);
 }
 
 tmp = timeval_diff_time(&now_timeval, &p->schedule_can_go);
 if (tmp > 1000000) /* one second */
 {
  p->schedule_can_go.tv_sec = now_timeval.tv_sec;
  p->schedule_can_go.tv_usec = now_timeval.tv_usec;

  sched_pass_limit /= 4;
  
  while (((tmp -= 1000000) > 0) && (sched_pass_limit < SCHEDULE_MAX_AT_ONCE) &&
         (sched_pass_limit < p->schedule_time))
  {
   sched_pass_limit += SCHEDULE_ABOVE_ONE_INC;
  }
  
  if (sched_pass_limit >= p->schedule_time) 
    p->schedule_time = 0;
  else
    p->schedule_time -= sched_pass_limit;
 }

 if (p->schedule_time < sched_pass_limit)
   return (TRUE);

 DEBUG_START(SCHED_DEBUG && p->saved);
 fprintf(stderr, "%s %lu %lu %lu",
         p->saved->name, p->schedule_time, sched_pass_limit, tmp);
 DEBUG_END();

 return (FALSE);
}

