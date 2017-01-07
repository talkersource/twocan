#define CHILD_COM_C
/*
 *  Copyright (C) 1999, 2000 James Antill
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

#define HANDLE_FD_CREATE_ERROR(recover) do { switch (errno) \
 { \
  case EBADF: goto recover; \
  default: \
    assert(1 && 0); \
  case EFAULT: \
    assert(0 && 1); \
  case EMFILE: /* too many fd's */ \
  case ENFILE: /* sys file table full */ \
    goto recover; \
 } } while (FALSE)


void child_com_open(child_com *node, int input, FILE *output, pid_t pid)
{
 fcntl(input, F_SETFL, O_NONBLOCK);

 node->input = input;
 node->output = output;
 node->pid = pid;
 node->used = 0;
 node->buffer[0] = 0;
 node->buffer[node->buffer_size - 1] = 0;
 node->cached_terminator = NULL;
 node->c_timestamp = node->s_timestamp = node->r_timestamp = now;
 node->bad = FALSE;
 node->output_waiting = FALSE;
}

child_com *child_com_create(const char *prog_name, char *argv[])
{
 struct stat buf;

 if (!stat(prog_name, &buf) && (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)))
 {
  int saved_fds[2];
  int out_pipe[2];
  int in_pipe[2];
  char *dummy_argv[] = {0, NULL};
#ifdef HAVE_NON_CONST_ARGV
  char prog_name_copy[512];
#endif
  pid_t child_pid = 0;
  child_com *child_node = XMALLOC(sizeof(child_com), CHILD_COM);
  
  if (!child_node)
    return (NULL);

  child_node->buffer_size = CHILD_COM_DEFAULT_SIZE;
  
  if ((saved_fds[0] = dup(0)) < 0)
    HANDLE_FD_CREATE_ERROR(recover_dup_1);
  if ((saved_fds[1] = dup(1)) < 0)
    HANDLE_FD_CREATE_ERROR(recover_dup_2);
  
  if (pipe(out_pipe) < 0)
    HANDLE_FD_CREATE_ERROR(recover_pipe_1);
  if (pipe(in_pipe) < 0)
    HANDLE_FD_CREATE_ERROR(recover_pipe_2);
  
  if (dup2(out_pipe[0], 0) < 0)
    HANDLE_FD_CREATE_ERROR(recover_dup2_1);
  if (dup2(in_pipe[1], 1) < 0)
    HANDLE_FD_CREATE_ERROR(recover_dup2_2);
  
  close(out_pipe[0]);
  close(in_pipe[1]);
  
  if (!argv) /* ease of use */
  {
   const char *last_slash = NULL;
   /* for historical reasons exec takes char *[],
    * but it is treated like const char *[] *sigh*
    * If this isn't true on a weird system define HAVE_NON_CONST_ARGV
    */
#ifndef HAVE_NON_CONST_ARGV
   argv = dummy_argv;
   
   if ((last_slash = C_strrchr(prog_name, '/')))
     ++last_slash;
   else
     last_slash = prog_name;
   argv[0] = (char *)last_slash; /* warning */
#else
   argv = dummy_argv;

   if ((last_slash = C_strrchr(prog_name, '/')))
     sprintf(prog_name_copy, "%.*s", 511, last_slash + 1);
   else
     sprintf(prog_name_copy, "%.*s", 511, prog_name);
   prog_name_copy[511] = 0;
   argv[0] = prog_name_copy;
#endif
  }
  
  switch ((child_pid = fork()))
  {
   case -1: /* failure */
     XFREE(child_node, CHILD_COM);
     child_node = NULL;
     break;
     
   case 0: /* child */
     close(out_pipe[1]);
     close(in_pipe[0]);
     close(saved_fds[0]);
     close(saved_fds[1]);
     fcntl(0, F_SETFL, O_NONBLOCK);
     execvp(prog_name, argv);
     /* shoud never happen because of the stat call */
     assert(FALSE);
     vwlog("child_com.error", " Failed to exec -- %s -- \n", prog_name);
     while (1)
       _exit (EXIT_FAILURE);
     
   default: /* parent */
     child_com_open(child_node,
                    in_pipe[0], fdopen(out_pipe[1], "w"),
                    child_pid);
  }
  
  dup2(saved_fds[0], 0); /* this has ? to work */
  dup2(saved_fds[1], 1);
  close(saved_fds[0]);
  close(saved_fds[1]);
  
  return (child_node);
  
 recover_dup2_2:
  close(out_pipe[0]);
  dup2(saved_fds[0], 0); /* this really has to work -- we really hope */
 recover_dup2_1:
  close(in_pipe[0]);
  close(in_pipe[1]);
  
 recover_pipe_2:
  close(out_pipe[0]);
  close(out_pipe[1]);
 recover_pipe_1:
  close(saved_fds[1]);
  
 recover_dup_2:
  close(saved_fds[0]);
 recover_dup_1:
  return (NULL); /* needed for ANSI C */
 }

 return (NULL);
}

void child_com_close(child_com *node)
{
 if (kill(node->pid, SIGTERM) != -1)
 {
  sleep(1);
  /* just make core resource 0 and it will act like a SIGKILL */
  kill(node->pid, SIGABRT);
 }
 
 close(node->input);
 fclose(node->output);
}

void child_com_delete(child_com *child_node)
{
 child_com_close(child_node);
 XFREE(child_node, CHILD_COM);
}

/* actual coms ... */

void child_com_send(child_com *child_node, const char *fmt, ...)
{
 va_list ap;

 va_start(ap, fmt);
 vfprintf(child_node->output, fmt, ap);
 va_end(ap);

 child_node->s_timestamp = now;
}

int child_com_flush(child_com *child_node)
{
 if (fflush(child_node->output) == EOF)
   switch (errno)
   {
   case EAGAIN:
   case EINTR:
   case ENOSPC:
     child_node->output_waiting = TRUE;
     return (TRUE);

   default:
     child_node->bad = TRUE;
     return (FALSE);
   }

 child_node->output_waiting = FALSE;
 child_node->s_timestamp = now;
 return (TRUE);
}

static size_t child_com_read(child_com *node)
{
 if (SOCKET_POLL_INDICATOR(node->io_indicator)->revents &
     (POLLERR | POLLHUP | POLLNVAL))
 {
  node->bad = TRUE;
  return (0);
 }
 
 if ((SOCKET_POLL_INDICATOR(node->io_indicator)->revents & POLLIN) &&
     (node->used < node->buffer_size))
 {
  int tmp = read(node->input, node->buffer + node->used,
                 node->buffer_size - node->used);
  
  if (tmp == -1)
    switch (errno)
    {
     default:
       node->bad = TRUE;
       return (0);
       
       /* FALLTHROUGH */
     case EINTR: /* interupt occured */
     case EAGAIN: /* too little data */
       tmp = 0;
       break;
    }
  else
  {
   node->r_timestamp = now;
   node->used += tmp;
  }
 }
 else if (kill(node->pid, 0) < 0)
 {
  node->bad = TRUE;
  return (0);
 }

 return (node->used);
}

size_t child_com_gather(child_com *node, char terminator)
{
 ssize_t ret = 0;

 assert(node->used <= node->buffer_size);

 if (node->cached_terminator &&
     (*node->cached_terminator == terminator))
   return (node->cached_terminator - node->buffer + 1);

 child_com_read(node);

 if (node->bad)
   return (0);

 if (node->used)
   node->cached_terminator = N_memchr(node->buffer,
                                      (unsigned char)terminator, node->used);
 
 if (node->cached_terminator)
   ret = (node->cached_terminator - node->buffer + 1);
  
 return (ret); 
}

size_t child_com_recv(child_com *node, char *buffer, size_t size)
{
 assert(size);
 assert(node->used <= node->buffer_size);
 
 if (size > node->used)
 {
  child_com_read(node);

  if (node->bad)
    return (0);
  
  if (size > node->used)
    return (node->used);
 }

 if (buffer)
   memcpy(buffer, node->buffer, size);
 
 if (size < node->used)
 {
  memmove(node->buffer, node->buffer + size, node->used - size);
  node->used -= size;
 }
 else
 {
  assert(size == node->used);
  node->used = 0;
 }
 node->cached_terminator = NULL;
 
 return (size);
}

int child_com_waiting_input(child_com *node, int seconds)
{
 if (difftime(node->s_timestamp, node->r_timestamp) > 0)
 { /* sent after we recieved */
  if (difftime(now, node->r_timestamp) > seconds)
    return (TRUE);
 }

 return (FALSE);
}
