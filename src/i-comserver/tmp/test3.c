#define TEST3_C

#include "main.h"

static int keep_going = TRUE;
static unsigned int count = 0;

static int q_read_func(socket_com_q_node *node, int flags, size_t size)
{
 char buf[1024 * 8];
 int len = 0;

 if (size > sizeof(buf))
   return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);
 
 socket_com_write_fmt(node->base, "test3:%08u:%8u %n", ++count,
                      (unsigned)getpid(), &len);
 socket_com_write_fmt(node->base, "%*s", 80 - len,
                      "-------------------------------------------------"
                      );
 socket_com_write_fmt(node->base, "%*s\n",
                      80, "-------------------------------------------------"
                      );

 socket_com_read_buf(node->base, buf, size);
 fwrite(buf, 1, size, stderr);
 
 fflush(NULL);

 return (flags | SOCKET_COM_Q_RET_REORDER_START);
}

static int q_read_cmsg_func(socket_com_q_node *node, int flags, int cmsg_flags)
{
 int len = 0;
 int fd = -1;
 struct ucred creds;
 
 if (cmsg_flags & SOCKET_COM_Q_CMSG_FDS)
 {
  if (!socket_com_read_fds(node->base, &fd, 1))
    assert(FALSE);
 }
 
 socket_com_write_fmt(node->base, "test3:%08u:%8u %n", count + 1,
                      (unsigned)getpid(), &len);
 socket_com_write_fmt(node->base, "%*s", 80 - len,
                      "-------------------------------------------------");

 if (cmsg_flags & SOCKET_COM_Q_CMSG_FDS)
   socket_com_write_fmt(node->base, "%*s", 80,
                        "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");
 
 if (cmsg_flags & SOCKET_COM_Q_CMSG_CREDS)
 {
  if (!socket_com_read_creds_pass(node->base, &creds))
    assert(FALSE);
  
  socket_com_write_fmt(node->base, "%*s", 80,
                       "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
 }
 
 return (flags | SOCKET_COM_Q_RET_REORDER_START);
}

static int q_bad_func(socket_com_q_node *node, int flags)
{
 IGNORE_PARAMETER(flags);
 
 if (node->base->read_bad)
   fprintf(stderr, "%ld read bad\n", (long)getpid());
 if (node->base->read_cmsg_bad)
   fprintf(stderr, "%ld read cmsg bad\n", (long)getpid());
 if (node->base->write_bad)
   fprintf(stderr, "%ld write bad\n", (long)getpid());
 if (node->base->write_creds_bad)
   fprintf(stderr, "%ld write creds bad\n", (long)getpid());
 if (node->base->malloc_bad)
   fprintf(stderr, "%ld malloc bad\n", (long)getpid());
 
 keep_going = FALSE;

 return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);
}

int main(void)
{
 int len = 1;
 socket_com_base *base = socket_com_create_base(0, 1);
 socket_com_q_node *q_node = NULL;
 
 nice(20);
 
 if (!base)
   exit (EXIT_FAILURE);

 if (!socket_com_read_fd_sz(base, 200))
   exit (EXIT_FAILURE);

 if (!socket_com_read_creds_start(base))
   exit (EXIT_FAILURE);
 
 q_node = socket_com_q_add_node(base);
 SOCKET_COM_Q_SET_TYPE(q_node, TERMINATED, '\n');
 q_node->read_func = q_read_func;
 q_node->read_cmsg_func = q_read_cmsg_func;
 q_node->bad_func = q_bad_func;
 q_node->limit_len = (8 * 1024);

 fprintf(stderr, "%ld starting\n", (long)getpid());

 sleep (4);
 
 socket_com_write_fmt(base, "test3:%08u:%8u %n", ++count,
                      (unsigned)getpid(), &len);
 socket_com_write_fmt(base, "%*s", 80 - len,
                      "-------------------------------------------------"
                      );
 socket_com_write_fmt(base, "%*s\n",
                      80, "-------------------------------------------------"
                      );

 while (keep_going && socket_com_q_num)
 {
  socket_com_q_update_all(2000);
  socket_com_q_run();
 }
 
 exit (EXIT_SUCCESS);
}

