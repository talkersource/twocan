#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/fcntl.h>

#include <timer_q.h>

#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE 1
#endif

#ifndef __GNUC__
# define __PRETTY_FUNCTION__ "(unknown)"
#endif

#define STR(x) #x
#define XSTR(x) STR(x)

#define SHOW(x, y, z) \
 "File: " x "\n" \
 "Function: " y "\n" \
 "Line: " XSTR(z) "\n" \
 "Problem: "

#define PROBLEM(x) problem(SHOW(__FILE__, __PRETTY_FUNCTION__, __LINE__) x)

static void problem(const char *str)
{
 int saved_errno = errno;

 fprintf(stderr, "%s", str);
 
 if (str[strlen(str) - 1] == ':')
   fprintf(stderr, " %d %s", saved_errno, strerror(saved_errno));
 fprintf(stderr, "%s", "\n");

 exit (EXIT_FAILURE);
}

/* real timer code from this point onwards... */

static void timer_func1(int type, void *data)
{
 if (type == TIMER_Q_TYPE_CALL_DEL)
   return;

 fprintf(stdout, " Data is -- %s -- \n", (char *)data);
 fflush(NULL);
}

int main(void)
{
 Timer_q_base *base = NULL;
 Timer_q_node *node = NULL;
 struct timeval s_tv;
 const struct timeval *tv = NULL;

 gettimeofday(&s_tv, NULL);
 
 base = timer_q_add_base(timer_func1, TIMER_Q_FLAG_BASE_DEFAULT);
 if (!base)
   PROBLEM("Mem err");
 
 TIMER_Q_TIMEVAL_ADD_SECS(&s_tv, 2, 0);
 node = timer_q_add_node(base, (char *)"data 1", &s_tv,
                         TIMER_Q_FLAG_NODE_DEFAULT);
 if (!node)
   PROBLEM("Mem err");

 while ((tv = timer_q_first_timeval()))
 {
  long wait_period = 0;

  gettimeofday(&s_tv, NULL);

  wait_period = timer_q_timeval_diff_usecs(tv, &s_tv);
  if (wait_period > 0)
    usleep(wait_period); /* this is probably going to be select()/poll() IRL */
  else
    timer_q_run_norm(&s_tv);
 }
 
 exit (EXIT_SUCCESS);
}
