#define STATS_FILES_C
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

/* Contains all the functions for stats_open_fileing, closing and addings to
   the perminant statistics files to enable the talker usage logging and so
   on. Please consult Documentation/stats_files.README for more information. */

/* the file pointer and name for the currently stats_open_file stats file */
static FILE *stats_fp = NULL;

static int stats_resi_level(player *p)
{
 if (p->saved->priv_admin)
   return (6);
 else if (p->saved->priv_lower_admin)
   return (5);
 else if (p->saved->priv_senior_su)
   return (4);
 else if (p->saved->priv_normal_su)
   return (3);
 else if (p->saved->priv_basic_su)
   return (2);
 else if (p->saved->priv_base)
   return (1);
 else 
   return (0);
}

void stats_open_file(int event)
{
 if (stats_fp) 
   return;

 if (!(stats_fp = fopen(disp_time_filename(now, "logs/stats/", ".stats"),
                        "ab")))
 {
  log_assert(FALSE);
  return;
 }
 
 /* NOTE: does fprintf work like fwrite and return 0 on error ? */
 if (fprintf(stats_fp, "%c%s\n", event, disp_time_file_name(now)) < 2)
   log_assert(FALSE);
}

void stats_close_file(char type)
{
 if (!stats_fp) /* happens on shutdown */
   return;

 switch (type)
 {
  case STATS_CRASH:
  case STATS_SHUT:
  case STATS_STILL_UP:
    if (fprintf(stats_fp, "%c%s\n", type, disp_time_file_name(now)) < 2)
      log_assert(FALSE);
    break;
    
  default:
    log_assert(FALSE);
 }

 if (fflush(stats_fp))
   log_assert(FALSE);
 
 /* close the file etc */
 if (fclose(stats_fp))
   log_assert(FALSE);

 stats_fp = NULL;
}

static void user_su_stats_new_file(player *p)
{
 if (stats_fp)
   stats_close_file(STATS_STILL_UP);
 else
   fvtell_player(NORMAL_T(p), "%s", " Stats file not open!\n");
 
 stats_open_file(STATS_STILL_UP);

 fvtell_player(NORMAL_T(p), "%s", " New file started.\n");
}

static void user_su_stats_flush_file(player *p)
{
 if (!stats_fp)
   fvtell_player(NORMAL_T(p), "%s", " Stats file not open!\n");
 else
   if (fflush(stats_fp))
     fvtell_player(NORMAL_T(p),
                   " Flushed but something bad happened %d %s.\n",
                   errno, strerror(errno));
   else
     fvtell_player(NORMAL_T(p), "%s", " Flushed ok.\n");
}

void stats_log_event(player *p, char event, int extra_info)
{
 static unsigned int last_command_number = 0;
 static int last_event = 0;
 
 if (!stats_fp)
   return;

 /*makes sure we don't get two events for a player when logging out */
 if ((last_command_number == current_command_number) && (event == last_event))
   return;
 else
 {
  last_command_number = current_command_number;
  last_event = event;
 }

 if (fprintf(stats_fp, "%c%s", event, disp_time_file_name(now)) < 2)
 {
  log_assert(FALSE);
  return;
 }

 switch (event)
 {
  case STATS_RESI_OFF:
    if (!extra_info) /* if they're being forced off, this will be set */
      extra_info = 1;
    /* FALLTHROUGH */
  case STATS_RESI_ON:
  {
   int idle_time = difftime(now, p->last_command_timestamp);

   if (!p)
   {
    log_assert(FALSE);
    return;
   }
   else if (!p->saved)
   {
    log_assert(FALSE);
    return;
   }
   
   fprintf(stats_fp, ",%s", p->saved->lower_name);

   fprintf(stats_fp, ",%d", stats_resi_level(p));

   fprintf(stats_fp, ",%d", extra_info);

   if (idle_time >= IDLE_TIME_PERMITTED)
     idle_time += p->saved->total_idle_logon;
   else
     idle_time = p->saved->total_idle_logon;
   idle_time += p->idle_logon;
   
   fprintf(stats_fp, ",%u", idle_time);
   break;
  }
  
  case STATS_ON_SU:
  case STATS_OFF_SU:
  {
   fprintf(stats_fp, ",%s", p->saved->lower_name);
   
   fprintf(stats_fp, ",%d", count_sus_on_or_off_duty(TRUE));
   
   fprintf(stats_fp, ",%d", extra_info);
   break;
  }

  default:
    break;
 }

 fputc('\n', stats_fp);
}

void init_stats_files(void)
{
 stats_open_file(STATS_BOOT);
}

void cmds_init_stats_files(void)
{
 CMDS_BEGIN_DECLS();

#define CMDS_SECTION_SUB CMDS_SECTION_STATS
 CMDS_ADD_SUB("flush", user_su_stats_flush_file, NO_CHARS);
 CMDS_PRIV(coder_normal_su);
 CMDS_ADD_SUB("new_stats_file", user_su_stats_new_file, NO_CHARS);
 CMDS_PRIV(coder_normal_su);
#undef CMDS_SECTION_SUB
}
