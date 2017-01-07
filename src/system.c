#define SYSTEM_C
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


/* system things */
sys_info system_data;

static Timer_q_func_node stats_timer_node;

/* functions... */

static void reset_system_file(void)
{
 /* Date */
 system_data.date_stamp = now;
 /* General (ever) */
 system_data.total_ever_logons = total_logons;
 CONST_COPY_STR_LEN(system_data.shutdown, "Unknown.");
 system_data.max_time_logged = difftime(now, talker_started_on);
 /* Maximums */
 system_data.max_logons = total_logons;
 system_data.max_l_timeup = difftime(now, talker_started_on);
 system_data.max_uniq_logons = total_uniq_logons;
 system_data.max_nl_timeup = difftime(now, talker_started_on);
 system_data.max_people = current_players;
 system_data.max_time = difftime(now, talker_started_on);
 system_data.longest_spod_name[0] = 0;
 system_data.longest_spod_time = 0;

 system_save_file();
}

static void user_su_system_save_file_verb(player *p)
{
 fvtell_player(SYSTEM_T(p), "%s", " Saving system file...\n");
 system_save_file();
}

static void user_su_system_change_last_shutdown(player *p, const char *str)
{
 if (!*str)
   TELL_FORMAT(p, "<reason>");
 else
 {
  COPY_STR(system_data.shutdown, str, SHUTDOWN_MSG_SZ);
  
  fvtell_player(NORMAL_T(p), 
                " All done. Reason set to:\n %s\n", system_data.shutdown);
 }
 
 system_save_file();
}

static void update_save_stats(void)
{
 int time_up = difftime(now, talker_started_on);
 
 /* Setup the info where possible */  
 if (time_up > system_data.max_time)
   system_data.max_time = time_up;
 
 if (total_logons > system_data.max_logons)
 {
  system_data.max_logons = total_logons;
  system_data.max_l_timeup = time_up;
 }
 
 if (total_uniq_logons > system_data.max_uniq_logons)
 {
  system_data.max_uniq_logons = total_uniq_logons;
  system_data.max_nl_timeup = time_up;
 }
}

void system_save_file(void)
{
 file_io real_io_system;
 file_io *io_system = &real_io_system;

 if (configure.talker_read_only)
   return;
 
 update_save_stats();
 
 if (file_write_open("files/sys/stats.tmp",
                     SYSTEM_STATS_FILE_VERSION, io_system))
 {
  file_section_beg("date", io_system);
  file_put_time_t("stamp", system_data.date_stamp, io_system);
  file_section_end("date", io_system);  

  file_section_beg("maximum", io_system);
  file_put_int("lin_timeup", system_data.max_l_timeup, io_system);
  file_put_int("logons", system_data.max_logons, io_system);
  file_put_int("nl_timeup", system_data.max_nl_timeup, io_system);
  file_put_int("people", system_data.max_people, io_system);
  file_put_int("spod_time", system_data.longest_spod_time, io_system);
  file_put_int("time", system_data.max_time, io_system);
  file_put_int("uniq_logons", system_data.max_uniq_logons, io_system);
  file_section_end("maximum", io_system);  

  file_section_beg("misc", io_system);
  file_put_string("longest_spod_name",
                  system_data.longest_spod_name, 0, io_system);
  file_section_end("misc", io_system);
  
  file_section_beg("total", io_system);
  file_put_int("ever_logons", system_data.total_ever_logons, io_system);
  file_put_int("max_logged", system_data.max_time_logged, io_system);
  file_put_string("shutdown", system_data.shutdown, 0, io_system);
  file_section_end("total", io_system);
  
  if (file_write_close(io_system))
    rename("files/sys/stats.tmp", "files/sys/stats");
 }
 else
   log_assert(FALSE);
}

static void load_system_file(void)
{
 file_io real_io_system;
 file_io *io_system = &real_io_system;
 
 if (file_read_open("files/sys/stats", io_system))
 {
  int new_logons_not = FALSE;
  
  file_section_beg("date", io_system);
  system_data.date_stamp = file_get_time_t("stamp", io_system);
  if (difftime(system_data.date_stamp,
               disp_time_create(1995, 2, 1, 12, 0, 0)) < 0)
    system_data.date_stamp = disp_time_create(1995, 02, 01, 12, 0, 0);
  file_section_end("date", io_system);  
  
  file_section_beg("maximum", io_system);
  system_data.max_l_timeup = file_get_int("lin_timeup", io_system);
  system_data.max_logons = file_get_int("logins", io_system);
  if (FILE_IO_CREATED(io_system)) /* use the new version */
    system_data.max_logons = file_get_int("logons", io_system);
  system_data.max_uniq_logons = file_get_int("new_logins", io_system);
  new_logons_not = FILE_IO_CREATED(io_system);
  system_data.max_nl_timeup = file_get_int("nl_timeup", io_system);
  system_data.max_people = file_get_int("people", io_system);
  system_data.longest_spod_time = file_get_int("spod_time", io_system);
  system_data.max_time = file_get_int("time", io_system);
  if (new_logons_not) /* use the new version */
    system_data.max_uniq_logons = file_get_int("uniq_logons", io_system);
  file_section_end("maximum", io_system);

  file_section_beg("misc", io_system);
  file_get_string("longest_spod_name",
                  system_data.longest_spod_name, PLAYER_S_NAME_SZ, io_system);
  file_section_end("misc", io_system);
  
  file_section_beg("total", io_system);
  system_data.total_ever_logons = file_get_int("ever_logons", io_system);
  if (FILE_IO_CREATED(io_system)) /* use the old version */
    system_data.total_ever_logons = file_get_int("logins", io_system);
  system_data.max_time_logged = file_get_int("max_logged", io_system);
  file_get_string("shutdown", system_data.shutdown, SHUTDOWN_MSG_SZ,
                  io_system);
  file_section_end("total", io_system);
  
  file_read_close(io_system);
 }
 else
   reset_system_file();
}

static void timed_stats_save(int timed_type, void *data)
{
 if (timed_type == TIMER_Q_TYPE_CALL_DEL)
   return;

 IGNORE_PARAMETER(data);
 
 system_save_file();

 if (timed_type != TIMER_Q_TYPE_CALL_RUN_ALL)
 {
  struct timeval tv;

  gettimeofday(&tv, NULL);

  TIMER_Q_TIMEVAL_ADD_SECS(&tv, STATS_SYNC_TIME, 0);

  timer_q_add_static_node(&stats_timer_node.s, &timer_queue_global,
                          &stats_timer_node, &tv, TIMER_Q_FLAG_NODE_FUNC);
  timer_q_cntl_node(&stats_timer_node.s, TIMER_Q_CNTL_NODE_SET_FUNC,
                    timed_stats_save);
 }
}

void init_system_file(void)
{
 struct timeval tv;
 
 load_system_file();
 
 gettimeofday(&tv, NULL);
 
 TIMER_Q_TIMEVAL_ADD_SECS(&tv, STATS_SYNC_TIME, 0);
 
 timer_q_add_static_node(&stats_timer_node.s, &timer_queue_global,
                          &stats_timer_node, &tv, TIMER_Q_FLAG_NODE_FUNC);
 timer_q_cntl_node(&stats_timer_node.s, TIMER_Q_CNTL_NODE_SET_FUNC,
                   timed_stats_save);
}

static void stats_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s",
               " Re-Entering stats mode. Use '^S^Bend^s' to leave.\n"
               " '/<command>' does normal commands.\n");
}

static int user_stats_command(player *p, const char *str, size_t length)
{
 ICTRACE("stats_command"); 

 system_save_file();
 
 if (MODE_IN_MODE(p, STATS))
 {
  MODE_HELPER_COMMAND();

  return (cmds_sub_match(p, str, length,
                         &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_STATS)]));
 }

 if (!*str || !BEG_CONST_STRCASECMP("crazy news", str))
 { /* ARGGGHH socials have borken out... oh no *phew*/
  char buf1[256];
  char buf2[256];
  char buf3[256];
  
  fvtell_player(NORMAL_T(p),
                "-- CrazyStats $Line_fill(-)\n" /* FIXME */
                "\n"
                "  %-22s%s\n"
                "  %-22s%d\n"
                "\n"
                "Since boot time (%s)...\n"
                "  %-22s%d\n"
                "  %-22s%d\n"
                "  %-22s%s\n"
                "\n"
                "Since %s...\n"
                "  %-22s%d\n"
                "  %-22s%s\n"
                "\n"
                "  %-22s%s\n"
                "  %-22s%s\n"
                "$Line_fill(-)\n",
                "Uptime:",
                word_time_long(buf1, sizeof(buf1),
                               floor(difftime(now, talker_started_on)),
                               WORD_TIME_ALL),
                "Current Players:", current_players,
                DISP_TIME_P_STD(talker_started_on, p),
                "Connections:", total_logons,
                "New Connections:", total_uniq_logons,
                "Last Shutdown Reason:", system_data.shutdown,
                DISP_TIME_P_STD(system_data.date_stamp, p),
                "Connections:", system_data.total_ever_logons,
                "Longest Uptime:",
                word_time_long(buf2, sizeof(buf2),
                               system_data.max_time, WORD_TIME_ALL),
                "Longest spod:", system_data.longest_spod_name,
                "time:",
                word_time_long(buf3, sizeof(buf3),
                               system_data.longest_spod_time, WORD_TIME_ALL));
 }
 else if (*str && BEG_CONST_STRCASECMP("-m", str))
 {
  command_base *comlist = NULL;
  size_t cmd_count = 0;
  
  if (isalpha((unsigned char) *str))
    comlist = &cmds_alpha[ALPHA_LOWER_OFFSET(tolower((unsigned char) *str))];
  
  while (comlist && (cmd_count < comlist->size))
  {
   if (cmds_do_match(str, comlist->ptr[cmd_count], p->flag_no_cmd_matching))
   {
    if (comlist->ptr[cmd_count]->totals)  
      if (comlist->ptr[cmd_count]->totals == 1)
        fvtell_player(NORMAL_T(p), " The command %s has been used once "
                      "since the talker booted.\n",
                      comlist->ptr[cmd_count]->name);
      else
        fvtell_player(NORMAL_T(p), " The command %s has been used %d times "
                      "since the talker booted.\n",
                      comlist->ptr[cmd_count]->name,
                      comlist->ptr[cmd_count]->totals);
    else
      fvtell_player(NORMAL_T(p), " The command %s has not been used "
                    "since the talker booted.\n",
                    comlist->ptr[cmd_count]->name);
    return (TRUE);
   }
   ++cmd_count;
  }
  
  fvtell_player(NORMAL_T(p), "%s", " Command not found.\n");
  return (TRUE); /* or FALSE .. needs a policy decision */
 }
 else if (!BEG_CONST_STRCASECMP("-m", str))
 {
  const char *orig = str;
  
  str += CONST_STRLEN("-m");
  str += strspn(str, " ");
  length -= (orig - str);
  
  if (!*str)
  {
   cmds_function tmp_cmd;
   cmds_function tmp_rejoin;
   
   CMDS_FUNC_TYPE_RET_CHARS_SIZE_T((&tmp_cmd), user_stats_command);
   CMDS_FUNC_TYPE_NO_CHARS((&tmp_rejoin), stats_rejoin_func);
   
   if (mode_add(p, "Stats Mode-> ", MODE_ID_STATS, 0,
                &tmp_cmd, &tmp_rejoin, NULL))
     fvtell_player(NORMAL_T(p), "%s",
                   " Entering stats mode. Use '^S^Bend^s' to leave.\n"
                   " '/<command>' does normal commands.\n");
   else
     fvtell_player(SYSTEM_T(p), 
                   " You cannot enter stats mode as you are in too many "
                   "other modes.\n");
   return (TRUE);
  }
  
  return (cmds_sub_match(p, str, length,
                         &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_STATS)]));
 }

 return (TRUE);
}

static void user_system_see_all(player *p)
{
 char buf[256];
 
 system_save_file();

 /* some of these aren't used */
 
 fvtell_player(NORMAL_T(p), "%s", " Viewing all the system data...\n");

 fvtell_player(NORMAL_T(p), "   Motd date         - %s\n",
               DISP_TIME_P_STD(system_data.motd, p));
 fvtell_player(NORMAL_T(p), "   SU_Motd date      - %s\n",
               DISP_TIME_P_STD(system_data.su_motd, p));

 fvtell_player(NORMAL_T(p), "   Total logons      - %d\n",
               system_data.total_ever_logons);
 fvtell_player(NORMAL_T(p), "   Shutdown message  - %s\n",
               system_data.shutdown);
 fvtell_player(NORMAL_T(p), "   Total time        - %s\n",
               word_time_long(buf, sizeof(buf),
                              system_data.max_time_logged, WORD_TIME_ALL));
 fvtell_player(NORMAL_T(p), "   Most logons ever  - %d\n", 
               system_data.max_logons);
 fvtell_player(NORMAL_T(p), "        Time up then - %s\n",
               word_time_long(buf, sizeof(buf),
                              system_data.max_l_timeup, WORD_TIME_ALL));
 fvtell_player(NORMAL_T(p), "   Most uniq logons   - %d\n",
               system_data.max_uniq_logons);      
 fvtell_player(NORMAL_T(p), "        Time up then - %s\n",
               word_time_long(buf, sizeof(buf),
                              system_data.max_nl_timeup, WORD_TIME_ALL));
 fvtell_player(NORMAL_T(p), "   Most on at once   - %d\n",
               system_data.max_people);
 fvtell_player(NORMAL_T(p), "   Longest up time   - %s\n",
               word_time_long(buf, sizeof(buf),
                              system_data.max_time, WORD_TIME_ALL));
 fvtell_player(NORMAL_T(p), "   Longest spod name - %s\n",
               system_data.longest_spod_name);
 fvtell_player(NORMAL_T(p), "          Their time - %s\n",
               word_time_long(buf, sizeof(buf),
                              system_data.longest_spod_time, WORD_TIME_ALL));
 fvtell_player(NORMAL_T(p), "      Version number - %s\n", VERSION);
}

static void stats_view_commands(player *p)
{
 user_cmds_show_section(p, "stats");
}

static void stats_exit_mode(player *p)
{
 assert(MODE_IN_MODE(p, STATS));
 
 fvtell_player(NORMAL_T(p), "%s", " Leaving stats mode.\n");

 mode_del(p);
}

static void user_system_version(player *p)
{
 fvtell_player(NORMAL_FT(OVERRIDE_RAW_OUTPUT_VARIABLES, p),
               " Current version of $Talker-Name is: %s\n", VERSION);
}

static void user_su_system_view_info(player *p)
{ 
 fvtell_player(NORMAL_T(p), "%s", "\n$Line_fill(-)\n"
               "Version system information\n"
               "\n$Line_fill(-)\n");
 
 fvtell_player(NORMAL_T(p), "Messages of the day:\n"
               "  Standard      : %s\n",
               DISP_TIME_P_STD(system_data.motd, p));
 
 fvtell_player(NORMAL_T(p), "  Su            : %s\n",
               DISP_TIME_P_STD(system_data.su_motd, p));
 
 fvtell_player(NORMAL_T(p), "Code Updates:\n"
               "  Crazy code    : $Date($Talker-Version)-Date\n\n");
}

static void user_su_system_update_data(player *p, const char *str)
{ 
 if (!strcasecmp(str, "motd"))
 {
  fvtell_player(NORMAL_T(p), "%s", " Motd has been updated.\n");
  system_data.motd = now;
 }
 else if (!strcasecmp(str, "sumotd"))
 {
  fvtell_player(NORMAL_T(p), "%s", " SU motd has been updated.\n");
  system_data.su_motd = now;
 }
 else
   TELL_FORMAT(p, "[ motd | sumotd ]");
 
 system_save_file();
}

void cmds_init_stats(void)
{
 CMDS_BEGIN_DECLS();

#define CMDS_SECTION_SUB CMDS_SECTION_STATS
 
 CMDS_ADD_SUB("commands", stats_view_commands, NO_CHARS);
 CMDS_ADD_SUB("end", stats_exit_mode, NO_CHARS);
 CMDS_PRIV(mode_stats);

 CMDS_ADD_SUB("update", user_su_system_update_data, CONST_CHARS);
 CMDS_PRIV(coder_normal_su);
 CMDS_ADD_SUB("save", user_su_system_save_file_verb, NO_CHARS);
 CMDS_PRIV(coder_admin);
 CMDS_ADD_SUB("see_all", user_system_see_all, NO_CHARS);
 CMDS_ADD_SUB("sreason", user_su_system_change_last_shutdown, CONST_CHARS);
 CMDS_PRIV(coder_admin);
 CMDS_ADD_SUB("version", user_system_version, NO_CHARS);
 CMDS_ADD_SUB("view_system", user_su_system_view_info, NO_CHARS);
 CMDS_PRIV(coder_admin);
 
#undef CMDS_SECTION_SUB

 CMDS_ADD("stats", user_stats_command, RET_CHARS_SIZE_T, INFORMATION);
 CMDS_ADD("version", user_system_version, NO_CHARS, INFORMATION);
}
