#define CONVERSE_C
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


static void converse_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s",
               " Re-Entering 'converse' mode."
               " Everything you type will get said.\n"
               " Use '^B/end^N'to leave this mode.\n"); 
}

static int converse_mode(player *p, const char *str, size_t length)
{
 ICTRACE("converse_command");
 
 if (MODE_IN_MODE(p, CONVERSE))
 {
  if ((length == 4) && !memcmp("/end", str, 4))
  {
   fvtell_player(NORMAL_T(p), "%s", " Leaving converse mode.\n");
   mode_del(p);
   return (TRUE);
  }
 }
 else
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T((&tmp_cmd), converse_mode);
    CMDS_FUNC_TYPE_NO_CHARS((&tmp_rejoin), converse_rejoin_func);
    
    if (mode_add(p, p->converse_prompt, MODE_ID_CONVERSE, 0,
                 &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), "%s",
                    " Entering 'converse' mode."
                    " Everything you type will get said.\n"
                    " Use '^B/end^N'to leave this mode.\n");

    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter converse mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }

 if (!*str)
   user_say(p, "^N", 2);
 else
   user_say(p, str, length);

 return (TRUE);
}

static void user_converse_prompt(player *p, const char *str)
{
 COPY_STR_BUILD_FUNC_NORM(p->converse_prompt, str, "converse prompt",
                          PROMPT_SZ, 0);
}

void cmds_init_converse(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("converse", converse_mode, RET_CHARS_SIZE_T, COMMUNICATION);
 CMDS_PRIV(base);
 CMDS_ADD("cprompt", user_converse_prompt, CONST_CHARS, SETTINGS);
 CMDS_FLAG(no_beg_space); CMDS_FLAG(no_end_space);
}
