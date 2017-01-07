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

#define TIMER_Q_TYPE_CALL_RUN_NORM 1
#define TIMER_Q_TYPE_CALL_DEL 2
#define TIMER_Q_TYPE_CALL_RUN_ALL 3


#define TIMER_Q_FLAG_BASE_RUN_ALL (1<<0)
#define TIMER_Q_FLAG_BASE_INSERT_FROM_END (1<<1)
#define TIMER_Q_FLAG_BASE_MALLOC_DOUBLE (1<<2)
#define TIMER_Q_FLAG_BASE_MALLOC_FUNC (1<<3)
#define TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY (1<<4)

#define TIMER_Q_FLAG_BASE_DEFAULT (TIMER_Q_FLAG_BASE_INSERT_FROM_END | \
                                   TIMER_Q_FLAG_BASE_MOVE_WHEN_EMPTY | \
                                   0)

#define TIMER_Q_FLAG_NODE_SINGLE (1<<0)
#define TIMER_Q_FLAG_NODE_DOUBLE (1<<1)
#define TIMER_Q_FLAG_NODE_FUNC (1<<2)

#define TIMER_Q_FLAG_NODE_DEFAULT 0


#define TIMER_Q__CNTL(x, y) ((TIMER_Q__CNTL_ ## x ## _OFFSET) + (y))

#define TIMER_Q__CNTL_OPT_OFFSET 0
#define TIMER_Q__CNTL_NODE_OFFSET 1000
#define TIMER_Q__CNTL_BASE_OFFSET 2000

#define TIMER_Q_CNTL_OPT_GET_ TIMER_Q__CNTL(OPT, 1)
#define TIMER_Q_CNTL_OPT_SET_ TIMER_Q__CNTL(OPT, 2)

#define TIMER_Q_CNTL_NODE_GET_DATA TIMER_Q__CNTL(NODE, 1)
#define TIMER_Q_CNTL_NODE_SET_DATA TIMER_Q__CNTL(NODE, 2)
#define TIMER_Q_CNTL_NODE_GET_FUNC TIMER_Q__CNTL(NODE, 3)
#define TIMER_Q_CNTL_NODE_SET_FUNC TIMER_Q__CNTL(NODE, 4)
#define TIMER_Q_CNTL_NODE_GET_TIMEVAL TIMER_Q__CNTL(NODE, 5)
#define TIMER_Q_CNTL_NODE_SET_TIMEVAL TIMER_Q__CNTL(NODE, 6)

#define TIMER_Q_CNTL_BASE_GET_FUNC TIMER_Q__CNTL(BASE, 1)
#define TIMER_Q_CNTL_BASE_SET_FUNC TIMER_Q__CNTL(BASE, 2)
#define TIMER_Q_CNTL_BASE_GET_FLAG_RUN_ALL TIMER_Q__CNTL(BASE, 3)
#define TIMER_Q_CNTL_BASE_SET_FLAG_RUN_ALL TIMER_Q__CNTL(BASE, 4)
#define TIMER_Q_CNTL_BASE_GET_FLAG_INSERT_FROM_END TIMER_Q__CNTL(BASE, 5)
#define TIMER_Q_CNTL_BASE_SET_FLAG_INSERT_FROM_END TIMER_Q__CNTL(BASE, 6)
#define TIMER_Q_CNTL_BASE_GET_FLAG_MALLOC_DOUBLE TIMER_Q__CNTL(BASE, 7)
#define TIMER_Q_CNTL_BASE_SET_FLAG_MALLOC_DOUBLE TIMER_Q__CNTL(BASE, 8)
#define TIMER_Q_CNTL_BASE_GET_FLAG_MALLOC_FUNC TIMER_Q__CNTL(BASE, 9)
#define TIMER_Q_CNTL_BASE_SET_FLAG_MALLOC_FUNC TIMER_Q__CNTL(BASE, 10)
#define TIMER_Q_CNTL_BASE_GET_FLAG_MOVE_WHEN_EMPTY TIMER_Q__CNTL(BASE, 11)
#define TIMER_Q_CNTL_BASE_SET_FLAG_MOVE_WHEN_EMPTY TIMER_Q__CNTL(BASE, 12)
#define TIMER_Q_CNTL_BASE_GET_FLAG_HAVE_DELETED TIMER_Q__CNTL(BASE, 13)
#define TIMER_Q_CNTL_BASE_SET_FLAG_HAVE_DELETED TIMER_Q__CNTL(BASE, 14)

