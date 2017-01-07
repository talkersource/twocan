#define LAST_LOGON_C
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

static char last_logon_names[LAST_LOGON_NAME_SZ][PLAYER_S_NAME_SZ];
static int last_logon_count = 0;

static int last_logon_find(const char *name)
{
 int tmp = last_logon_count;

 do
 {
  if (*last_logon_names[tmp] && !strcmp(name, last_logon_names[tmp]))
    return (tmp);
  
  ++tmp;
  tmp %= LAST_LOGON_NAME_SZ;   
 } while (tmp != last_logon_count);

 return (-1);
}

void last_logon_add(const char *name)
{
 int offset = 0;
 
 if ((offset = last_logon_find(name)) != -1)
 {
  int prev = offset;
  
  ++offset;
  offset %= LAST_LOGON_NAME_SZ;   

  if (offset == last_logon_count)
    return; /* optimise */
  
  while (offset != last_logon_count)
  {
   qstrcpy(last_logon_names[prev], last_logon_names[offset]);

   prev = offset;
   
   ++offset;
   offset %= LAST_LOGON_NAME_SZ;   
  }

  qstrcpy(last_logon_names[prev], name);
 }
 else
 {
  qstrcpy(last_logon_names[last_logon_count++], name);
  
  last_logon_count %= LAST_LOGON_NAME_SZ;
 }
}

static void user_last_logon_display(player *p, parameter_holder *params)
{
 int tmp = last_logon_count;
 player_tree_node *sps[LAST_LOGON_NAME_SZ];
 int count = 0;
 int user_count = 0;
 
 if (params->last_param)
 {
  if (params->last_param == 1)
    user_count = atoi(GET_PARAMETER_STR(params, 1));
  else
    TELL_FORMAT(p, "[number]");
 }
 
 if (user_count <= 0)
   user_count = configure.last_logon_def_show;
 
 do
 {
  if (*last_logon_names[tmp])
  {
   sps[count] = player_find_all(p, last_logon_names[tmp], PLAYER_FIND_SELF);
   if (sps[count])
     ++count;
  }
  
  ++tmp;
  tmp %= LAST_LOGON_NAME_SZ;
 } while (tmp != last_logon_count);

 if (count)
 {
  char buffer[sizeof("Last ^S^B%d^s logons") + BUF_NUM_TYPE_SZ(int)];

  if (user_count > count)
    user_count = count;
  
  sprintf(buffer, "Last ^S^B%d^s logons", user_count);
  ptell_mid(NORMAL_T(p), buffer, FALSE);
  
  tmp = count;
  while ((tmp > 0) && (user_count > 0))
  {
   --tmp;
   --user_count;
   fvtell_player(NORMAL_WFT(SPLIT_ON_PUNCTUATION | (PLAYER_S_NAME_SZ + 3), p),
                 "%-*s - %s", PLAYER_S_NAME_SZ, sps[tmp]->name,
                 DISP_TIME_P_STD(sps[tmp]->logoff_timestamp, p));
   if (p->saved->priv_command_trace)
     fvtell_player(NORMAL_WFT(SPLIT_ON_PUNCTUATION | (PLAYER_S_NAME_SZ + 3),
                              p), " - %d.%d.%d.%d:^S^B%s^s",
                   (int)sps[tmp]->last_ip_address[0],
                   (int)sps[tmp]->last_ip_address[1],
                   (int)sps[tmp]->last_ip_address[2],
                   (int)sps[tmp]->last_ip_address[3],
                   sps[tmp]->last_dns_address);
   
   fvtell_player(NORMAL_T(p), "%s", "\n");
  }
  
  fvtell_player(NORMAL_T(p), "%s", DASH_LEN);

  pager(p, PAGER_DEFAULT);
 }
 else
   fvtell_player(NORMAL_T(p), "%s", " No-one has logged off yet.\n");
}

void user_configure_last_logon_def_show(player *p, const char *str)
{
 USER_CONFIGURE_INT_FUNC(last_logon_def_show,
                         "Last logon", "default show", 1, LAST_LOGON_NAME_SZ);
}

void cmds_init_last_logon(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("last_logoff", user_last_logon_display, PARSE_PARAMS, INFORMATION);
}
