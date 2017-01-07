#define DNS_CHILD_C

#include "main.h"

time_t now;


int main(void)
{
 char buffer[1024];
 child_com real_parent = CHILD_COM_INIT(CHILD_COM_DEFAULT_SIZE);
 child_com *parent = &real_parent;
 size_t size = 0;
 unsigned long squashed = -1;
 Socket_poll_typedef_offset p_io;
 time_t start_timestamp;
 
#ifdef HAVE_PRCTL
 if (prctl(PR_SET_PDEATHSIG, 9, 0, 0, 0) == -1)
 { /* FIXME: doesn't work ... */ }
#endif

 start_timestamp = now = time(NULL);
 
 child_com_open(parent, fileno(stdin), stdout, getpid());
 p_io = socket_poll_add(fileno(stdin));
 CHILD_COM_POLL(parent, p_io);
 
 while (!(size = child_com_gather(parent, '\n')))
 {
  if (parent->bad)
    exit (EXIT_FAILURE);

  if ((getppid() == 1) || (difftime(now, start_timestamp) > MK_MINUTES(30)))
    exit (EXIT_FAILURE);

  socket_poll_update_all(MAX_DNS_TIMEOUT + 1);
  now = time(NULL);
 }
 
 if (child_com_recv(parent, buffer, size) == size)
   buffer[size - 1] = 0;
 else
 {
  assert(FALSE);
  exit (EXIT_FAILURE);
 }

 if (inet_pton(AF_INET, buffer, &squashed)) /* AF_ needs to be passed in */
 {
  struct hostent *hp = gethostbyaddr((char *)&squashed,
                                     sizeof(unsigned long), AF_INET);

  if (hp)
    child_com_send(parent, "%s\n", hp->h_name);
  else
    child_com_send(parent, "%s\n", buffer);
 }
 else
   child_com_send(parent, "%s\n", buffer);

 if (!child_com_flush(parent))
   exit (EXIT_FAILURE);
 
 sleep(2);

 exit (EXIT_SUCCESS);
}
