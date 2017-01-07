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

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_node)
{
 struct Timer_q_node *next; /* private */
 
 void *data; /* private */

 struct timeval tv; /* private */

 Timer_q_typedef_bitflag do_free : 1; /* private */
 Timer_q_typedef_bitflag has_prev : 1; /* private */
 Timer_q_typedef_bitflag has_func : 1; /* private */
 Timer_q_typedef_bitflag quick_del : 1; /* private */
} TIMER_Q__DECL_TYPEDEF2(Timer_q_node);

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_double_node)
{
 struct Timer_q_node s; /* public/read */
 struct Timer_q_node *prev; /* private */
} TIMER_Q__DECL_TYPEDEF2(Timer_q_double_node);

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_func_node)
{
 struct Timer_q_node s; /* public/read */
 void (*func)(int, void *); /* private */
} TIMER_Q__DECL_TYPEDEF2(Timer_q_func_node);

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_double_func_node)
{
 struct Timer_q_double_node s; /* public/read */
 void (*func)(int, void *); /* private */
} TIMER_Q__DECL_TYPEDEF2(Timer_q_double_func_node);

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_base)
{
 struct Timer_q_base *next; /* private */

 struct Timer_q_node *start; /* private */
 struct Timer_q_node *end; /* private */

 void (*func)(int, void *); /* private */
 
 unsigned int num; /* public/read */
 
 Timer_q_typedef_bitflag do_free : 1; /* private */
 Timer_q_typedef_bitflag run_all : 1; /* private */
 Timer_q_typedef_bitflag insert_from_end : 1; /* private */
 Timer_q_typedef_bitflag malloc_double : 1; /* private */
 Timer_q_typedef_bitflag malloc_func : 1; /* private */
 Timer_q_typedef_bitflag move_when_empty : 1; /* private */
 Timer_q_typedef_bitflag quick_del : 1; /* private */
} TIMER_Q__DECL_TYPEDEF2(Timer_q_base);


TIMER_Q__DECL_TYPEDEF1(struct Timer_q_cntl_ret_int)
{
 int ret;
} TIMER_Q__DECL_TYPEDEF2(Timer_q_cntl_ret_int);

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_cntl_ret_size_t)
{
 size_t ret;
} TIMER_Q__DECL_TYPEDEF2(Timer_q_cntl_ret_size_t);

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_cntl_ret_void_ptr)
{
 void *ret;
} TIMER_Q__DECL_TYPEDEF2(Timer_q_cntl_ret_void_ptr);

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_cntl_ret_func_ptr)
{
 void (*ret)(int, void *);
} TIMER_Q__DECL_TYPEDEF2(Timer_q_cntl_ret_func_ptr);

TIMER_Q__DECL_TYPEDEF1(struct Timer_q_cntl_ret_timeval_ptr)
{
 const struct timeval *val;
} TIMER_Q__DECL_TYPEDEF2(Timer_q_cntl_ret_timeval_ptr);
