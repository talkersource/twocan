#define TIMER_Q_DIFF_C
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

long timer_q_timeval_diff_usecs(const struct timeval *tv1,
                                const struct timeval *tv2)
{
 unsigned long start_sec = 0;
 unsigned long start_usec = 0;
 unsigned long end_sec = 0;
 unsigned long end_usec = 0;
 long ret = 0;
 
 if (tv1->tv_sec == tv2->tv_sec)
   return (tv1->tv_usec - tv2->tv_usec);
 else if (tv1->tv_sec > tv2->tv_sec)
 {
  start_sec = tv2->tv_sec;
  end_sec = tv1->tv_sec;
  start_usec = tv2->tv_usec;
  end_usec = tv1->tv_usec;
 }
 else
 {
  start_sec = tv1->tv_sec;
  end_sec = tv2->tv_sec;
  start_usec = tv1->tv_usec;
  end_usec = tv2->tv_usec;
 }

 if ((end_sec - start_sec) > ((LONG_MAX / 1000000) - 1))
 {
  if (tv1->tv_sec > tv2->tv_sec)
    return (LONG_MAX);
  return (LONG_MIN);
 }
 
 if (end_usec < start_usec)
 {
  end_usec += 1000000;
  --end_sec;
 }
 
 ret = (((end_sec - start_sec) * 1000000) + (end_usec - start_usec));

 if (tv1->tv_sec > tv2->tv_sec)
   return (ret);
 return (-ret);
}

unsigned long timer_q_timeval_udiff_usecs(const struct timeval *tv1,
                                          const struct timeval *tv2)
{
 unsigned long start_sec = 0;
 unsigned long start_usec = 0;
 unsigned long end_sec = 0;
 unsigned long end_usec = 0;
 unsigned long ret = 0;
 
 if (tv1->tv_sec == tv2->tv_sec)
 {
  if (tv1->tv_usec < tv2->tv_usec)
    return (0);
  return (tv1->tv_usec - tv2->tv_usec);
 }
 else if (tv1->tv_sec < tv2->tv_sec)
   return (0);

 start_sec = tv2->tv_sec;
 end_sec = tv1->tv_sec;
 start_usec = tv2->tv_usec;
 end_usec = tv1->tv_usec;

 if ((end_sec - start_sec) > ((ULONG_MAX / 1000000) - 1))
   return (ULONG_MAX);
 
 if (end_usec < start_usec)
 {
  end_usec += 1000000;
  --end_sec;
 }
 
 ret = (((end_sec - start_sec) * 1000000) + (end_usec - start_usec));

 return (ret);
}

long timer_q_timeval_diff_msecs(const struct timeval *tv1,
                                const struct timeval *tv2)
{
 unsigned long start_sec = 0;
 unsigned long start_usec = 0;
 unsigned long end_sec = 0;
 unsigned long end_usec = 0;
 long ret = 0;
 
 if (tv1->tv_sec == tv2->tv_sec)
 {
  if (tv1->tv_usec == tv2->tv_usec)
    return (0);
  return ((tv1->tv_usec - tv2->tv_usec) / 1000);
 }
 else if (tv1->tv_sec > tv2->tv_sec)
 {
  start_sec = tv2->tv_sec;
  end_sec = tv1->tv_sec;
  start_usec = tv2->tv_usec;
  end_usec = tv1->tv_usec;
 }
 else
 {
  start_sec = tv1->tv_sec;
  end_sec = tv2->tv_sec;
  start_usec = tv1->tv_usec;
  end_usec = tv2->tv_usec;
 }
 
 if ((end_sec - start_sec) > ((LONG_MAX / 1000) - 1))
 {
  if (tv1->tv_sec > tv2->tv_sec)
    return (LONG_MAX);
  else
    return (LONG_MIN);
 }
 
 if (end_usec < start_usec)
 {
  end_usec += 1000000;
  --end_sec;
 }

 if (start_usec)
   start_usec /= 1000;
 if (end_usec)
   end_usec /= 1000;

 ret = (((end_sec - start_sec) * 1000) + (end_usec - start_usec));

 if (tv1->tv_sec > tv2->tv_sec)
   return (ret);
 else
   return (-ret);
}

unsigned long timer_q_timeval_udiff_msecs(const struct timeval *tv1,
                                          const struct timeval *tv2)
{
 unsigned long start_sec = 0;
 unsigned long start_usec = 0;
 unsigned long end_sec = 0;
 unsigned long end_usec = 0;
 unsigned long ret = 0;
 
 if (tv1->tv_sec == tv2->tv_sec)
 {
  if (tv1->tv_usec <= tv2->tv_usec)
    return (0);
  return ((tv1->tv_usec - tv2->tv_usec) / 1000);
 }
 else if (tv1->tv_sec < tv2->tv_sec)
   return (0);

 start_sec = tv2->tv_sec;
 end_sec = tv1->tv_sec;
 start_usec = tv2->tv_usec;
 end_usec = tv1->tv_usec;
 
 if ((end_sec - start_sec) > (ULONG_MAX / 1000) - 1)
   return (ULONG_MAX);
 
 if (end_usec < start_usec)
 {
  end_usec += 1000000;
  --end_sec;
 }

 if (start_usec)
   start_usec /= 1000;
 if (end_usec)
   end_usec /= 1000;

 ret = (((end_sec - start_sec) * 1000) + (end_usec - start_usec));

 return (ret);
}

long timer_q_timeval_diff_secs(const struct timeval *tv1,
                               const struct timeval *tv2)
{
 unsigned long start_sec = 0;
 unsigned long start_usec = 0;
 unsigned long end_sec = 0;
 unsigned long end_usec = 0;
 long ret = 0;
 
 if (tv1->tv_sec == tv2->tv_sec)
   return (0);
 else if (tv1->tv_sec > tv2->tv_sec)
 {
  start_sec = tv2->tv_sec;
  end_sec = tv1->tv_sec;
  start_usec = tv2->tv_usec;
  end_usec = tv1->tv_usec;
 }
 else
 {
  start_sec = tv1->tv_sec;
  end_sec = tv2->tv_sec;
  start_usec = tv1->tv_usec;
  end_usec = tv2->tv_usec;
 }

 if ((end_sec - start_sec) > (LONG_MAX - 1))
 {
  if (tv1->tv_sec > tv2->tv_sec)
    return (LONG_MAX);
  else
    return (LONG_MIN);
 }
 
 if (end_usec < start_usec)
 {
  end_usec += 1000000;
  --end_sec;
 }
 
 if (start_usec)
   start_usec /= 1000000;
 if (end_usec)
   end_usec /= 1000000;

 ret = ((end_sec - start_sec) + (end_usec - start_usec));

 if (tv1->tv_sec > tv2->tv_sec)
   return (ret);
 else
   return (-ret);
}

unsigned long timer_q_timeval_udiff_secs(const struct timeval *tv1,
                                         const struct timeval *tv2)
{
 unsigned long start_sec = 0;
 unsigned long start_usec = 0;
 unsigned long end_sec = 0;
 unsigned long end_usec = 0;
 unsigned long ret = 0;
 
 if (tv1->tv_sec <= tv2->tv_sec)
   return (0);
 
 start_sec = tv2->tv_sec;
 end_sec = tv1->tv_sec;
 start_usec = tv2->tv_usec;
 end_usec = tv1->tv_usec;
 
 if (end_usec < start_usec)
 {
  end_usec += 1000000;
  --end_sec;
 }
 
 if (start_usec)
   start_usec /= 1000000;
 if (end_usec)
   end_usec /= 1000000;

 ret = ((end_sec - start_sec) + (end_usec - start_usec));

 return (ret);
}
