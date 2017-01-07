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

/* can't get around this one typedef */
#ifdef TIMER_Q_AUTOCONF_HAVE_CHAR_BITFLAG
typedef unsigned char Timer_q_typedef_bitflag;
#else
/* ISO C says that only int is a supported bitflag type...
 * Might be quicker doing this anyway,
 * although it will take up to much more room */
typedef unsigned int Timer_q_typedef_bitflag;
#endif

#ifndef TIMER_Q_COMPILE_TYPEDEF
# define TIMER_Q_COMPILE_TYPEDEF 1
#endif


#if TIMER_Q_COMPILE_TYPEDEF
# define TIMER_Q__DECL_TYPEDEF1(x) typedef x
# define TIMER_Q__DECL_TYPEDEF2(x) x
#else
# define TIMER_Q__DECL_TYPEDEF1(x) x
# define TIMER_Q__DECL_TYPEDEF2(x) /* nothing */
#endif
