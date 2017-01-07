#define TEST1B_C

#undef NDEBUG

#include "main.h"

static int keep_going = TRUE;
static int local_file = 0;

static int q_read_func(socket_com_q_node *node, int flags, size_t size)
{
 char buf[1024 * 8];

 if (size > sizeof(buf))
   return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);
 
 socket_com_read_buf(node->base, buf, size);
 fwrite(buf, 1, size, stderr);
 fflush(NULL);
 if (strncmp("RECV FILE ", buf, sizeof("RECV FILE ") - 1))
   return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);

 return (flags | SOCKET_COM_Q_RET_REORDER_START);
}

static int q_write_func(socket_com_q_node *node, int flags, size_t size)
{
 IGNORE_PARAMETER(node && size);
 fprintf(stderr, "abcd\n");
 fflush(NULL);
 return (flags);
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
 socket_com_base *base = socket_com_spawn_socket("./test4", NULL);
 socket_com_q_node *q_node = NULL;
 
 nice(20);
 
 if (!base)
   exit (EXIT_FAILURE);

 local_file = open("test_file1b.txt", O_RDONLY);
 assert(local_file != -1);
 
 q_node = socket_com_q_add_node(base);
 SOCKET_COM_Q_SET_TYPE(q_node, TERMINATED, '\n');
 q_node->read_func = q_read_func;
 q_node->bad_func = q_bad_func;
 q_node->limit_len = (8 * 1024);

 q_node->write_func = q_write_func;
 q_node->ignore_write_call_pre = TRUE;
 
 fprintf(stderr, "%ld starting\n", (long)getpid());

 sleep (4);
 
 {
  struct stat buf;
  off_t offset = 0;
  
  if (fstat(local_file, &buf) == -1)
    perror("fstat");
  
  socket_com_write_fmt(base, "SEND FILE %d\n", (int)buf.st_size);
  if (!socket_com_write_sendfile(base, local_file, buf.st_size, &offset))
    perror("sendfile");
  socket_com_write_fmt(base, "%s", "END\n");
 }

 while (keep_going && socket_com_q_num)
 {
  socket_com_q_update_all(2000);
  socket_com_q_run();
 }
 
 exit (EXIT_SUCCESS);
}

