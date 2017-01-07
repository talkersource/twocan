#define KARMA_C
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


static int karma_points(player *p)
{
 int available = real_total_logon_time(p->saved);

 if (available)
   available /= MK_HOURS(1);

 KARMA_TRANS_FUNC(available);
 
 return (available - p->karma_used);
}

static int karma_user_points(player *p)
{
 int points = karma_points(p);

 if (points < 0)
   points = 0;
 
 if (!points)
   fvtell_player(SYSTEM_T(p), "%s",
                 " You do not have any ^S^Bkarma points^s left.\n");

 return (points);
}

static void user_karma_command(player *p, parameter_holder *params)
{  
 if (params->last_param < 1)
   TELL_FORMAT(p, "<cutoff|add|delete|view> [option]");

 lower_case(GET_PARAMETER_STR(params, 1));
 
 if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "cutoff"))
 {
  int is_new = FALSE;
  
  if (params->last_param > 2)
    TELL_FORMAT(p, "cutoff [value]");
  else if (params->last_param == 2)
  {
   unsigned long karma_amount = atoi(GET_PARAMETER_STR(params, 2)); 
   
   p->karma_cutoff = karma_amount;
  }
  
  fvtell_player(NORMAL_T(p), " Karma cut off %sset to '^S^B%d^s'.\n",
                is_new ? "^S^Bnow^s " : "", p->karma_cutoff);
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "add"))
 {
  player *p2 = NULL;
  
  if (params->last_param != 2)
    TELL_FORMAT(p, "add <player>");

  if (!karma_user_points(p))
    return;
  
  if (!(p2 = player_find_load(p, GET_PARAMETER_STR(params, 2),
                              PLAYER_FIND_SC_EXTERN & ~PLAYER_FIND_SELF)))
    return;
  
  ++p2->saved->karma_value;
  ++p->karma_used;
  
  fvtell_player(NORMAL_T(p), " You have added a karma point to the "
                "player '^S^B%s^s'.\n", p2->saved->name);  
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "delete"))
 {
  player *p2 = NULL;
  
  if (params->last_param != 2)
    TELL_FORMAT(p, "delete <player>");

  if (!karma_user_points(p))
    return;
  
  if (!(p2 = player_find_load(p, GET_PARAMETER_STR(params, 2),
                              PLAYER_FIND_SC_EXTERN & ~PLAYER_FIND_SELF)))
    return;
  
  --p2->saved->karma_value;
  ++p->karma_used;
  
  fvtell_player(NORMAL_T(p), " You have removed a karma point from the "
                "player '^S^B%s^s'.\n", p2->saved->name);
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "view"))
 {
  player *p2 = NULL;
  
  switch (params->last_param)
  {
   case 1:
   case 2:
     break;
     
   default:
     TELL_FORMAT(p, "view [player]");
  }

  if (params->last_param == 1)
    p2 = p;
  else if (!(p2 = player_find_load(p, GET_PARAMETER_STR(params, 2),
                                   PLAYER_FIND_SC_EXTERN)))
    return;
  
  fvtell_player(NORMAL_T(p), " The player '^S^B%s^s' has the karma "
                "value of ^S^B%d^s.\n",
                p2->saved->name, p2->saved->karma_value);
 }
 else
   TELL_FORMAT(p, "<cutoff|add|delete> [option]");
}

void cmds_init_karma(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("karma", user_karma_command, PARSE_PARAMS, LIST);
}
