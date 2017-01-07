#define SOCKET_POLL_C
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
#include "main.h"

struct pollfd *socket_poll_indicators = NULL;

/* internal */
static size_t socket_poll_grow_sz = 2;

static Socket_poll_typedef_offset socket_poll_offset_num = 0;
static Socket_poll_typedef_offset socket_poll_offset_max = 1;
static Socket_poll_typedef_offset socket_poll_offset_min = 1;
static Socket_poll_typedef_offset socket_poll_offset_sz = 0;

static unsigned char *socket_poll_free_arr = NULL;

#if defined(HAVE_CHAR_BITFLAG)
typedef unsigned char bitflag;
#else
/* ANSI says that only int is a supported bitflag type...
 * Might be quicker doing this anyway,
 * although it might take up to much more room */
typedef unsigned int bitflag;
#endif

struct socket_poll_internal_options
{
 int map_type;
};

static struct socket_poll_internal_options socket_poll_internal_options =
{
 SOCKET_POLL_TYPE_MAP_COMPRESSED
};

static int socket_poll_internal_change_map(int val)
{
 if (val == socket_poll_internal_options.map_type)
   return (TRUE);

 if (socket_poll_offset_num)
   return (FALSE);
 
 socket_poll_internal_options.map_type = val;

 if (socket_poll_internal_options.map_type == SOCKET_POLL_TYPE_MAP_DIRECT)
 {
  struct pollfd *scan = socket_poll_indicators;
  size_t size = socket_poll_offset_sz;
  
  while (size > 0)
  {
   scan->fd = -1;
   scan->events = 0;
   scan->revents = 0;
   
   ++scan;
   --size;
  }
 }
 else if (socket_poll_free_arr)
   memset(socket_poll_free_arr, 0, socket_poll_offset_sz);

 return (TRUE);
}

static int socket_poll_internal_realloc(size_t mall_size)
{
 struct pollfd *tmp1 = NULL;
 unsigned char *tmp2 = NULL;
 size_t diff_size = 0;

 if (mall_size > socket_poll_offset_sz)
   diff_size = mall_size - socket_poll_offset_sz;
 
 if (!(tmp1 = realloc(socket_poll_indicators,
                      sizeof(struct pollfd) * mall_size)))
   goto recover_malloc_pollfd;
 
 if (!(tmp2 = realloc(socket_poll_free_arr, mall_size)))
   goto recover_malloc_free_arr;

 if (diff_size)
 {
  if (socket_poll_internal_options.map_type == SOCKET_POLL_TYPE_MAP_DIRECT)
  {
   struct pollfd *scan = tmp1 + socket_poll_offset_sz;
   
   while (diff_size > 0)
   {
    scan->fd = -1;
    scan->events = 0;
    scan->revents = 0;

    ++scan;
    --diff_size;
   }
  }
  else
    memset(tmp2 + socket_poll_offset_sz, 0, diff_size);
 }
 
 socket_poll_offset_sz = mall_size;
 socket_poll_indicators = tmp1;
 socket_poll_free_arr = tmp2;

 return (TRUE);

 recover_malloc_free_arr:
 free(tmp1);
 recover_malloc_pollfd:

 return (FALSE);
}

int socket_poll_init(size_t default_sz)
{
 if (!default_sz)
   default_sz = socket_poll_grow_sz;

 return (socket_poll_internal_realloc(default_sz));
}

Socket_poll_typedef_offset socket_poll_add(int fd)
{
 Socket_poll_typedef_offset offset = 0;
 unsigned char *first_free = NULL;

 if (!socket_poll_offset_sz)
   socket_poll_init(0);
 
 assert(socket_poll_offset_num <= socket_poll_offset_sz);
 assert(socket_poll_offset_max <= socket_poll_offset_sz);

 if (fd == -1)
   return (0);

 if (socket_poll_internal_options.map_type == SOCKET_POLL_TYPE_MAP_DIRECT)
 {
  if ((socket_poll_offset_sz < (size_t)fd) &&
      !socket_poll_internal_realloc((size_t)fd + 1))
  {
   assert(FALSE);
   
   return (0);
  }
  
  first_free = socket_poll_free_arr + (size_t)fd;

  offset = (first_free - socket_poll_free_arr) + 1;

  if (socket_poll_indicators[offset - 1].fd == fd)
  {
   ++socket_poll_free_arr[offset - 1];
   return (offset);
  }
 }
 else
 { /* _TYPE_MAP_COMPRESSED */
  if (socket_poll_offset_sz == socket_poll_offset_num)
  {
   assert(socket_poll_offset_num == socket_poll_offset_max);
   
   if (!socket_poll_internal_realloc(socket_poll_offset_sz +
                                     socket_poll_grow_sz))
   {
    assert(FALSE);
    
    return (0);
   }

   goto last_entry;
  }
  else if (socket_poll_offset_num == socket_poll_offset_max)
  {
  last_entry:
   first_free = socket_poll_free_arr + socket_poll_offset_max;
  }
  else if (socket_poll_offset_min > 1)
    first_free = socket_poll_free_arr + socket_poll_offset_min - 2;
  else
    first_free = memchr(socket_poll_free_arr, 0, socket_poll_offset_sz);
  assert(first_free);

  offset = (first_free - socket_poll_free_arr) + 1;
 }
 
 ++socket_poll_offset_num;
 
 socket_poll_indicators[offset - 1].fd = fd;
 socket_poll_indicators[offset - 1].events = 0;
 socket_poll_indicators[offset - 1].revents = 0;
 
 if (socket_poll_internal_options.map_type == SOCKET_POLL_TYPE_MAP_DIRECT)
   socket_poll_free_arr[offset - 1] = 1;
 else
   socket_poll_free_arr[offset - 1] = UCHAR_MAX;
 
 if (offset > socket_poll_offset_max)
   socket_poll_offset_max = offset;
 if (offset < socket_poll_offset_min)
   socket_poll_offset_min = offset;
 
 return (offset);
}

void socket_poll_del(Socket_poll_typedef_offset offset)
{
 struct pollfd *to_del = NULL;

 if (!offset)
   return;

 to_del = SOCKET_POLL_INDICATOR(offset);

 assert(socket_poll_offset_num && socket_poll_offset_num &&
        socket_poll_offset_max && socket_poll_offset_min);
 assert(offset <= socket_poll_offset_max);
 assert(offset >= socket_poll_offset_min);

 if (socket_poll_internal_options.map_type == SOCKET_POLL_TYPE_MAP_DIRECT)
 {
  if (--socket_poll_free_arr[offset])
    return;
 }
 else
   socket_poll_free_arr[offset - 1] = 0;
 
 --socket_poll_offset_num;
 
 to_del->fd = -1;

 if ((offset == socket_poll_offset_max) && (offset == socket_poll_offset_min))
 {
  assert(!socket_poll_offset_num);
  socket_poll_offset_max = 1;
  socket_poll_offset_min = 1;
 }
 else if (offset == socket_poll_offset_max)
 {
  while (to_del >= (socket_poll_indicators + socket_poll_offset_min))
  {
   if (to_del->fd != -1)
   {
    socket_poll_offset_max = (to_del - socket_poll_indicators) + 1;
    return;
   }
   
   --to_del;
  }
  
  socket_poll_offset_max = socket_poll_offset_min;
 }
 else if (offset == socket_poll_offset_min)
 {
  while (to_del <= (socket_poll_indicators + socket_poll_offset_max))
  {
   if (to_del->fd != -1)
   {
    socket_poll_offset_min = (to_del - socket_poll_indicators) + 1;
    return;
   }
   
   ++to_del;
  }

  socket_poll_offset_min = socket_poll_offset_max;
 }
}

int socket_poll_update_all(int usecs)
{
 if (!socket_poll_offset_max)
 {
  usleep (usecs);
  return (0);
 }
 
 return (poll(socket_poll_indicators + socket_poll_offset_min - 1,
              socket_poll_offset_max - socket_poll_offset_min + 1,
              usecs));
}

int socket_poll_update_one(Socket_poll_typedef_offset offset, int usecs)
{
 return (poll(SOCKET_POLL_INDICATOR(offset), 1, usecs));
}

int socket_poll_cntl_opt(int option, ...)
{
 int ret = 0;
 
 va_list ap;

 va_start(ap, option);
 
 switch (option)
 {
  case SOCKET_POLL_CNTL_OPT_GET_MAP_TYPE:
  {
   int *val = va_arg(ap, int *);
   
   *val = socket_poll_internal_options.map_type;
   ret = TRUE;
  }
  break;
  
  case SOCKET_POLL_CNTL_OPT_SET_MAP_TYPE:
  {
   int val = va_arg(ap, int);

   if ((val != SOCKET_POLL_TYPE_MAP_COMPRESSED) &&
       (val != SOCKET_POLL_TYPE_MAP_DIRECT))
     break;
   
   ret = socket_poll_internal_change_map(val);
  }
  break;
  
  case SOCKET_POLL_CNTL_OPT_GET_MAP_COMPRESSED_GROW:
  {
   size_t *val = va_arg(ap, size_t *);
   
   *val = socket_poll_grow_sz;
   ret = TRUE;
  }
  break;

  case SOCKET_POLL_CNTL_OPT_SET_MAP_COMPRESSED_GROW:
  {
   size_t val = va_arg(ap, size_t);

   if (!val)
     break;
   
   socket_poll_grow_sz = val;
   ret = TRUE;
  }
  break;

  case SOCKET_POLL_CNTL_OPT_GET_MAP_SIZE:
  {
   size_t *val = va_arg(ap, size_t *);
   
   *val = socket_poll_offset_sz;
   ret = TRUE;
  }
  break;

  case SOCKET_POLL_CNTL_OPT_SET_MAP_SIZE:
  {
   size_t val = va_arg(ap, size_t);

   if (val < socket_poll_offset_max)
     break;

   if (!socket_poll_internal_realloc(val))
     break;
   
   ret = TRUE;
  }
  break;

  case SOCKET_POLL_CNTL_OPT_GET_MAP_END_SIZE:
  {
   size_t *val = va_arg(ap, size_t *);

   assert(socket_poll_offset_sz >= socket_poll_offset_max);
   
   *val = socket_poll_offset_sz - socket_poll_offset_max;
   ret = TRUE;
  }
  break;

  case SOCKET_POLL_CNTL_OPT_SET_MAP_END_SIZE:
  {
   size_t val = va_arg(ap, size_t);

   val += socket_poll_offset_max;

   if (!socket_poll_internal_realloc(val))
     break;
   
   ret = TRUE;
  }
  break;

  case SOCKET_POLL_CNTL_OPT_GET_MAP_END_ARRAY:
  {
   struct pollfd **val = va_arg(ap, size_t);

   *val = socket_poll_indicators + socket_poll_offset_max;
   
   ret = TRUE;
  }
  /* FALLTHROUGH */
  case SOCKET_POLL_CNTL_OPT_SET_MAP_END_ARRAY:
    break;

  default:
    break;
 }

 va_end(ap);
 
 return (ret);
}
