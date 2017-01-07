#ifndef SOCKET_POLL__HEADER_H
# error " You must _just_ #include <socket_poll.h>"
#endif

/*
 *  Copyright (C) 2000  James Antill
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

#ifndef SOCKET_POLL_COMPILE_TYPEDEF
# define SOCKET_POLL_COMPILE_TYPEDEF 1
#endif


#if SOCKET_POLL_COMPILE_TYPEDEF
# define SOCKET_POLL__DECL_TYPEDEF1(x) typedef x
# define SOCKET_POLL__DECL_TYPEDEF2(x) x
#else
# define SOCKET_POLL__DECL_TYPEDEF1(x) x
# define SOCKET_POLL__DECL_TYPEDEF2(x) /* nothing */
#endif
