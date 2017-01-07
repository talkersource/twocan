#define OUTPUT_COMPRESS_C
/*
 *  Copyright (C) 1999 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@twocan.org
 */
#include "main.h"

#ifdef HAVE_ZLIB_H

static void *internal_output_compress_alloc(void *data, unsigned int items,
                                            unsigned int size)
{
 IGNORE_PARAMETER(data);
 return (XMALLOC(items * size, OUTPUT_COMPRESS_LIB));
}

static void internal_output_compress_free(void *data, void *address)
{
 IGNORE_PARAMETER(data);
 XFREE(address, OUTPUT_COMPRESS_LIB);
}

int output_compress_start_1(player *p)
{
 assert(!p->output_compress_do);
 assert(!p->output_compress_lib);
 assert(!p->output_compress_buf);

 if (!(p->output_compress_lib = XMALLOC(sizeof(z_stream),
                                        OUTPUT_COMPRESS_STREAM)))
   goto malloc_stream_error;
 
 if (!(p->output_compress_buf = XMALLOC(OUTPUT_COMPRESS_BUF_SZ,
                                        OUTPUT_COMPRESS_BUF)))
   goto malloc_buf_error;

 p->output_compress_ptr = p->output_compress_buf;
 
 p->output_compress_lib->next_in = NULL;
 p->output_compress_lib->avail_in = 0;

 p->output_compress_lib->next_out = p->output_compress_buf;
 p->output_compress_lib->avail_out = OUTPUT_COMPRESS_BUF_SZ;

 p->output_compress_lib->zalloc = internal_output_compress_alloc;
 p->output_compress_lib->zfree  = internal_output_compress_free;
 p->output_compress_lib->opaque = p;

 if (deflateInit(p->output_compress_lib, OUTPUT_COMPRESS_LEVEL) != Z_OK)
   goto init_error;

 return (TRUE);

 init_error:
 XFREE(p->output_compress_buf, OUTPUT_COMPRESS_BUF);
 p->output_compress_buf = NULL;
 p->output_compress_ptr = NULL;
 malloc_buf_error:
 XFREE(p->output_compress_lib, OUTPUT_COMPRESS_STREAM);
 p->output_compress_lib = NULL;
 malloc_stream_error:
 user_logoff(p, NULL); /* possibly don't need this */
 return (FALSE);
}

void output_compress_start_2(player *p)
{
 assert(!p->output_compress_do);
 assert(p->output_compress_lib);
 assert(p->output_compress_buf);
  
 p->output_compress_do = TRUE;
 logon_shortcut_logon_start(p);
}

int output_compress_stop_1(player *p)
{
 assert(p->output_compress_do);
 assert(p->output_compress_lib);
 assert(p->output_compress_buf);

 return (TRUE);
}

void output_compress_stop_2(player *p)
{ 
 assert(p->output_compress_lib);
 assert(p->output_compress_buf);

 deflateEnd(p->output_compress_lib);

 XFREE(p->output_compress_buf, OUTPUT_COMPRESS_BUF);
 p->output_compress_buf = NULL;
 p->output_compress_ptr = NULL;
 
 XFREE(p->output_compress_lib, OUTPUT_COMPRESS_STREAM);
 p->output_compress_lib = NULL;

 p->output_compress_do = FALSE;
}

int output_compress_writev_1(player *p, struct iovec *iovs, int used,
                             int found_mark)
{
 int count = 0;
 z_stream *tmp = p->output_compress_lib;
 int ret = 0;
 int expecting = 0;
 
 assert(p->output_compress_do);
 assert(p->output_compress_lib);
 assert(p->output_compress_buf);
 
 while ((count < used) && tmp->avail_out)
 {
  int flag = Z_NO_FLUSH;
  int def_ret = 0;
  
  tmp->next_in = iovs[count].iov_base;
  expecting = tmp->avail_in = iovs[count].iov_len;
  ++count;
  
  if (count == used)
  {
   if (found_mark)
     flag = Z_FINISH;
   else
     flag = Z_SYNC_FLUSH;
  }

 try_deflate:
  SCHEDULE_CODE_GENERIC_START();
  def_ret = deflate(tmp, flag);
  SCHEDULE_CODE_GENERIC_END(&p->comp_cpu);
    
  if (def_ret == Z_OK)
  {
   if (!tmp->avail_out)
   {
    if (tmp->next_out == (p->output_compress_buf + OUTPUT_COMPRESS_BUF_SZ))
    {
     tmp->next_out = p->output_compress_buf;
     tmp->avail_out = p->output_compress_ptr - p->output_compress_buf;
     if (tmp->avail_out)
       goto try_deflate;
    }
    break;
   }
  }
  else if (def_ret != Z_STREAM_END)
    break;
  
  assert(!tmp->avail_in);
  
  ret += expecting;
 }
 if (tmp->avail_in)
   ret += expecting - tmp->avail_in;

 tmp->next_in = NULL;
 tmp->avail_in = 0;
 
 return (ret);
}

int output_compress_writev_2(player *p)
{
 struct iovec iovs[2];
 z_stream *tmp = p->output_compress_lib;
 int first = 0;
 int second = 0;
 int ret = 0;
 
 if (tmp->avail_out == OUTPUT_COMPRESS_BUF_SZ)
 {
  assert(tmp->next_out == p->output_compress_ptr);
  assert(tmp->next_out == p->output_compress_buf);

  return (FALSE);
 }
 
 if (p->output_compress_ptr >= tmp->next_out)
 {
  assert((p->output_compress_ptr > tmp->next_out) ||
         !tmp->avail_out);

  iovs[0].iov_base = p->output_compress_ptr;
  iovs[0].iov_len = (OUTPUT_COMPRESS_BUF_SZ -
                     (p->output_compress_ptr - p->output_compress_buf));
  first = iovs[0].iov_len;

  iovs[1].iov_base = p->output_compress_buf;
  iovs[1].iov_len = tmp->next_out - p->output_compress_buf;
  second = iovs[1].iov_len;
  
  assert(second || !tmp->avail_out);
 }
 else
 {
  iovs[0].iov_base = p->output_compress_ptr;
  iovs[0].iov_len = (tmp->next_out - p->output_compress_ptr);
  first = iovs[0].iov_len;
  second = 0;  
 }

 ret = socket_writev(&SOCKET_POLL_INDICATOR(p->io_indicator)->fd,
                     iovs, second ? 2 : 1);
 
 if (!ret || (ret == -1))
 {
  return (TRUE);
 }
 if (ret == (first + second))
 {
  tmp->next_out = p->output_compress_ptr = p->output_compress_buf;
  tmp->avail_out = OUTPUT_COMPRESS_BUF_SZ;
  return (FALSE);
 }
 
 if (second)
 {
  if (ret > first)
    tmp->avail_out += first;
  else
    tmp->avail_out += ret;
 }
 
 return (TRUE);
}

#else

extern int output_compress_start_1(player *p)
{
 IGNORE_PARAMETER(p);
 return (FALSE);
}

extern void output_compress_start_2(player *p)
{
 IGNORE_PARAMETER(p);
 log_assert(FALSE);
}

extern int output_compress_stop_1(player *p)
{
 IGNORE_PARAMETER(p);
 return (FALSE);
}

extern void output_compress_stop_2(player *p)
{
 IGNORE_PARAMETER(p);
 log_assert(FALSE);
}

extern int output_compress_writev_1(player *p, struct iovec *iovs, int used,
                                    int ignore)
{
 IGNORE_PARAMETER(ignore);
 log_assert(FALSE);
 return (socket_writev(&p->io_indicator->fd, iovs, used));
}

extern int output_compress_writev_2(player *p)
{
 IGNORE_PARAMETER(p);
 log_assert(FALSE);
 return (FALSE);
}

#endif
