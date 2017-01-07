#define INTERCOM_SIGNALS_C
/*
 *  Copyright (C) 1999 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@and.org
 */
#include "main.h"


static void internal_signal_big_crash(S_IGNAL_CRASH_PARAMS(s_ig_num, con))
{
 if (!intercom_panic)
 {
  S_IGNAL_CRASH_INFO("intercom-address");
 }

 shutdown_error("Crash signal received (%d) -- intercom-address.log -- might help.\n",
                s_ig_num);
}

static void internal_signal_small_crash(int s_ig_num)
{
 shutdown_error("Crash signal received (%d).\n", s_ig_num);
}

static void internal_signal_reboot(int s_ig_num)
{
 IGNORE_PARAMETER(s_ig_num);
 
 intercom_reboot = TRUE;
}

void init_signals(void)
{
 struct sigaction sa;

 if (sigemptyset(&sa.sa_mask) == -1)
   vwlog("intercom", "sigemptyset: %d %s.\n", errno, strerror(errno));

 sa.sa_flags = SA_RESTART; /* restart a system call if it interupts */

 /* external signals... */
 sa.sa_handler = internal_signal_reboot;
 S_IGNAL_SETUP(SIGUSR1);
 S_IGNAL_SETUP(SIGHUP);
 
 /* ignore it... we don't have a use for it */
 sa.sa_handler = SIG_IGN;

 S_IGNAL_SETUP(SIGALRM);
 
 sa.sa_flags |= SA_NOCLDSTOP; /* use SA_RESTART as well */

 S_IGNAL_SETUP(SIGCHLD);
 
 /* don't use SA_RESTART ... */
 sa.sa_flags = 0;
 S_IGNAL_SETUP(SIGPIPE);

 sa.sa_flags = SA_NOMASK; /* allow us to get two at once */

#ifdef HAVE_SIGALTSTACK
 do
 {
  struct sigaltstack alt_stack;
  static char buffer[SIGSTKSZ];
  
  alt_stack.ss_size = SIGSTKSZ;
  alt_stack.ss_sp = buffer;
  alt_stack.ss_flags = 0;

  if (sigaltstack(&alt_stack, NULL) == -1)
  { /* this should be impossible */
   vwlog("intercom", "sigaltstack error: %d %s\n", errno, strerror(errno));
   break;
  }

  sa.sa_flags |= SA_ONSTACK; /* help the debugger out */
  
 } while (FALSE);
#endif
 sa.sa_handler = internal_signal_small_crash;

 S_IGNAL_SETUP(SIGQUIT);

 /* bad things have happened */ 
 sa.sa_handler = (void (*) (int)) internal_signal_big_crash;

 S_IGNAL_SETUP(SIGSEGV);
 S_IGNAL_SETUP(SIGBUS);
 S_IGNAL_SETUP(SIGILL);
 S_IGNAL_SETUP(SIGFPE);
 S_IGNAL_SETUP(SIGTERM);
 S_IGNAL_SETUP(SIGXFSZ);
 
#ifdef SIGSYS
 S_IGNAL_SETUP(SIGSYS);
#endif
}
