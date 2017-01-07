#ifndef TIMER_Q__HEADER_H
# error " You must _just_ #include <timer_q.h>"
#endif

/*
 *  Copyright (C) 1999, 2000  James Antill
 *  
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *   
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *  email: james@and.org
 */

#define TIMER_Q_TIMEVAL_CMP(x, y) (((x)->tv_sec == (y)->tv_sec) ? \
 ((x)->tv_usec - (y)->tv_usec) : ((x)->tv_sec - (y)->tv_sec))

#define TIMER_Q_TIMEVAL_NORM(x) do { \
 if ((x)->tv_usec > 1000000) { \
   (x)->tv_sec += ((x)->tv_usec / 1000000); \
   (x)->tv_usec %= 1000000; \
  } \
 } while (FALSE) 
    
#define TIMER_Q_TIMEVAL_ADD_SECS(x, y, z) do { \
 (x)->tv_sec += (y); \
 (x)->tv_usec += (z); \
 TIMER_Q_TIMEVAL_NORM(x); \
 } while (FALSE)
#define TIMER_Q_TIMEVAL_ADD_MINS(x, y, z) do { \
 (x)->tv_sec += ((y) * 60); \
 (x)->tv_usec += (z); \
 TIMER_Q_TIMEVAL_NORM(x); \
 } while (FALSE)
#define TIMER_Q_TIMEVAL_ADD_HOURS(x, y, z) do { \
 (x)->tv_sec += ((y) * 60 * 60); \
 (x)->tv_usec += (z); \
 TIMER_Q_TIMEVAL_NORM(x); \
 } while (FALSE)
#define TIMER_Q_TIMEVAL_ADD_DAYS(x, y, z) do { \
 (x)->tv_sec += ((y) * 60 * 60 * 24); \
 (x)->tv_usec += (z); \
 TIMER_Q_TIMEVAL_NORM(x); \
 } while (FALSE)

extern struct Timer_q_base *timer_q_add_base(void (*)(int, void *),
                                             int);
extern struct Timer_q_base *timer_q_add_static_base(struct Timer_q_base *,
                                                    void (*)(int, void *),
                                                    int);
extern void timer_q_del_base(struct Timer_q_base *);
extern void timer_q_quick_del_base(struct Timer_q_base *);
    
extern struct Timer_q_node *timer_q_add_node(struct Timer_q_base *,
                                             void *,
                                             const struct timeval *,
                                             int);
extern struct Timer_q_node *timer_q_add_static_node(struct Timer_q_node *,
                                                    struct Timer_q_base *,
                                                    void *,
                                                    const struct timeval *,
                                                    int);
extern void timer_q_del_node(struct Timer_q_base *, struct Timer_q_node *);
extern void timer_q_quick_del_node(struct Timer_q_node *);
extern int timer_q_del_data(struct Timer_q_base *, void *);

extern struct Timer_q_node *timer_q_find_data(struct Timer_q_base *, void *);

extern unsigned int timer_q_run_norm(const struct timeval *);
extern unsigned int timer_q_run_all(void);

extern const struct timeval *timer_q_first_timeval(void);

extern long timer_q_timeval_diff_usecs(const struct timeval *,
                                       const struct timeval *);
extern unsigned long timer_q_timeval_udiff_usecs(const struct timeval *,
                                                 const struct timeval *);
extern long timer_q_timeval_diff_msecs(const struct timeval *,
                                       const struct timeval *);
extern unsigned long timer_q_timeval_udiff_msecs(const struct timeval *,
                                                 const struct timeval *);
extern long timer_q_timeval_diff_secs(const struct timeval *,
                                      const struct timeval *);
extern unsigned long timer_q_timeval_udiff_secs(const struct timeval *,
                                                const struct timeval *);

extern int timer_q_cntl_opt(int, ...);

extern int timer_q_cntl_node(struct Timer_q_node *, int, ...);
extern int timer_q_cntl_base(struct Timer_q_base *, int, ...);
