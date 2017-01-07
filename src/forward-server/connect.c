#define CONNECT_C

#include "main.h"

static void setup_socket(int fd)
{
 struct linger lingerval;
 int dummy = 1;
 
 setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &dummy, sizeof(int));
 
 lingerval.l_onoff = 1;
 lingerval.l_linger = 0;
 
 setsockopt(fd, SOL_SOCKET, SO_LINGER, (struct linger *) &lingerval,
            sizeof(struct linger));
}

int connect_to_site(const char *const numeric_address, int port)
{
 int new_socket = socket(AF_INET, SOCK_STREAM, 0);
 struct in_addr inet_address;
 struct sockaddr_in sa;
 
 if (new_socket < 0)
 {
  fprintf(stderr, " Failed to create socket.\n");
  exit(200);
 }
 
 if (!inet_aton(numeric_address, &inet_address))
 {
  fprintf(stderr, " Bad address.\n");
  exit( -1);
 }
 
 printf(" (debug) setting up the socket.\n");
 setup_socket(new_socket);
 
 sa.sin_family = AF_INET;
 sa.sin_port = htons(port);
 sa.sin_addr = inet_address;
 
 printf(" (debug) connecting up the socket.\n");
 
 if (connect(new_socket, (struct sockaddr *) &sa, sizeof(sa)))
 {
  fprintf(stderr, " Connect failed.\n");
  exit (200);
 }
 
 return (new_socket);
}

int send_to_socket(int fd, const char *fmt, ...)
{
 char buf[2048];
 int length = 0;
 va_list va;
 
 va_start(va, fmt);
 length = vsprintf(buf, fmt, va);
 va_end(va);
 
 write(fd, buf, length);
 
 return (length);
}

const char *read_data_maybe(char *buffer, size_t size, int fd)
{
 int chars_left;
 int c;
 fd_set fd_list;
 struct timeval timeout;
 int length = 0;
 
 FD_ZERO(&fd_list);
 FD_SET(fd, &fd_list);
 
 timeout.tv_sec = 0;
 timeout.tv_usec = 1000;
 
 if (select(FD_SETSIZE, &fd_list, 0, 0, &timeout) < 1)
 {
  sleep (1);
#ifdef SINGLE_PROCESS
  return (read_data_maybe(buffer, size, fd));
#else
  return (NULL); /* no chars in the queue */
#endif
 }  
 
 if (ioctl(fd, FIONREAD, &chars_left) == -1)
 {
  vwlog("error", "ioctl: %d %s\n", errno, strerror(errno));
  
  close(fd);
  
  return (NULL);
 }
 
 if (!chars_left)
 {
  shutdown(fd, 2);
  close(fd);
  
  /* the select SHOULD have failed, or the link has died */
  
  return (NULL);
 }
 else
   if (((size_t)chars_left) >= size)
     chars_left = size - 1;
 
 while (chars_left--)
 {
  if (read(fd, &c, 1) != 1)
  {
   shutdown(fd, 2);
   close(fd);
   
   return (NULL);
  }
  
  if ((buffer[length++] = c) == '\n')
  {
   buffer[length] = 0;
   return (buffer);
  }
 }
 
 return (buffer);
}

const char *read_data(char *buffer, size_t size, int fd)
{
 int c;
 int length = 0;
 
 if (!size)
   return (NULL);
 
 while (--size > 0)
 {
  if (read(fd, &c, 1) != 1)
  {
   shutdown(fd, 2);
   close(fd);
   
   return (NULL);
  }
  
  if ((buffer[length++] = c) == '\n')
  {
   buffer[length] = 0;
   return (buffer);
  }
 }
 assert (size == 0);
 
 buffer[length] = 0;
 return (buffer);
}


