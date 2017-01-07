#define BOOT_C
/*
 *  Copyright (C) 1999 James Antill, John Tobin
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
 * email: james@twocan.org, john@twocan.org
 */
#include "main.h"

const char *root_dir = ROOT;


time_t talker_started_on;
int no_of_resis = 0;
system_flags sys_flag = {FALSE, FALSE, FALSE, FALSE, FALSE};


static void setup_priv_lists(player_tree_node *sp,
                             va_list va __attribute__ ((unused)))
{
 assert(sp);
 
 if (PRIV_STAFF(sp))
   player_list_perm_staff_add(sp); /* add to staff list */

 if (sp->priv_spod)
   player_list_spod_add(sp); /* add to spod priv list */
}

void init_priv_lists(void)
{
 do_inorder_all(setup_priv_lists);
}

int main(int argc, char *argv[])
{
 STRACE("boot");
 
#ifndef NDEBUG
 /* if we are debugging show some stats... */
 fprintf(stderr, " Some debug stats...\n");
 fprintf(stderr, "\t player              = %lu\n",
         (unsigned long)sizeof(player));
 fprintf(stderr, "\t player_tree_node    = %lu\n",
         (unsigned long)sizeof(player_tree_node));
 fprintf(stderr, "\t player_linked_list  = %lu\n",
         (unsigned long)sizeof(player_linked_list));
 fprintf(stderr, "\t alias               = %lu\n",
         (unsigned long)sizeof(alias_node));
 fprintf(stderr, "\t alias library       = %lu\n",
         (unsigned long)sizeof(alias_lib_node));
 fprintf(stderr, "\t nickname            = %lu\n",
         (unsigned long)sizeof(nickname_node));
 fprintf(stderr, "\t room                = %lu\n",
         (unsigned long)sizeof(room));
 fprintf(stderr, "\t list entry          = %lu\n",
         (unsigned long)sizeof(list_node));
 fprintf(stderr, "\t command_base        = %lu\n",
         (unsigned long)sizeof(command_base));
 fprintf(stderr, "\t command_node        = %lu\n",
         (unsigned long)sizeof(command_node));
 fprintf(stderr, "\t glob_timer          = %lu\n",
         (unsigned long)sizeof(global_timers));
 fprintf(stderr, "\t input_node          = %lu\n",
         (unsigned long)sizeof(input_node));
 fprintf(stderr, "\t output_node         = %lu\n",
         (unsigned long)sizeof(output_node));
 fprintf(stderr, "\t system_flags        = %lu\n",
         (unsigned long)sizeof(system_flags));
 fflush(stderr);
#endif

 fprintf(stderr, "\n");
  
 talker_started_on = now = time(NULL); 
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " boot...");

 fcntl(0, F_SETFD, 1); /* set close on exec */
 fcntl(1, F_SETFD, 1); /* set close on exec */

 now = time(NULL); /* program boot */
 TIMER_STOP(&glob_timer, shutdown);

 setenv("TZ", "GMT", 1); /* ok the problem is this ....
                          * mktime is defined (in C89) to convert from
                          * something returned by _localtime_
                          * ... this means that if you do...
                          *   new = mktime(gmtime(&old));
                          * then old and new _won't_ be the same ... fuckwits
                          * the setenv hack gets around this "feature".
                          *
                          * Newer specs for mktime (in C9x) define it to know
                          *  whether it's gmtime/localtime
                          * so it works in (glibc-2.x onwards)
                          */
 {
  struct timeval tmp;
  
  if (gettimeofday(&tmp, NULL))
  {
   assert(FALSE);
  }
  
  srand(tmp.tv_usec ^ getpid());
 }
 
 fprintf(stderr, " cmd line...");
 fflush(stderr);
 init_cmd_line(argc, argv);

 fprintf(stderr, "\n");
 
 now = time(NULL); 
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " chdir...");
 fflush(stderr);
 if (chdir(root_dir))
 {
  fprintf(stderr, "\n Error: chdir: %d %s.\n", errno, strerror(errno));
  exit (EXIT_FAILURE);
 }

 fprintf(stderr, " logs...");
 fflush(stderr);
 init_log(); /* make sure we can log to "error" ... */
 
 fprintf(stderr, " timer...");
 fflush(stderr);
 init_timer(); /* must be near the front so other people can use the global
                * timer queue */
 fprintf(stderr, "\n");
  
 now = time(NULL);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));

 fprintf(stderr, " shutdown...");
 fflush(stderr);
 init_shutdown(); /* moves cores etc. */

 fprintf(stderr, " sys_rlimit...");
 fflush(stderr);
 init_sys_rlim();

 fprintf(stderr, " configure...");
 fflush(stderr);
 init_configure();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 
 fprintf(stderr, " log PID...");
 fflush(stderr);
 log_pid("junk/talker_PID", FALSE); /* after configure */
 
 fprintf(stderr, " socket...");
 fflush(stderr);
 init_socket();

 fprintf(stderr, "\n");

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " players...");
 fflush(stderr);
 init_players();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " mail...");
 fflush(stderr);
 init_mail();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " angel...");
 fflush(stderr);
 init_angel();

 fprintf(stderr, "\n");

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " help...");
 fflush(stderr);
 init_help();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " msgs...");
 fflush(stderr);
 msgs_load();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " hangman...");
 fflush(stderr);
 init_hangman();

 fprintf(stderr, "\n");

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " rooms...");
 fflush(stderr);
 init_rooms();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " news...");
 fflush(stderr);
 init_news();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " auth...");
 fflush(stderr);
 init_auth_player();

 fprintf(stderr, "\n");

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " multis...");
 fflush(stderr);
 init_multis();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " spodlist...");
 fflush(stderr);
 init_spodlist();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " channels...");
 fflush(stderr);
 init_channels();

 fprintf(stderr, "\n");

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " dns...");
 fflush(stderr);
 INIT_DNS();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " priv lists...");
 fflush(stderr);
 init_priv_lists();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " stats file...");
 fflush(stderr);
 init_stats_files();
 
 fprintf(stderr, "\n");

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " sys file...");
 fflush(stderr);
 init_system_file();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " signals...");
 fflush(stderr);
 init_signals();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " tips...");
 fflush(stderr);
 init_tip();

 fprintf(stderr, "\n");
 fflush(stderr);
 
 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE)); 
 fprintf(stderr, " text objs...");
 fflush(stderr);
 init_text_objs();

 fprintf(stderr, " player find...");
 fflush(stderr);
 init_player_find();

 fprintf(stderr, " idle...");
 fflush(stderr);
 init_idle();

 fprintf(stderr, "\n");
 fflush(stderr);

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " misc commands...");
 fflush(stderr);
 init_commands();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " cmds...");
 fflush(stderr);
 init_cmds_list();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, " aliases...");
 fflush(stderr);
 init_alias();

 fprintf(stderr, "\n");
 fflush(stderr);

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " lists...");
 fflush(stderr);
 init_list();

 fprintf(stderr, " logon...");
 fflush(stderr);
 init_logon();
 
 fprintf(stderr, " nuke...");
 fflush(stderr);
 init_nuke();

 fprintf(stderr, "\n");
 fflush(stderr);

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " privs...");
 fflush(stderr);
 init_privs();

 fprintf(stderr, " session...");
 fflush(stderr);
 init_session();

 fprintf(stderr, "\n");
 fflush(stderr);

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " player timers...");
 fflush(stderr);
 init_timer_player();

 fprintf(stderr, " quit in...");
 fflush(stderr);
 init_quit_in();

 fprintf(stderr, " alarm...");
 fflush(stderr);
 init_alarm();

 fprintf(stderr, "\n");
 fflush(stderr);

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "%s - Initialising:", disp_time_std(now, 0, TRUE, TRUE));
 fprintf(stderr, " local...");
 fflush(stderr);
 init_local_talker();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 INTERCOM_INIT();

 gettimeofday(&now_timeval, NULL); now = time(NULL);
 timer_q_run_norm(&now_timeval);
 fprintf(stderr, "\n Finnished boot.\n");
 fflush(stderr);
 
 nice(configure.sys_nice_value);

 /* email the relevant ppls about boot */
 {
  char subj[100] = {0};
  email_info ei = EMAIL_INFO_INIT();

  sprintf(subj, "%s - Talker booted   (%s)",
          configure.name_abbr_upper, disp_time_std(now, 0, TRUE, TRUE));

  ei.to = configure.email_to_up_down;
  ei.reply_to = configure.email_to_admin;
  ei.subject = subj;
  ei.body = ("Version: " VERSION "\n"
             "Package: " TALKER_CODE_SNAPSHOT "\n"
             "\n\nTalker has booted.\n");

  email_generic(&ei);
 }
 
 while (!sys_flag.shutdown)
 {  
  if (floor(difftime(time(NULL), now)))
    timer_run_do();

  if (sys_flag.backups_run)
  {
   STRACE("backups_run");
   backups_run();
  }

  if (floor(difftime(time(NULL), now)))
    timer_run_do();

  if (sys_flag.reload_run)
  {
   STRACE("reload_run");
   MSGS_RELOAD();
   sys_flag.reload_run = FALSE;
  }

  if (floor(difftime(time(NULL), now)))
    timer_run_do();

  STRACE("socket_update_indicators");
  if (TIMER_IS_ACTIVE(&glob_timer, shutdown))
    socket_update_indicators(SCHEDULE_POLL_SHUTDOWN_TIMEOUT);
  else
    socket_update_indicators(SCHEDULE_POLL_TIMEOUT);

  if (floor(difftime(time(NULL), now)))
    timer_run_do();

  STRACE("socket_main");
  socket_main_socket();

  if (floor(difftime(time(NULL), now)))
    timer_run_do();
  
  STRACE("player events");
  do_order_misc_on_all(player_event_do,  player_list_cron_start());
 }

 shutdown_exit();

 DNS_DELETE();
 
 exit (EXIT_SUCCESS);
}

