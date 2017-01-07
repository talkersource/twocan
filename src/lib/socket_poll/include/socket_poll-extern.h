#ifndef SOCKET_POLL__HEADER_H
# error " You must _just_ #include <socket_poll.h>"
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

extern struct pollfd *socket_poll_indicators; /* public/read */

#define SOCKET_POLL_INDICATOR(offset) (socket_poll_indicators + offset - 1)

extern int socket_poll_cntl_opt(int, ...);
    
extern int socket_poll_init(size_t);

extern Socket_poll_typedef_offset socket_poll_add(int);
extern void socket_poll_del(Socket_poll_typedef_offset);

extern int socket_poll_update_all(int);
extern int socket_poll_update_one(Socket_poll_typedef_offset, int);

