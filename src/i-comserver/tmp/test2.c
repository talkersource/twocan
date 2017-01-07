#define TEST2_C

#include "main.h"


int main(void)
{
 socket_com_base *base = socket_com_create_base(0, 1);
 unsigned int count = 0;
 
 nice(20);
 
 if (!base)
   exit (EXIT_FAILURE);
 
 fprintf(stderr, "%ld starting\n", (long)getpid());
 while (!base->read_bad && !base->write_bad && !base->malloc_bad)
 {
  char buf[1024 * 8];
  size_t size = 0;
  int len = 0;
  
  socket_poll_update_all(2000);

  socket_com_write_fmt(base, "test2:%08u:%8u %n", ++count,
                       (unsigned)getpid(), &len);
  socket_com_write_fmt(base, "%*s", 80 - len,
                       "-------------------------------------------------"
                       );
  socket_com_write_fmt(base, "%*s%*s%*s\n",
                       80, "-------------------------------------------------",
                       80, "-------------------------------------------------",
                       80, "-------------------------------------------------"
                       );
  socket_com_write_call(base);

  socket_com_read_call(base);
  while ((size = socket_com_read_gather(base, '\n')))
  {
   while (size > sizeof(buf))
   {
    socket_com_read_buf(base, buf, sizeof(buf));
    socket_com_read_cleanup(base, sizeof(buf));
    fwrite(buf, 1, sizeof(buf), stderr);
    size -= sizeof(buf);
   }
   if (size)
   {
    socket_com_read_buf(base, buf, size);
    socket_com_read_cleanup(base, size);
    fwrite(buf, 1, size, stderr);
   }
   fflush(NULL);
  }
 }

 if (base->read_bad)
   fprintf(stderr, "%ld read bad\n", (long)getpid());
 if (base->write_bad)
   fprintf(stderr, "%ld write bad\n", (long)getpid());
 if (base->malloc_bad)
   fprintf(stderr, "%ld malloc bad\n", (long)getpid());

 sleep (30);
 
 exit (EXIT_SUCCESS);
}

