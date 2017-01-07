#define DNS_C

/* accepts an address ... sends back address:hostname */


#include "main.h"

static unsigned int requests_going = 0;
static dns_request requests[MAX_PARALLELISM]; /* statics are nulled */

#if 0
static struct pollfd fds[MAX_PARALLELISM + 1];
static int fds_count = 0;
#endif

time_t now;

static void init_signals(void)
{
 struct sigaction sa;

 sigemptyset(&sa.sa_mask);
 sa.sa_handler = SIG_IGN;
 sa.sa_flags = SA_NOCLDSTOP;
 sigaction(SIGCHLD, &sa, NULL);
#ifdef HAVE_PRCTL
 if (prctl(PR_SET_PDEATHSIG, 9, 0, 0, 0) == -1)
 { /* FIXME: doesn't work ... */ }
#endif
}

static int find_slot(const char *inet_address)
{
 int tmp = 0;
 int done = 0;
 int last_unused = 0;
 
 while (tmp < MAX_PARALLELISM)
 {
  if (!requests[tmp].child)
  {
   if (!done)
     last_unused = tmp;
   done = TRUE;
  }
  else if (!strcmp(inet_address, requests[tmp].inet_address))
    return (-1); /* we already have a request going for that ip */
  
  ++tmp;
 }

 assert(done);
 
 return (last_unused);
}

#if 0
static struct pollfd *get_poll(void)
{
 int count = 0;

 while (count < MAX_PARALLELISM + 1)
 {
  if (fds[count].fd == -1)
  {
   if ((count + 1) > fds_count)
     fds_count = count + 1;
   
   return (fds + count);
  }
  
  ++count;
 }

 assert(FALSE);
 return (NULL);
}

static void shrink_poll(struct pollfd *io_indicator)
{
 int count = io_indicator - fds;

 io_indicator->fd = -1;
 assert(count >= 0);
 if (count >= fds_count)
 {
  assert((count == fds_count) && (fds[count].fd == -1));
  while (fds[fds_count].fd == -1)
    --fds_count;
 }
}

static void init_polls(void)
{
 int count = 0;

 while (count < MAX_PARALLELISM + 1)
   fds[count++].fd = -1;
}

#endif

static int deal_with_main(child_com *parent)
{
 char buffer[MAX_INET_ADDRESS + 1];
 int slot = -1;
 size_t chars_toget = child_com_gather(parent, '\n');
 child_com *child = NULL;

 assert(chars_toget <= MAX_INET_ADDRESS);
 if (chars_toget > MAX_INET_ADDRESS)
   chars_toget = MAX_INET_ADDRESS;
 
 if ((chars_toget > 0) && (requests_going < MAX_PARALLELISM))
   if ((child = child_com_create("bin/dns_child", NULL)))
   {
    if (child_com_recv(parent, buffer, chars_toget) == chars_toget)
    {
     buffer[chars_toget - 1] = 0;
     if ((slot = find_slot(buffer)) >= 0)
     {
      memcpy(requests[slot].inet_address, buffer, MAX_INET_ADDRESS);
      
      requests[slot].child = child;
      child_com_send(requests[slot].child, "%s\n",
                     requests[slot].inet_address);
      if (!child_com_flush(requests[slot].child))
      {
       child_com_delete(requests[slot].child);
       requests[slot].child = NULL;
      }
      else
      {
       Socket_poll_typedef_offset io = socket_poll_add(child->input);
       
       if (io)
       {
        CHILD_COM_POLL(child, io);
        ++requests_going;
       }
       else
       {
        socket_poll_del(child->io_indicator);
        requests[slot].child = NULL;
        child_com_delete(child);
       }
      }
     }
     else
       child_com_delete(child);
    }
    else
      child_com_delete(child);
   }
 
 if (chars_toget > 0)
   return (TRUE);
 
 if (parent->bad)
 {
  remove("junk/dns_server_PID");
  exit (EXIT_FAILURE);
 }
 
 return (FALSE);
}

static void deal_with_requests(child_com *parent)
{
 int scan = 0;
 unsigned int count = 0;

 while ((scan < MAX_PARALLELISM) && (count < requests_going))
 {
  if (requests[scan].child)
  {
   size_t chars_toget = child_com_gather(requests[scan].child, '\n');

   if (chars_toget)
   {
    char buffer[1024];

    if (chars_toget > 1023)
      chars_toget = 1023;
    
    if (child_com_recv(requests[scan].child, buffer, chars_toget) ==
        chars_toget)
      buffer[chars_toget - 1] = 0;
    else
    {
     assert(FALSE);
    }

    child_com_send(parent, "%s:%s\n", requests[scan].inet_address, buffer);   
    if (!child_com_flush(parent))
    {
     remove("junk/dns_server_PID");
     exit (EXIT_FAILURE);
    }

    socket_poll_del(requests[scan].child->io_indicator);
    child_com_delete(requests[scan].child);
    requests[scan].child = NULL;
    --requests_going;
   }
   else
   {
    if (requests[scan].child->bad ||
        child_com_waiting_input(requests[scan].child, MAX_DNS_TIMEOUT))
    {
     child_com_send(parent, "%s:%s\n",
                    requests[scan].inet_address, requests[scan].inet_address);
     if (!child_com_flush(parent))
     {
      remove("junk/dns_server_PID");
      exit (EXIT_FAILURE);
     }

     socket_poll_del(requests[scan].child->io_indicator);
     child_com_delete(requests[scan].child);
     requests[scan].child = NULL;
     --requests_going;
    }
    else
    {
     if (requests[scan].child->output_waiting &&
         !child_com_flush(requests[scan].child))
     {
      socket_poll_del(requests[scan].child->io_indicator);
      child_com_delete(requests[scan].child);
      requests[scan].child = NULL;
      --requests_going;
     }
     
     ++count;
    }
   }
  }

  ++scan;
 }
}

void init_sys_rlim(void)
{
 SYS_RLIM_CHANGE(RLIMIT_CORE, 0);
}

int main(void)
{
 child_com real_parent = CHILD_COM_INIT(CHILD_COM_DEFAULT_SIZE);
 child_com *parent = &real_parent;
 
 init_signals();
 init_sys_rlim();
 /* init_polls(); */
 
 now = time(NULL);

 log_pid("junk/dns_server_PID", TRUE);
 
 child_com_open(parent, fileno(stdin), stdout, getppid());
 CHILD_COM_POLL(parent, socket_poll_add(parent->input));
 /* ++fds_count; */
 
 while (TRUE)
 {
  int do_sleep = TRUE;
  
  assert(requests_going <= MAX_PARALLELISM);

  now = time(NULL);
  if (deal_with_main(parent))
    do_sleep = FALSE;
  else if ((getppid() == 1) ||
           child_com_waiting_input(parent, MAX_SERVER_TIMEOUT))
      break;
  
  deal_with_requests(parent);

  socket_poll_update_all(do_sleep ? MAX_SERVER_TIMEOUT : 0);
 }

 remove("junk/dns_server_PID");
 exit (EXIT_SUCCESS);
}
