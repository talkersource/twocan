#define TIMER_Q_CNTL_C
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

int timer_q_cntl_opt(int option, ...)
{
 int ret = FALSE;
 
 va_list ap;

 va_start(ap, option);
 
 switch (option)
 {  
  case TIMER_Q_CNTL_OPT_GET_:
  {
  }
  break;

  case TIMER_Q_CNTL_OPT_SET_:
  {
  }
  break;
    
  default:
    break;
 }

 va_end(ap);
 
 return (ret);
}

typedef void (*Timer_q__typedef_func)(int, void *);

int timer_q_cntl_node(Timer_q_node *node, int option, ...)
{
 int ret = FALSE;
 
 va_list ap;

 va_start(ap, option);
 
 switch (option)
 {
  case TIMER_Q_CNTL_NODE_GET_DATA:
  {
   void **val = va_arg(ap, void **);
   
   *val = node->data;
   ret = TRUE;
  }
  break;

  case TIMER_Q_CNTL_NODE_SET_DATA:
  {
   void *val = va_arg(ap, void *);
   node->data = val;
   ret = TRUE;
  }
  break;
  
  case TIMER_Q_CNTL_NODE_GET_FUNC:
  {
   Timer_q_cntl_ret_func_ptr *val = va_arg(ap, Timer_q_cntl_ret_func_ptr *);
   
   if (!node->has_func)
     break;
   
   if (node->has_prev)
     val->ret = ((Timer_q_double_func_node *) node)->func;
   else
     val->ret = ((Timer_q_func_node *) node)->func;

   ret = TRUE;
  }
  break;

  case TIMER_Q_CNTL_NODE_SET_FUNC:
  {
   void (*val)(int, void *) = va_arg(ap, Timer_q__typedef_func);

   if (!node->has_func)
     break;

   if (node->has_prev)
     ((Timer_q_double_func_node *) node)->func = val;
   else
     ((Timer_q_func_node *) node)->func = val;
   
   ret = TRUE;
  }
  break;

  case TIMER_Q_CNTL_NODE_GET_TIMEVAL:
  {
   const struct timeval **val = va_arg(ap, const struct timeval **);
   
   *val = &node->tv;
   ret = TRUE;
  }
  break;

  case TIMER_Q_CNTL_NODE_SET_TIMEVAL:
  {
   Timer_q_base *base = va_arg(ap, Timer_q_base *);
   struct timeval *val = va_arg(ap, struct timeval *);

   assert(base && val);

   /* node->tv = *val; -- needs to relink in not implemented atm. */
  }
  break;
  
  default:
    break;
 }

 va_end(ap);
 
 return (ret);
}

int timer_q_cntl_base(Timer_q_base *base, int option, ...)
{
 int ret = FALSE;
 
 va_list ap;

 va_start(ap, option);
 
 switch (option)
 {
  case TIMER_Q_CNTL_BASE_GET_FUNC:
  {
   Timer_q_cntl_ret_func_ptr *val = va_arg(ap, Timer_q_cntl_ret_func_ptr *);
   
   val->ret = base->func;
   ret = TRUE;
  }
  break;
    
  case TIMER_Q_CNTL_BASE_SET_FUNC:
  {
   void (*val)(int, void *) = va_arg(ap, Timer_q__typedef_func);
   
   base->func = val;
   ret = TRUE;
  }
  break;
    
  case TIMER_Q_CNTL_BASE_GET_FLAG_RUN_ALL:
  {
   int *val = va_arg(ap, int *);
   
   *val = base->run_all;
   ret = TRUE;
  }
  break;
    
  case TIMER_Q_CNTL_BASE_SET_FLAG_RUN_ALL:
  {
   int val = va_arg(ap, int);
   
   base->run_all = val;
   ret = TRUE;
  }
  break;
  
  case TIMER_Q_CNTL_BASE_GET_FLAG_INSERT_FROM_END:
  {
   int *val = va_arg(ap, int *);
   
   *val = base->insert_from_end;
   ret = TRUE;
  }
  break;
    
  case TIMER_Q_CNTL_BASE_SET_FLAG_INSERT_FROM_END:
  {
   int val = va_arg(ap, int);
   
   base->insert_from_end = val;
   ret = TRUE;
  }
  break;
  
  case TIMER_Q_CNTL_BASE_GET_FLAG_MALLOC_DOUBLE:
  {
   int *val = va_arg(ap, int *);
   
   *val = base->malloc_double;
   ret = TRUE;
  }
  break;
    
  case TIMER_Q_CNTL_BASE_SET_FLAG_MALLOC_DOUBLE:
  {
   int val = va_arg(ap, int);
   
   base->malloc_double = val;
   ret = TRUE;
  }
  break;
  
  case TIMER_Q_CNTL_BASE_GET_FLAG_MALLOC_FUNC:
  {
   int *val = va_arg(ap, int *);
   
   *val = base->malloc_func;
   ret = TRUE;
  }
  break;
  
  case TIMER_Q_CNTL_BASE_SET_FLAG_MALLOC_FUNC:
  {
   int val = va_arg(ap, int);
   
   base->malloc_func = val;
   ret = TRUE;
  }
  break;  

  case TIMER_Q_CNTL_BASE_GET_FLAG_MOVE_WHEN_EMPTY:
  {
   int *val = va_arg(ap, int *);
   
   *val = base->move_when_empty;
   ret = TRUE;
  }
  break;
  
  case TIMER_Q_CNTL_BASE_SET_FLAG_MOVE_WHEN_EMPTY:
  {
   int val = va_arg(ap, int);
   
   if (!base->start && !val && base->move_when_empty)
     ++timer_q__walk_base_empty;

   base->move_when_empty = val;
   ret = TRUE;
  }
  break;  

  default:
    break;
 }

 va_end(ap);
 
 return (ret);
}

