#define TEST3_C

#include "main.h"

static int keep_going = TRUE;
static int local_file = 0;
static size_t file_len = 0;

static int q_read_func(socket_com_q_node *, int, size_t);
    
static int q_read_func2(socket_com_q_node *node, int flags, size_t size)
{
 struct iovec iovs[1000];
 int count = 1000;
 size_t len = 0;
 ssize_t ret = 0;

 if (!file_len)
 {
  node->read_func = q_read_func;
  SOCKET_COM_Q_SET_TYPE(node, TERMINATED, '\n');
  
  return (flags | SOCKET_COM_Q_RET_REORDER_START | SOCKET_COM_Q_RET_NO_CLEANUP |
          SOCKET_COM_Q_RET_REEVAL_TYPE);
 }
 
 if (!(len = socket_com_read_iovec(node->base, iovs, &count, file_len)))
   return (flags |
           SOCKET_COM_Q_RET_REORDER_START | SOCKET_COM_Q_RET_NO_CLEANUP);
 
 ret = writev(local_file, iovs, count);
 
 if (!ret || (ret == -1))
   return (flags |
           SOCKET_COM_Q_RET_REORDER_START | SOCKET_COM_Q_RET_NO_CLEANUP);

 assert(len <= size);
 
 len = ret;
 file_len -= len;
 socket_com_read_cleanup(node->base, len);
 
 return (flags | SOCKET_COM_Q_RET_REORDER_START | SOCKET_COM_Q_RET_NO_CLEANUP);
}

static int q_read_func(socket_com_q_node *node, int flags, size_t size)
{
 char buf[1024 * 1024];
 
 if (size > sizeof(buf))
   return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);
 
 socket_com_read_buf(node->base, buf, size);
 fwrite(buf, 1, size, stderr);
 fflush(NULL);
 if (strncmp("SEND FILE ", buf, sizeof("SEND FILE ") - 1))
   return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);

 socket_com_read_cleanup(node->base, size);

 file_len = atoi(buf + sizeof("SEND FILE ") - 1);

 socket_com_write_fmt(node->base, "RECV FILE %d\n", file_len);

 node->read_func = q_read_func2;
 SOCKET_COM_Q_SET_TYPE(node, SIZED, 1);

 return (flags | SOCKET_COM_Q_RET_REORDER_START | SOCKET_COM_Q_RET_NO_CLEANUP |
         SOCKET_COM_Q_RET_REEVAL_TYPE);
  
 if (node->base->read_io.len >= file_len)
 { /* handle locally */
  
  socket_com_read_cleanup(node->base, file_len);
  return (SOCKET_COM_Q_RET_REORDER_START | SOCKET_COM_Q_RET_NO_CLEANUP);
 }
 
 if (!socket_com_read_sendfile(node->base, local_file, file_len))
   return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);
   
 return (SOCKET_COM_Q_RET_REORDER_START | SOCKET_COM_Q_RET_NO_CLEANUP |
         SOCKET_COM_Q_RET_STOP_READ);
}

static int q_bad_func(socket_com_q_node *node, int flags)
{
 IGNORE_PARAMETER(flags);
 
 if (node->base->read_bad)
   fprintf(stderr, "%ld read bad\n", (long)getpid());
 if (node->base->write_bad)
   fprintf(stderr, "%ld write bad\n", (long)getpid());
 if (node->base->malloc_bad)
   fprintf(stderr, "%ld malloc bad\n", (long)getpid());
 
 keep_going = FALSE;

 return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);
}

int main(void)
{
 socket_com_base *base = socket_com_create_base(0, 1);
 socket_com_q_node *q_node = NULL;
 
 nice(20);
 
 if (!base)
   exit (EXIT_FAILURE);

 local_file = open("test_file4.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
 assert(local_file != -1);

 q_node = socket_com_q_add_node(base);
 SOCKET_COM_Q_SET_TYPE(q_node, TERMINATED, '\n');
 q_node->read_func = q_read_func;
 q_node->bad_func = q_bad_func;
 q_node->limit_len = (1024 * 1024);
 
 fprintf(stderr, "%ld starting\n", (long)getpid());
 while (keep_going && socket_com_q_num)
 {
  socket_com_q_update_all(20000);
  socket_com_q_run();
 }

 exit (EXIT_SUCCESS);
}
