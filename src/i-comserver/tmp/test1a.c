#define TEST1A_C

#undef NDEBUG

#include "main.h"

static int keep_going = TRUE;
static unsigned int count = 0;
static int local_file = 0;

static int q_read_func(socket_com_q_node *node, int flags, size_t size)
{
 int len = 0;
 char buf[1024 * 1024];

 if (size > sizeof(buf))
   return (SOCKET_COM_Q_RET_DELETE_Q | SOCKET_COM_Q_RET_DELETE_BASE);

 socket_com_write_fmt(node->base, "test1:%08u:%8u %n", ++count,
                      (unsigned)getpid(), &len);
 if (!(count % 8))
 {
  struct ucred creds;
  
  creds.pid = getpid();
  creds.uid = getuid();
  creds.gid = getgid();
  
  if (socket_com_write_creds(node->base, &creds))
    socket_com_write_fmt(node->base, "%*s", 80 - len,
                         "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
                         );
  socket_com_write_fmt(node->base, "%s", "\n");
 }
 else if (!(count % 4))
 {
  int num = 0;

  socket_com_write_fmt(node->base, "%*s", 80 - len,
                       "#################################################"
                       );
  
  while ((num < 2) && socket_com_write_fd(node->base, local_file))
  {
   ++num;
   socket_com_write_fmt(node->base, "%c", '#');
  }

  socket_com_write_fmt(node->base, "%s", "\n");
 }
 else
 {
  socket_com_write_fmt(node->base, "%*s", 80 - len,
                       "*************************************************"
                       );
  socket_com_write_fmt(node->base, "%*s\n",
                       80, "*************************************************"
                       );
 }
 
 socket_com_read_buf(node->base, buf, size);
 fwrite(buf, 1, size, stderr);
 
 fflush(NULL);

 return (flags | SOCKET_COM_Q_RET_REORDER_START);
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
 socket_com_base *base = socket_com_spawn_socket("./test3", NULL);
 socket_com_q_node *q_node = NULL;
 
 nice(20);
 
 if (!base)
   exit (EXIT_FAILURE);

 local_file = open("test_file1a.txt", O_RDONLY);
 assert(local_file != -1);

 base->write_cmsg_len = 1;
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
