#define ANGEL_C

#include "no_stack_here.h"

#include "main.h"

# define S_IGNAL_SETUP(x)  do { \
 if (sigaction(x, &sa, NULL) == -1) \
 vwlog("angel", " Couldn't set %s\n", #x); } while (FALSE)
#define SHOW_WHY_DIED 1

static char talker_ps_name[sizeof("-=> %s <=- Talker Server") +
                          CONFIGURE_NAME_SZ];

time_t now;

static child_com *child_node = NULL;

static int angel_param_dont_exec = FALSE;
static char *angel_param_root_dir = NULL;
static int angel_param_using_port = 0;
static int angel_param_read_only = FALSE;
static int angel_param_verbose = FALSE;
static int angel_param_configure_file_name = FALSE;

void shutdown_error(const char *str, ...)
{
 FILE *fp = wlog_open("angel");
 va_list ap;

 va_start(ap, str);
 fprintf(fp, "%s", " ** Error: ");
 vfprintf(fp, str, ap);
 va_end(ap);

 wlog_close(fp);
 
 remove("junk/angel_PID");
 
 exit (EXIT_FAILURE);
}

static void passon_signal(int num)
{
 if (child_node)
   kill(child_node->pid, num);
}

static void die_signal(int num)
{
 vwlog("angel", " signal: %d ... exiting.\n", num);
 remove("junk/angel_PID");
 exit (EXIT_FAILURE);
}

static void init_signals(void)
{
#ifndef CONFIG_NO_SIGACTION
 struct sigaction sa;

 sigemptyset(&sa.sa_mask);
 sa.sa_flags = SA_RESTART; /* restart a system call if it interupts */
 sa.sa_flags |= SA_NOCLDSTOP; /* use SA_RESTART as well */
 sa.sa_handler = SIG_IGN;

 S_IGNAL_SETUP(SIGCHLD);
 S_IGNAL_SETUP(SIGUSR1);
 S_IGNAL_SETUP(SIGUSR2);
 S_IGNAL_SETUP(SIGALRM);
 
 sa.sa_flags = 0;
 S_IGNAL_SETUP(SIGPIPE);

 sa.sa_handler = passon_signal;
 S_IGNAL_SETUP(SIGHUP);

 sa.sa_handler = die_signal;
 S_IGNAL_SETUP(SIGQUIT);
 S_IGNAL_SETUP(SIGILL);
 S_IGNAL_SETUP(SIGFPE);
 S_IGNAL_SETUP(SIGBUS);
 S_IGNAL_SETUP(SIGSEGV);
 S_IGNAL_SETUP(SIGTERM);
 S_IGNAL_SETUP(SIGXFSZ);
 
# ifdef SIGSYS
 S_IGNAL_SETUP(SIGSYS);
# endif
#else
# error "Get a decent OS http://www.redhat.com/"
#endif
}

static void init_cmd_line(int argc, char *argv[])
{
  char optchar = 0;
  const char *program_name = "angel";
  struct option long_options[] =
  {
   {"angel-internal-switch", no_argument, NULL, 1},
   {"configure", required_argument, NULL, 'C'},
   {"help", no_argument, NULL, 'h'},
   {"port", required_argument, NULL, 'p'},
   {"read-only", optional_argument, NULL, 'R'},
   {"root-dir", required_argument, NULL, 'r'},
   {"verbose", no_argument, NULL, 'd'},   
   {"version", no_argument, NULL, 'V'},   
   {NULL, 0, NULL, 0}
  };

  if (argv[0])
  {
   if ((program_name = C_strrchr(argv[0], '/')))
     ++program_name;
   else
     program_name = argv[0];
  }
  
  while ((optchar = getopt_long(argc, argv, "c:C:dhp:r:vHR:V",
                                long_options, NULL)) != EOF)
    switch (optchar)
    {
     case 1:
       angel_param_dont_exec = TRUE;
       break;
       
     case '?':
       fprintf(stderr, " That option is not valid.\n");
     case 'H':
     case 'h':
       printf(" Format: %s [-dhpvHPV]\n"
              " --help -h         - Print this message.\n"
              " --port -p         - Change the port number.\n"
              " --read-only -R    - Make program never do write operations.\n"
              " --root-dir -r     - Change the root dir.\n"
              " --verbose -d      - Print some debuging stuff.\n"
              " --version -v      - Print the version string.\n",
              program_name);
       if (optchar == '?')
         exit (EXIT_FAILURE);
       else
         exit (EXIT_SUCCESS);
       
     case 'v':
     case 'V':
       printf(" %s is version %s, package version %s.\n",
              program_name, VERSION, TALKER_CODE_SNAPSHOT);
       exit (EXIT_SUCCESS);

     case 'p':
       angel_param_using_port = atoi(optarg);
       break;

     case 'R':
       if (optarg && TOGGLE_MATCH_ON(optarg))
         angel_param_read_only = TRUE;
       else if (optarg && TOGGLE_MATCH_OFF(optarg))
         angel_param_read_only = FALSE;
       else if (!optarg || TOGGLE_MATCH_TOGGLE(optarg))
         angel_param_read_only = !angel_param_read_only;
       else
       {
        printf(" %s --read-only[=on|off|toggle]\n", program_name);
        exit (EXIT_FAILURE);
       }
       break;

       break;
       
     case 'r':
       angel_param_root_dir = optarg;
       break;
       
     case 'd':
       angel_param_verbose = TRUE;
       break;

     case 'c':
     case 'C':
       angel_param_configure_file_name = TRUE;
       COPY_STR(configure.configure_file_name, optarg, CONFIGURE_FILE_NAME_SZ);
       break;
    }
}


int main(int argc, char *argv[])
{
#ifdef SHOW_WHY_DIED
 int status = 0;
#endif
 int crashes = 0;
 int total_boots = 0;
 time_t booted_on;
 time_t angel_started_on = time(NULL);

 init_cmd_line(argc, argv);
 
 if (angel_param_root_dir)
 {
  if (chdir(angel_param_root_dir) == -1)
    shutdown_error(" Error: chdir: %s\n", angel_param_root_dir);
 }
 else
 {
  if (chdir(ROOT) == -1)
    shutdown_error(" Error: chdir: %s\n", ROOT);
 }

 init_configure();
 
 if (!angel_param_dont_exec)
 {
  char tmp1[sizeof("-=> %s <=- Gardian Angel") + CONFIGURE_NAME_SZ];
  char tmp2[] = "--angel-internal-switch";
  char tmp3[] = "--root-dir";
  char tmp4[] = "--port";
  char tmp5[] = "--read-only";
  char tmp6[] = "--verbose";
  char tmp7[] = "--configure";
  char buffer[BUF_NUM_TYPE_SZ(int)];
  char *angel_argv[10];
  int count = 1;

  sprintf(tmp1, "-=> %s <=- Gardian Angel", configure.name_long);
  
  angel_argv[0] = tmp1;
  angel_argv[1] = tmp2;

  if (angel_param_root_dir)
  {
   angel_argv[++count] = tmp3;
   angel_argv[++count] = angel_param_root_dir;
  }
   
  if (angel_param_using_port)
  {
   sprintf(buffer, "%d", angel_param_using_port);
   angel_argv[++count] = tmp4;
   angel_argv[++count] = buffer;
  }

  if (angel_param_read_only)
    angel_argv[++count] = tmp5;

  if (angel_param_verbose)
    angel_argv[++count] = tmp6;

  if (angel_param_configure_file_name)
  {
   angel_argv[++count] = tmp7;
   angel_argv[++count] = configure.configure_file_name;
  }

  angel_argv[++count] = NULL;
  
  execvp("bin/angel", angel_argv);
  
  shutdown_error("exec: %d %s\n", errno, strerror(errno));
 }

 log_pid("junk/angel_PID", TRUE);
 
 init_signals();

 booted_on = now = time(NULL);
 while (TRUE)
 {
  char *child_argv[7];
  ssize_t tmp = 0;
  int did_boot = FALSE;
  char tmp3[] = "--port";
  char tmp4[] = "--read-only";
  char tmp5[] = "--verbose";
  char tmp6[] = "--configure";
  char buffer[BUF_NUM_TYPE_SZ(int)];
  int count = 0;

  sprintf(talker_ps_name, "-=> %s <=- Talker Server", configure.name_long);
  child_argv[0] = talker_ps_name;
  
  if (angel_param_using_port)
  {
   sprintf(buffer, "%d", angel_param_using_port);
   child_argv[++count] = tmp3;
   child_argv[++count] = buffer;
  }
  
  if (angel_param_read_only)
    child_argv[++count] = tmp4;
  
  if (angel_param_verbose)
    child_argv[++count] = tmp5;

  if (angel_param_configure_file_name)
  {
   child_argv[++count] = tmp6;
   child_argv[++count] = configure.configure_file_name;
  }

  child_argv[++count] = NULL;
  
  now = time(NULL);
  
  if (total_boots && ((crashes > ANGEL_MAX_REBOOT_LIMIT) ||
                      (difftime(now, booted_on) < ANGEL_MIN_TIME_HALT)))
  {
   if (crashes > ANGEL_MAX_REBOOT_LIMIT)
     vwlog("angel", " Hit Max reboot limit. Stopping.\n");
   else
     vwlog("angel", " Hit Min reboot time. Stopping.\n");

   remove("junk/angel_PID");
   exit (EXIT_FAILURE);
  }
  else if (difftime(now, booted_on) > ANGEL_MAX_TIME_HALT)
  {
   booted_on = time(NULL);
   crashes = 0;
  }

  ++total_boots;
  ++crashes;
  
  wlog("angel", "Forking to boot server");
  
  if (!(child_node = child_com_create("bin/talker", child_argv)))
    shutdown_error("%s", "Failed to fork()");

  CHILD_COM_POLL(child_node, socket_poll_add(child_node->input));
  
  child_com_send(child_node, "total_boots:%d\n", total_boots);
  child_com_send(child_node, "angel_started:%s\n",
                 disp_time_cmp_string(angel_started_on));
  
  while (child_com_flush(child_node))
  {
   ssize_t tmp = child_com_recv(child_node, NULL, 512);

   if (child_node->bad ||
       child_com_waiting_input(child_node, ANGEL_TIMEOUT_BOOT))
     break;
   
   socket_poll_update_all(1000);

   if (tmp)
     did_boot = TRUE;
   
   if (child_node->bad)
     break;
   
   child_com_send(child_node, "a");
   
   now = time(NULL);
  }

  if (did_boot)
  {
   while (child_com_flush(child_node) &&
          !child_com_waiting_input(child_node, MK_MINUTES(1)))
   {
    socket_poll_update_all(1000);
    
    child_com_recv(child_node, NULL, 512);
    
    if (child_node->bad)
      break;
    
    child_com_send(child_node, "a");
    
    now = time(NULL);
   }
  }
  else
  { /* didn't boot before we gave up on it */
   vwlog("angel", " Didn't boot. Stopping.\n");
   
   remove("junk/angel_PID");
   exit (EXIT_FAILURE);
  }
  
  if (TRUE)
  {
   pid_t tmp_pid = child_node->pid;
   socket_poll_del(child_node->io_indicator);
   child_com_delete(child_node);
   child_node = NULL;
   
#ifdef SHOW_WHY_DIED
   waitpid(tmp_pid, &status, WNOHANG);
   
   if (WIFEXITED(status))
     wlog("angel", "Server exited safely.");
   else if (WIFSIGNALED(status))
     vwlog("angel", "Server stopped due to signal %d.\n",
           WTERMSIG(status));  
   else
     vwlog("angel", "Server exited with code %d.\n",
           WEXITSTATUS(status));
#endif
  }
 }
 
 assert(FALSE);
 remove("junk/angel_PID");
 exit (EXIT_FAILURE);
}

