#define MAIN_C

#include "main.h"

#define DEFAULT_PORT_LOCAL 6969
#define DEFAULT_PORT_REMOTE 6969
#define DEFAULT_MACHINE_REMOTE "127.0.0.1"

/* #define USE_TIMEOUTS 1 */

const char *program_name = "forward_server";

#define OPTION_STRING "hl:m:r:p:P:V"

struct option long_options[] =
{
 {"help", no_argument, NULL, 'h'},
 {"local-port", no_argument, NULL, 'p'},
 {"remote-port", no_argument, NULL, 'P'},
 {"remote-machine", no_argument, NULL, 'm'},
 {"version", no_argument, NULL, 'V'},

 {NULL, 0, NULL, 0}
};

struct cmd_line_options
{
 unsigned int show_version : 1;
 unsigned int show_help : 1;
} cmd_line_option = {FALSE, FALSE};

int socket_port_local = DEFAULT_PORT_LOCAL;
int socket_port_remote = DEFAULT_PORT_REMOTE;
const char *socket_machine_remote = DEFAULT_MACHINE_REMOTE;

time_t now;

static int main_descriptor = 0;

static void shutdown_error(const char *, ...) __attribute__ ((__noreturn__));
static void shutdown_error(const char *msg, ...)
{
 va_list ap;
 FILE *log_err_msg = NULL;

 va_start(ap, msg);
 
 if ((log_err_msg = open_wlog("error")))
 {
  vfprintf(log_err_msg, msg, ap);  
  close_wlog(log_err_msg);
 }

 va_end(ap);
 
 exit (EXIT_FAILURE);
}

static void usage(int status)
{
 if (status != EXIT_SUCCESS)
   fprintf (stderr, "Try `%s --help' for more information.\n", program_name);
 else
 {
  char hostname_here[1024];

  if (gethostname(hostname_here, 1024 - 1) == -1)
    strcpy(hostname_here, "localhost"); /* it didn't work for some reason */
  hostname_here[1024 - 1] = 0;
  
  printf(" Usage: %s [-h, --help] [-p, -l, --local-port]"
        " [-P, -r, --remote-port] [-m, --remote-machine]"
        " [--version]\n", program_name);

  printf("Remote...\n"
         " Default machine     %s\n"
         " Default port        %d\n"
         "Local...\n"
         " Default machine     %s\n"
         " Default port        %d\n",
         DEFAULT_MACHINE_REMOTE, DEFAULT_PORT_REMOTE,
         hostname_here, DEFAULT_PORT_LOCAL);
 }

 exit(status);
}

static int service_fd(int fd, int write_to_fd)
{
 int chars_ready = 0; 
 
 if ((ioctl(fd, FIONREAD, (caddr_t) &chars_ready) == -1) ||
     !chars_ready)
 {
  exit (EXIT_FAILURE);
 }
 else
 {
  char buffer[1024];

  if (chars_ready > 1024)
    chars_ready = 1024;

  if ((chars_ready = read(fd, buffer, chars_ready)) == -1)
    return (TRUE);
  
  if (write(write_to_fd, buffer, chars_ready) == -1)
    switch (errno)
    {
     case EINTR: /* interupt occured */
     case EAGAIN: /* too much data */
     case ENOSPC: /* too much data */
       return (TRUE);
       
     case EPIPE:
     default:
       shutdown(fd, 0);
     close(fd);
     fd = -1;
     shutdown(write_to_fd, 0);
     close(write_to_fd);
     fd = -1;
     return (FALSE);
    }
 }
 
 return (TRUE);
}

static void service_user(void)
{
 struct sockaddr_in incoming;
 struct hostent *hp;
 socklen_t length = sizeof(incoming);
 int server_fd = 0;
 int user_fd = 0;
 int done_warning = FALSE;
 time_t timestamp = now = time(NULL);

 IGNORE_PARAMETER(done_warning && timestamp);
 
 if (fork())
   /* we don't care if the child doesn't make it....
    * as we will try again in a bit */
   return;
 
 user_fd = accept(main_descriptor, (struct sockaddr *) &incoming, &length);
 server_fd = connect_to_site(socket_machine_remote, socket_port_remote);

 hp = gethostbyaddr((char *) &(incoming.sin_addr.s_addr),
		    sizeof(incoming.sin_addr.s_addr),
		    AF_INET);

 vwlog("users", " From: %s", hp ? hp->h_name : inet_ntoa(incoming.sin_addr));
 
#ifdef USE_TIMEOUTS
 { /* done down here so the parent doesn't have malloc'd memory */
  size_t enter_msg_sz = 0;
  char *enter_msg = file2text("enter_msg.txt", &enter_msg_sz);
  if (enter_msg_sz)
  {
   write(user_fd, enter_msg, enter_msg_sz);
   FREE(enter_msg);
  }
 }
#endif
 
 while ((user_fd > 0) && (server_fd > 0)
#ifdef USE_TIMEOUTS
 &&
        (difftime(now, timestamp) < MK_MINUTES(15))
#endif
        )
 {
  fd_set fset;
  struct timeval tv;

  now = time(NULL);
  
  FD_ZERO(&fset);
  
  FD_SET(user_fd, &fset);
  FD_SET(server_fd, &fset);

  tv.tv_sec = 10;
  tv.tv_usec = 0;

  switch (select(server_fd + 1, &fset, NULL, NULL, &tv))
  {
   case 0:
#ifdef USE_TIMEOUTS
     if ((difftime(now, timestamp) > MK_MINUTES(10)) && !done_warning)
     {
      size_t warn_msg_sz = 0;
      char *warn_msg = file2text("warning_msg.txt", &warn_msg_sz);
      
      if (warn_msg_sz)
      {
       write(user_fd, warn_msg, warn_msg_sz);
       FREE(warn_msg);
       done_warning = TRUE;
      }
     }
#endif
     break;
   
   case -1:
     switch (errno)
     {
     case EINTR:
       continue;

      default:
        exit (EXIT_FAILURE);
     }
     break; /* never reached */
     
   case 1:  
     if (FD_ISSET(server_fd, &fset))
       service_fd(server_fd, user_fd);
     else
     {
      assert(FD_ISSET(user_fd, &fset));
      
      service_fd(user_fd, server_fd);
     }
     break;

   case 2:
     assert(FD_ISSET(server_fd, &fset));
     assert(FD_ISSET(user_fd, &fset));
     if (service_fd(server_fd, user_fd))
       service_fd(user_fd, server_fd);
     break;

   default:
     exit (EXIT_FAILURE);
  }

  now = time(NULL);
 }

#ifdef USE_TIMEOUTS
 if (user_fd > 0)
 {
  size_t leave_msg_sz = 0;
  char *leave_msg = load_file_verbose("leave_msg.txt", &leave_msg_sz);

  assert(difftime(now, timestamp) >= MK_MINUTES(15));
  
  if (leave_msg_sz)
  {
   write(user_fd, leave_msg, leave_msg_sz);
   FREE(leave_msg);
  }
 }
#endif
 
 exit (EXIT_SUCCESS);
}

static void init_signals(void)
{
 struct sigaction sa;

 sigemptyset(&sa.sa_mask);
 sa.sa_handler = SIG_IGN; /* should do an automatic waitpid */
 sa.sa_flags = SA_NOCLDSTOP;
 sigaction(SIGCHLD, &sa, NULL);
}

int main(int argc, char *argv[])
{
 char optchar = 0;
 struct sockaddr_in main_socket;
 int dummy = 1;

 now = time(0);
 
 if (argv[0])
 {
  if ((program_name = C_strrchr(argv[0], '/')))
    ++program_name;
  else
    program_name = argv[0];
 }
 
 while ((optchar = getopt_long(argc, argv, OPTION_STRING, long_options, NULL))
        != EOF)
   switch (optchar)
   {
    case '?':
      usage(EXIT_FAILURE);
      
    case 'p':
    case 'l':
      socket_port_local = atoi(optarg);
      break;
      
    case 'r':
    case 'P':
      socket_port_remote = atoi(optarg);
      break;
      
    case 'm':
      socket_machine_remote = optarg;
      break;
      
    case 'V':
      cmd_line_option.show_version = TRUE;
      break;
      
    case 'h':
      cmd_line_option.show_help = TRUE;
      break;
     
    default:
      assert(FALSE);
   }

 if (cmd_line_option.show_version)
 {
  printf("%s%s", argv[0], " by Nevyn.\n");
  printf("%s%s", " forwards tcp packets from one port to another...",
         " use --help for help.\n");
  exit(EXIT_SUCCESS);
 }

 if (cmd_line_option.show_help)
   usage(EXIT_SUCCESS);

 init_signals();
 
 main_socket.sin_family = AF_INET;
 main_socket.sin_addr.s_addr = htonl(INADDR_ANY);
 main_socket.sin_port = htons(socket_port_local);
 
 {
  struct protoent *the_protocol = getprotobyname("ip");
  main_descriptor = socket(PF_INET, SOCK_STREAM, the_protocol->p_proto);
  
  if (main_descriptor < 0)
    shutdown_error("socket: %d %s\n", errno, strerror(errno));
  
  endprotoent(); /* cleanup */
 }
 
 if (setsockopt(main_descriptor, SOL_SOCKET, SO_REUSEADDR,
                &dummy, sizeof(dummy)) == -1)
   shutdown_error("setsockopt: %d %s\n", errno, strerror(errno));
 
 if (fcntl(main_descriptor, F_SETFL, O_NONBLOCK) == -1)
   shutdown_error("fcntl no-blocking: %d %s.\n", errno, strerror(errno));
 
 if (bind(main_descriptor, (struct sockaddr *) &main_socket,
          sizeof(struct sockaddr_in)) == -1)
   shutdown_error("bind: %d %s\n", errno, strerror(errno));

 if (listen(main_descriptor, 32) == -1)
   shutdown_error("listen: %d %s\n", errno, strerror(errno));

 printf("forward server bound and listening on port %d\n",
        socket_port_local);

 while (TRUE)
 {
  fd_set fset;
  
  FD_ZERO(&fset);  

  FD_SET(main_descriptor, &fset);
  
  switch (select(main_descriptor + 1, &fset, NULL, NULL, NULL))
  {
   case -1:
     switch (errno)
     {
     case EINTR:
       continue;

      default:
        exit (EXIT_FAILURE);
     }

   case 1:
     assert(FD_ISSET(main_descriptor, &fset));
     service_user();
     sleep(1);
     break;
     
   default:
     exit (EXIT_FAILURE);
  }
 }
 
 exit (EXIT_SUCCESS);
}

