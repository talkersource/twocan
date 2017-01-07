#define SHUTDOWN_C
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

char shutdown_player[PLAYER_S_NAME_SZ] = "";
char shutdown_msg[SHUTDOWN_MSG_SZ] = "";

player *current_player = NULL;


static int last_shutdown_time = 0;


static void user_shutdown(player *p, const char *str)
{
 int wt_err;
 unsigned long tmp;
 parameter_holder params;

 get_parameter_init(&params);

 if (!p->saved->priv_lower_admin || !get_parameter_parse(&params, &str, 1))
 {
  char buf[256];
  
  if (TIMER_IS_ACTIVE(&glob_timer, shutdown))
  {
   ptell_mid(NORMAL_T(p), "shutdown", FALSE);
   fvtell_player(NORMAL_T(p), "Started by: %s\n", shutdown_player);
   fvtell_player(NORMAL_T(p), "Reason: %s\n", shutdown_msg);
   fvtell_player(NORMAL_T(p), "Time togo: %s\n",
                 word_time_long(buf, sizeof(buf),
                                TIMER_TOGO(&glob_timer, shutdown),
                                WORD_TIME_DEFAULT));
   fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
  }
  else
    fvtell_player(NORMAL_T(p), "%s", " There isn't a shutdown going on.\n");
  return;
 }
 
 CHECK_DUTY(p);
   
 if (!*str)
 {
#ifndef NDEBUG
  fvtell_player(NORMAL_T(p), "%s",
                " -=> Okay I'll let you, seeing you're debugging stuff.\n");
  CONST_COPY_STR_LEN(shutdown_msg, "** Shutting down due to debugging **");
#else
  fvtell_player(NORMAL_T(p), " You must supply a shutdown reason first.\n");
  TELL_FORMAT(p, "<countdown> <reason>");
#endif   
 }
 else
   COPY_STR(shutdown_msg, str, SHUTDOWN_MSG_SZ);

 qstrcpy(shutdown_player, p->saved->lower_name);
 
 tmp = word_time_parse(GET_PARAMETER_STR(&params, 1),
                       WORD_TIME_PARSE_ERRORS, &wt_err);
 if (wt_err)
   TELL_FORMAT(p, "<countdown> <reason>");

 if (!tmp)
 { /* shutdown NOW */   
  last_shutdown_time = 1; /* don't output the shutdown in 1 second msg */
  
  TIMER_START(&glob_timer, shutdown, 1);
 }
 else
 {
  char buf[256];
  
  TIMER_START(&glob_timer, shutdown, tmp);
  
  fvtell_player(NORMAL_T(p),
                " -=> Program set to shutdown in ^S^B%s^s.\n",
                word_time_long(buf, sizeof(buf),
                               TIMER_TOGO(&glob_timer, shutdown),
                               WORD_TIME_DEFAULT));
 }
}

static void user_su_abort_shutdown(player *p)
{  
 if (TIMER_IS_ACTIVE(&glob_timer, shutdown))
 {
  last_shutdown_time = 0;
  shutdown_player[0] = 0;
  shutdown_msg[0] = 0;
  
  if (TIMER_TOGO(&glob_timer, shutdown) < MK_MINUTES(5))
    sys_wall(0, "%s", "\n\n -=> Shutdown aborted.\n\n");
  else
    fvtell_player(NORMAL_T(p), "%s", " Shutdown Aborted.\n");
  
  TIMER_STOP(&glob_timer, shutdown);
 }
 else
   fvtell_player(NORMAL_T(p), "%s", " No shutdown in progress.\n");
}

void shutdown_do_timer(void)
{
 if (TIMER_IS_ACTIVE(&glob_timer, shutdown))
 {
  int shutdown_time = TIMER_TOGO((&glob_timer), shutdown);
  
  BTRACE("shutdown_do_timer");
  
  if (last_shutdown_time < shutdown_time)
    last_shutdown_time = shutdown_time + 1;
  
  if ((last_shutdown_time > MK_HOURS(1)) && (shutdown_time <= MK_HOURS(1)))
  {
   sys_wall(HILIGHT, "%s", "\n\n -=> We'll be rebooting in 1 hour <=-\n\n");
   last_shutdown_time = MK_HOURS(1);
  }
  
  if ((last_shutdown_time > MK_MINUTES(2)) &&
      (shutdown_time <= MK_MINUTES(15)))
  {
   if ((last_shutdown_time > MK_MINUTES(15)) &&
       (shutdown_time <= MK_MINUTES(15)))
     sys_wall(HILIGHT, "\n\n -=> We'll be rebooting in %d minutes <=-\n\n",
              (last_shutdown_time = MK_MINUTES(15)) / 60);
   
   if ((last_shutdown_time > MK_MINUTES(10)) &&
       (shutdown_time <= MK_MINUTES(10)))
     sys_wall(HILIGHT, "\n\n -=> We'll be rebooting in %d minutes <=-\n\n",
              (last_shutdown_time = MK_MINUTES(10)) / 60);
   
   if ((last_shutdown_time > MK_MINUTES(5)) &&
       (shutdown_time <= MK_MINUTES(5)))
     sys_wall(HILIGHT, "\n\n -=> We'll be rebooting in %d minutes <=-\n\n",
              (last_shutdown_time = MK_MINUTES(5)) / 60);
   
   if ((last_shutdown_time > MK_MINUTES(4)) &&
       (shutdown_time <= MK_MINUTES(4)))
     sys_wall(HILIGHT, "\n\n -=> We'll be rebooting in %d minutes <=-\n\n",
              (last_shutdown_time = MK_MINUTES(4)) / 60);
   
   if ((last_shutdown_time > MK_MINUTES(3)) &&
       (shutdown_time <= MK_MINUTES(3)))
     sys_wall(HILIGHT, "\n\n -=> We'll be rebooting in %d minutes <=-\n\n",
              (last_shutdown_time = MK_MINUTES(3)) / 60);
   
   if ((last_shutdown_time > MK_MINUTES(2)) &&
       (shutdown_time <= MK_MINUTES(2)))
     sys_wall(HILIGHT, "\n\n -=> We'll be rebooting in %d minutes <=-\n\n",
              (last_shutdown_time = MK_MINUTES(2)) / 60);
  }
  
  if ((last_shutdown_time > MK_MINUTES(1)) &&
      (shutdown_time <= MK_MINUTES(1)))
  {
   sys_wall(HILIGHT, "%s", "\n\n -=> 1 minute left to the reboot <=-\n\n");
   last_shutdown_time = MK_MINUTES(1);
  }
  
  if ((last_shutdown_time > 2) && (shutdown_time <= 30))
  {
   if ((last_shutdown_time > 30) && (shutdown_time <= 30))
     sys_wall(HILIGHT, "\n\n -=> %d seconds left until the reboot <=-\n\n",
              (last_shutdown_time = 30));
   
   if ((last_shutdown_time > 15) && (shutdown_time <= 15))
     sys_wall(HILIGHT, "\n\n -=> %d seconds left until the reboot <=-\n\n",
              (last_shutdown_time = 15));

   if ((last_shutdown_time > 10) && (shutdown_time <= 10))
     sys_wall(HILIGHT, "\n\n -=> %d seconds left until the reboot <=-\n\n",
               (last_shutdown_time = 10));
   
   if ((last_shutdown_time > 5) && (shutdown_time <= 5))
     sys_wall(HILIGHT, "\n\n -=> %d seconds left until the reboot <=-\n\n",
              (last_shutdown_time = 5));

   if ((last_shutdown_time > 4) && (shutdown_time <= 4))
     sys_wall(HILIGHT, "\n\n -=> %d seconds left until the reboot <=-\n\n",
               (last_shutdown_time = 4));

   if ((last_shutdown_time > 3) && (shutdown_time <= 3))
     sys_wall(HILIGHT, "\n\n -=> %d seconds left until the reboot <=-\n\n",
              (last_shutdown_time = 3));
   
   if ((last_shutdown_time > 2) && (shutdown_time <= 2))
     sys_wall(HILIGHT, "\n\n -=> %d seconds left until the reboot <=-\n\n",
              (last_shutdown_time = 2));
  }

  if ((last_shutdown_time > 1) && (shutdown_time <= 1))
  {
   sys_wall(HILIGHT, "%s", "\n\n -=> 1 second left until the reboot <=-\n\n");
   last_shutdown_time = 1;
  }
  
  if (TIMER_IS_DONE(&glob_timer, shutdown))
    sys_flag.shutdown = TRUE;
 }
}

void shutdown_error(const char *error_msg, ...)
{
 va_list ap;
 FILE *log_err_msg = NULL;
 char buf[256];
 
 if (sys_flag.panic)
 {
  if (!sys_flag.bad_panic)
  {
   sys_flag.bad_panic = TRUE;
   vwlog("error", "** PANIC **\n");
  }
  
  abort();

  while (1)
    exit(EXIT_FAILURE);
 }
 sys_flag.panic = TRUE;

 va_start(ap, error_msg);
 if ((log_err_msg = open_wlog("error")))
 {
  vfprintf(log_err_msg, error_msg, ap);  
  close_wlog(log_err_msg);
 }
 va_end(ap);
 
 wlog("dump", "************* Starting dump");
  
 DUMP_CTRACE();
 DUMP_ICTRACE();
 DUMP_TCTRACE();
 DUMP_BTRACE();
 DUMP_STRACE();

 vwlog("dump", "Uptime = %s\n",
       word_time_long(buf, sizeof(buf),
                      difftime(now, talker_started_on), WORD_TIME_DEFAULT));
 
 if (current_command)
   vwlog("dump", "%s%s", "current_command = ", current_command);

 if (cmds_last_ran)
 {
  vwlog("dump", "cmds_last_ran.name = %s", cmds_last_ran->name);
  
  switch (cmds_last_ran->func.type)
  {
   case CMDS_PARAM_NOTHING:
     vwlog("dump", "cmds_last_ran.func_type = nothing");
     break;
   case CMDS_PARAM_CONST_CHARS:
     vwlog("dump", "cmds_last_ran.func_type = const chars");
     vwlog("dump", "cmds_last_ran.func = %p",
           cmds_last_ran->func.func.player_and_const_chars);
     break;
   case CMDS_PARAM_CHARS_SIZE_T:
     vwlog("dump", "cmds_last_ran.func_type = chars size_t");
     vwlog("dump", "cmds_last_ran.func = %p",
           cmds_last_ran->func.func.player_and_chars_and_length);
     break;
   case CMDS_PARAM_RET_CHARS_SIZE_T:
     vwlog("dump", "cmds_last_ran.func_type = ret chars size_t");
     vwlog("dump", "cmds_last_ran.func = %p",
           cmds_last_ran->func.func.player_ret_and_chars_and_length);
     break;
   case CMDS_PARAM_NO_CHARS:
     vwlog("dump", "cmds_last_ran.func_type = no chars");
     vwlog("dump", "cmds_last_ran.func = %p",
           cmds_last_ran->func.func.player_only);
     break;
   case CMDS_PARAM_PARSE_PARAMS:
     vwlog("dump", "cmds_last_ran.func_type = parsed params");
     vwlog("dump", "cmds_last_ran.func = %p",
           cmds_last_ran->func.func.player_and_parsed_params);
     break;
     
   default:
     vwlog("dump", "cmds_last_ran.func_type = UNKNOWN");
     assert(FALSE);  
  }
 }
 
 if (current_player)
 {
  if (current_player->saved)
  {
   vwlog("dump", "player %s", current_player->saved->name);
   if (current_player->location)
     vwlog("dump", "in %s.%s", current_player->location->owner ?
           current_player->location->owner->lower_name : "(nothing)",
           current_player->location->id);
   else
     wlog("dump", "No room of current player");
  }
  else
    wlog("dump", "Current player, but not logged on properly.");
  
  if (current_player->input_start && current_player->input_start->ready)
    vwlog("dump", "input = %s\n", current_player->input_start->input);
 }
 else
   wlog("dump", "No current player");
 
 vwlog("dump", "players %d", current_players);
  
 wlog("dump", "------------- End dump");

 sys_wall(0, "%s",
          "\n^N\n" /* again the ^N is a hack */
          "$Merge(1($Line_fill( ))"
          "2(^B*WIBBLE*^N Something bad has happened.))\n"
          "$Merge(1($Line_fill( )) 2(Trying to save files.))\n"
          "$Bell\n\n");

 stats_close_file(STATS_CRASH);
 CONST_COPY_STR_LEN(system_data.shutdown, "An error occured in the program.");

 shutdown_exit();

 abort();
 
 while (1)
   exit (EXIT_FAILURE);
}

static int internal_shutdown_exit(player *scan,
                                  va_list ap __attribute__ ((unused)))
{
 assert(scan == scan->saved->player_ptr);
 if (scan->saved->priv_base)
   logoff_player_update(scan);
 
 return (TRUE);
}

static int internal_shutdown_email(FILE *fp, void *passed_str)
{
 const char *str = passed_str;
 int len = strlen(str);
 int count = 0;
 int ret = 0;
 
 ret += fprintf(fp, "Version: %s\n", VERSION);
 ret += fprintf(fp, "Package: %s\n", TALKER_CODE_SNAPSHOT);

 ret += fprintf(fp, "%s", "\n");
 ret += fprintf(fp, "Time: %s (GMT)\n", disp_time_std(now, 0, TRUE, TRUE));
 ret += fprintf(fp, "Player: %s\n", shutdown_player);
 ret += fprintf(fp, "%s", "\n\n");
 
 if (shutdown_player[0])
   ret += fprintf(fp, "%s", " ---------- Shutdown ---------- \n");
 else
   ret += fprintf(fp, "%s", " ---------- Crashed  ---------- \n");

 while (count < len)
 {
  ret += fprintf(fp, " %.*s\n", 74, str + count);
  count += 74;
 }
 ret += fprintf(fp, "%s", "\n");

 return (ret);
}

void shutdown_exit(void)
{
 sys_wall(0, "%s", "\n$Bell\n");

 { /* email the relevant ppls about close */
  char subj[256];
  email_info ei = EMAIL_INFO_INIT();
  
  ei.to = configure.email_to_up_down;
  ei.reply_to = configure.email_to_admin;
  ei.subject = subj;
  ei.func = internal_shutdown_email;
  ei.param = (char *)shutdown_msg; /* warning */
  
  if (!sys_flag.panic)
    sprintf(subj, "%s - Talker shutdown (%s)",
            configure.name_abbr_upper, disp_time_std(now, 0, TRUE, TRUE));
  else
    sprintf(subj, "%s - Talker ** CRASHED ** (%s)",
            configure.name_abbr_upper, disp_time_std(now, 0, TRUE, TRUE));

  email_generic(&ei);
 }

 if (!sys_flag.panic)
 {
  sprintf(system_data.shutdown, "Admin re-boot - %.*s",
          (int)sizeof(system_data.shutdown), shutdown_msg);
  
  sys_wall(RAW_OUTPUT | HILIGHT, " -=> %s\n", shutdown_msg);
  vwlog("shutdown", "%s\n%s\n", shutdown_player, shutdown_msg);
  
  stats_close_file(STATS_SHUT);
 }

 sys_wall(0, "%s", "\n\n"
          "$Merge(1($Line_fill( ))"
          "2(-==> Program shutting down ^BNOW^N <==-))"
          "\n\n\n\n");

 socket_all_players_output();
 
 do_inorder_logged_on(internal_shutdown_exit);

 timer_exec_run_do();
 
 socket_all_players_output();

 system_save_file();

 socket_all_players_output();
 
 socket_close_all(); /* also does another output_for_player */

 if (!configure.talker_read_only)
   remove("junk/talker_PID");
}

void init_shutdown(void)
{
 struct stat buf;
 
 if (stat("core", &buf) != -1)
 {
  const char *tmp = disp_time_filename(buf.st_ctime, "cores/", ".core");

  if (stat(tmp, &buf) != -1)
     vwlog("error", "Error: double core: %s\n", tmp);
  else if (rename("core", tmp) == -1)
    vwlog("error", "core: rename: %d %s\n", errno, strerror(errno));
 }
}

#ifndef NDEBUG
static void user_shutdown_crash(player *p)
{
 char *tmp = NULL;


 *tmp = 0; /* So we can easily make sure the crashj logging routines work */

 
 fvtell_player(NORMAL_T(p), "%s", " It didn't work.n");
}
#endif

void cmds_init_shutdown(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("abort", user_su_abort_shutdown, NO_CHARS, ADMIN);
 CMDS_PRIV(coder_admin);
 CMDS_ADD("shutdown", user_shutdown, CONST_CHARS, SU);
 CMDS_FLAG(no_expand);

#ifndef NDEBUG
 CMDS_ADD("shutdown_crash", user_shutdown_crash, NO_CHARS, ADMIN);
 CMDS_PRIV(coder_admin);
 CMDS_FLAG(no_expand);
#endif
}
