#define CHECK_RECIEVE_C
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

int check_receive_state(player_tree_node *from, player *to)
{
 assert((from != to->saved) && to && to->saved && from && P_IS_ON(from));

 /* check for bad call */
 if (!(to && to->saved && from && P_IS_ON(from)) || (from == to->saved))
   return (TRUE);

 if (to->idle_msg[0])
   fvtell_player(TALK_FT(RAW_OUTPUT | HILIGHT, NULL, from->player_ptr),
                 " -=> Idling: %s%s%s\n", to->saved->name,
                 isits1(to->idle_msg), isits2(to->idle_msg));
 else if (MODE_IN_MODE(to, SCREENLOCK))
   fvtell_player(ALL_T(NULL, from->player_ptr, NULL,
                       3 | HILIGHT | SYSTEM_INFO, now), "%s%s%s",
		 " -=> ", to->saved->name, " is currently screenlocked!\n");
 else if (to->system_info_only)
   fvtell_player(ALL_T(NULL, from->player_ptr, NULL,
                       7 | HILIGHT | SYSTEM_INFO, now), "%s%s%s%s%s%s",
		 " -=> ",
                 gender_choose_str(to->gender, "", "", "The ", "The "),
                 to->saved->name,
                 " can't see messages, at the moment, because ",
                 gender_choose_str(to->gender, "he is busy. He",
                                   "she is busy. She",
                                   "they are busy. They", "it is busy. It"),
                 " should get them in a few minutes.\n");
 else if (!from->player_ptr->flag_idle_warns_block &&
          (difftime(now, to->last_command_timestamp) > MK_MINUTES(5)))
 {
  char buf[256];
  
  fvtell_player(ALL_T(NULL, from->player_ptr, NULL,
                      3 | HILIGHT | SYSTEM_INFO, now),
                "%s%s%s%s%s",
                " -=> ", to->saved->name,
                (from->player_ptr->gender == GENDER_PLURAL) ?
                " are " : " is ",
                word_time_long(buf, sizeof(buf),
                               difftime(now, to->last_command_timestamp),
                               WORD_TIME_SECONDS | WORD_TIME_MINUTES |
                               WORD_TIME_HOURS),
                " idle!\n");
 }
 
 return (TRUE);
}
