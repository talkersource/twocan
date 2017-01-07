#define BACKUPS_C
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
 * email: james@twocan.org
 */
#include "main.h"



void backups_run(void)
{
 static int been_here_before = FALSE;
 static time_t timestamp;
 char buf[sizeof("scripts/backups_run %d &") + BUF_NUM_TYPE_SZ(int)];
 
 if (been_here_before && (difftime(now, timestamp) < MK_MINUTES(2)))
   return;

 been_here_before = TRUE;
 timestamp = now;
 
 sys_flag.backups_run = FALSE;

 sprintf(buf, "scripts/backups_run %d &", configure.backups_ammount);
 
 gettimeofday(&now_timeval, NULL);
 timer_q_run_norm(&now_timeval);
 
 stats_log_event(NULL, STATS_BACKUP, STATS_NO_EXTRA);
 
 sys_wall(0, "%s", "\n\n -=> Syncing all the files for the daily backups <=-\n"
	  " -=> Program pausing to save files <=-\n\n");
 socket_all_players_output();

 gettimeofday(&now_timeval, NULL);
 timer_q_run_norm(&now_timeval);
 
 sys_wall(0, "%s", " -=> Part one complete <=-\n");

#if 0
 socket_all_players_output();

 dump_spodlist_to_file_number(NULL, NULL);

 gettimeofday(&now_timeval, NULL);
 timer_q_run_norm(&now_timeval);
#endif
 
 sys_wall(0, "%s", " -=> Part two complete <=-\n");

#ifdef USE_FIXWEB
 socket_all_players_output();

 system("scripts/fix_web");

 gettimeofday(&now_timeval, NULL);
 timer_q_run_norm(&now_timeval);
#endif
 
 sys_wall(0, "%s", " -=> Part three complete <=-\n");
 socket_all_players_output();
 
 system(buf);

 sys_wall(0, "%s", " -=> Part four complete <=-\n");
 sys_wall(0, "%s", " -=> Backup complete <=-\n\n\n");

 stats_log_event(NULL, STATS_BACKUP, STATS_NO_EXTRA);
}

static void user_backups_run(player *p)
{
 sys_flag.backups_run = TRUE;

 channels_wall("staff", 3, NULL, " -=> %s%s %s started backups.",
               gender_choose_str(p->gender, "", "", "The ", "The "),
               p->saved->name, (p->gender == GENDER_PLURAL) ? "have" : "has");
}

void user_configure_backups_ammount(player *p, const char *str)
{
 USER_CONFIGURE_INT_FUNC(backups_ammount, "Backups", "ammount", 0, INT_MAX);
}

void cmds_init_backups(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("backups_run", user_backups_run, NO_CHARS, ADMIN);
 CMDS_FLAG(no_expand); CMDS_PRIV(admin);
}
