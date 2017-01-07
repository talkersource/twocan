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

#define SOCKET_POLL_CNTL_SET_MAP_DIRECT_OFFSET 1
#define SOCKET_POLL_CNTL_GET_MAP_DIRECT_OFFSET 2
#define SOCKET_POLL_CNTL_SET_MAP_COMPRESSED_GROW 3
#define SOCKET_POLL_CNTL_GET_MAP_COMPRESSED_GROW 4
    
typedef unsigned int Socket_poll_typedef_offset;

SOCKET_POLL__DECL_TYPEDEF1(struct Socket_poll_cntl_ret_int)
{
 int ret;
} SOCKET_POLL__DECL_TYPEDEF2(Socket_poll_cntl_ret_int);

SOCKET_POLL__DECL_TYPEDEF1(struct Socket_poll_cntl_ret_size_t)
{
 size_t ret;
} SOCKET_POLL__DECL_TYPEDEF2(Socket_poll_cntl_ret_size_t);

SOCKET_POLL__DECL_TYPEDEF1(struct Socket_poll_cntl_ret_void_ptr)
{
 void *ret;
} SOCKET_POLL__DECL_TYPEDEF2(Socket_poll_cntl_ret_void_ptr);

SOCKET_POLL__DECL_TYPEDEF1(struct Socket_poll_cntl_ret_func_ptr)
{
 void (*ret)(void);
} SOCKET_POLL__DECL_TYPEDEF2(Socket_poll_cntl_ret_func_ptr);

