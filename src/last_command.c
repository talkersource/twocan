#define LAST_COMMAND_C
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



void last_command_add(player *p, const char *command_name)
{
 if (p->last_commands[p->last_commands_index].command_name &&
     !strcmp(command_name,
             p->last_commands[p->last_commands_index].command_name))
 {
  int timediff = difftime(now,
                          p->last_commands[p->last_commands_index].timestamp);

  ++p->last_commands[p->last_commands_index].number_of_times;

  if (timediff < p->last_commands[p->last_commands_index].min_timediff)
    p->last_commands[p->last_commands_index].min_timediff = timediff;
  if (timediff > p->last_commands[p->last_commands_index].max_timediff)
    p->last_commands[p->last_commands_index].max_timediff = timediff;
 }
 else
 {
  int timediff = 0;

  if (p->last_commands[p->last_commands_index].timestamp)
    timediff = difftime(now,
                        p->last_commands[p->last_commands_index].timestamp);
  else
    timediff = difftime(now, p->logon_timestamp);
  
  ++p->last_commands_index;
  p->last_commands_index %= LAST_COMMANDS_SZ;
  
  p->last_commands[p->last_commands_index].min_timediff = timediff;
  p->last_commands[p->last_commands_index].max_timediff = timediff;

  p->last_commands[p->last_commands_index].number_of_times = 1;
 }

 p->last_commands[p->last_commands_index].command_name = command_name;
 p->last_commands[p->last_commands_index].timestamp = now;
}

void last_command_clear(player *p)
{
 int count = 0;
 
 while (count < LAST_COMMANDS_SZ)
 {
  p->last_commands[count].min_timediff = 0;
  p->last_commands[count].max_timediff = 0;
  p->last_commands[count].command_name = NULL;
  p->last_commands[count].number_of_times = 0;
  
  ++count;
 }
 
 p->last_commands_index = 0;
}

static void user_su_last_command(player *p, const char *str)
{
 player *p2 = NULL;
 int count = 0;
 int cyc_index = 0;
 char buf[256];
 
 if (!*str)
   TELL_FORMAT(p, "<player>");

 if (!(p2 = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL)))
   return;

 cyc_index = p2->last_commands_index + 1; /* pointing at last one currently */
 cyc_index %= LAST_COMMANDS_SZ;
 
 ptell_mid(NORMAL_T(p), p2->saved->name, FALSE);
 
 while (count < LAST_COMMANDS_SZ)
 {
  if (p2->last_commands[cyc_index].command_name)
  {
   fvtell_player(INFO_FTP(27 | RAW_OUTPUT, p),
                 "%02d %-15s -- (%02d) ",
                 LAST_COMMANDS_SZ - count,
                 p2->last_commands[cyc_index].command_name,
                 p2->last_commands[cyc_index].number_of_times);
   
   if (difftime(now, p2->last_commands[cyc_index].timestamp))
     fvtell_player(INFO_FTP(27, p), "^BSINCE^b %s ",
                   word_time_long(buf, sizeof(buf),
                                  difftime(now,
                                       p2->last_commands[cyc_index].timestamp),
                                  WORD_TIME_DEFAULT));

   if (p2->last_commands[cyc_index].number_of_times > 1)
   {
    if (p2->last_commands[cyc_index].min_timediff)
      fvtell_player(INFO_FTP(27, p), "^BMIN^b %s ",
                    word_time_long(buf, sizeof(buf),
                                   p2->last_commands[cyc_index].min_timediff,
                                   WORD_TIME_DEFAULT));
    else
      fvtell_player(INFO_FTP(27, p), "%s", "^BMIN^b instantly ");
    
    if (p2->last_commands[cyc_index].max_timediff)
      fvtell_player(INFO_FTP(27, p), "^BMAX^b %s ",
                    word_time_long(buf, sizeof(buf),
                                   p2->last_commands[cyc_index].max_timediff,
                                   WORD_TIME_DEFAULT));
    else
      fvtell_player(INFO_FTP(27, p), "%s", "^BMAX^b instantly ");
   }
   else
   {
    assert(p2->last_commands[cyc_index].min_timediff ==
           p2->last_commands[cyc_index].max_timediff);

    if (p2->last_commands[cyc_index].max_timediff)
      fvtell_player(INFO_FTP(27, p), "^BTIME^b %s ",
                    word_time_long(buf, sizeof(buf),
                                   p2->last_commands[cyc_index].max_timediff,
                                   WORD_TIME_DEFAULT));
    else
      fvtell_player(INFO_FTP(27, p), "%s", "^BTIME^b instantly ");
   }
   
   fvtell_player(INFO_FTP(27, p), "\n");
  }
  
  ++count;
  ++cyc_index;
  cyc_index %= LAST_COMMANDS_SZ;
 }
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void cmds_init_last_command(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("last_command", user_su_last_command, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);
}
