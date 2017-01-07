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

#define SOCKET_POLL_TYPE_MAP_COMPRESSED 1
#define SOCKET_POLL_TYPE_MAP_DIRECT 2

#define SOCKET_POLL__CNTL(x, y) ((SOCKET_POLL__CNTL_ ## x ## _OFFSET) + (y))

#define SOCKET_POLL__CNTL_OPT_OFFSET 0

#define SOCKET_POLL_CNTL_OPT_GET_MAP_TYPE SOCKET_POLL__CNTL(OPT, 1)
#define SOCKET_POLL_CNTL_OPT_SET_MAP_TYPE SOCKET_POLL__CNTL(OPT, 2)
#define SOCKET_POLL_CNTL_OPT_GET_MAP_COMPRESSED_GROW SOCKET_POLL__CNTL(OPT, 3)
#define SOCKET_POLL_CNTL_OPT_SET_MAP_COMPRESSED_GROW SOCKET_POLL__CNTL(OPT, 4)
#define SOCKET_POLL_CNTL_OPT_GET_MAP_SIZE SOCKET_POLL__CNTL(OPT, 5)
#define SOCKET_POLL_CNTL_OPT_SET_MAP_SIZE SOCKET_POLL__CNTL(OPT, 6)
#define SOCKET_POLL_CNTL_OPT_GET_MAP_END_SIZE SOCKET_POLL__CNTL(OPT, 7)
#define SOCKET_POLL_CNTL_OPT_SET_MAP_END_SIZE SOCKET_POLL__CNTL(OPT, 8)
#define SOCKET_POLL_CNTL_OPT_GET_MAP_END_ARRAY SOCKET_POLL__CNTL(OPT, 9)
#define SOCKET_POLL_CNTL_OPT_SET_MAP_END_ARRAY SOCKET_POLL__CNTL(OPT, 10)

