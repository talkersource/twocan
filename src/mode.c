#define MODE_C
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

static const char *mode_map_names[MODE_ID_SZ] = {
 "**** ERROR ****",
 "EW Base mode", "NUTS Base mode", "IRC Base mode",

 "Mail", "News", "NewsGroup", "Room",
 "Draughts", "Edit", "Stats", "Check",
 "Converse", "Intercom", "Pager",

 "Residency 1 (email)", "Residency part 2 (passwd)", "Residency 3 (passwd 2)",

 "Jailed",

 "Login Name ", "Login Password",
 
 "Login Newbie 1", "Login Newbie 2 (check name)", "Login Newbie 3 (gender)",
 "Login Newbie 4 (continue/end)", "Login Newbie 5 (msg)",
 
 "Login Motd", "Login SuMotd",

 "Mail 1 (To)", "Mail 2 (Cc)", "Mail 3 (Subject)",

 "Passwd 1 (old)", "Passwd 2 (new)", "Passwd 3 (verify)",

 "Screenlock",
};



void mode_change(player *p, const char *prompt, unsigned int id,
                 unsigned int flags,
                 cmds_function *cmd_func,
                 cmds_function *rejoin_func, cmds_function *cleanup_func)
{
 MODE_CURRENT(p).cmd_func = *cmd_func;
 
 if (rejoin_func)
   MODE_CURRENT(p).rejoin_func = *rejoin_func;
 if (cleanup_func)
   MODE_CURRENT(p).cleanup_func = *cleanup_func;

 MODE_CURRENT(p).flags = flags;
 MODE_CURRENT(p).id = id;
 
 if ((MODE_CURRENT(p).prompt = prompt))
   prompt_update(p, prompt);

 if (MODE_CURRENT(p).flags & MODE_FLAGS_SYSTEM_INFO_ONLY)
   p->system_info_only = TRUE;
 else
   p->system_info_only = FALSE;
}

int mode_add(player *p, const char *prompt, unsigned int id,
             unsigned int flags,
             cmds_function *cmd_func,
             cmds_function *rejoin_func,
             cmds_function *cleanup_func)
{
 cmds_function rejoin_dummy;
 cmds_function cleanup_dummy;
 
 if (p->mode_count >= (MODE_MODES_SZ - 1))
   return (FALSE);

 ++p->mode_count;

 if (!cleanup_func)
 {
  CMDS_FUNC_TYPE_NOTHING(&cleanup_dummy, NULL);
  cleanup_func = &cleanup_dummy;
 }

 if (!rejoin_func)
 {
  CMDS_FUNC_TYPE_NOTHING(&rejoin_dummy, NULL);
  rejoin_func = &rejoin_dummy;
 }

 mode_change(p, prompt, id, flags, cmd_func, rejoin_func, cleanup_func);
 
 return (TRUE);
}

int mode_del(player *p)
{
 if (MODE_INVALID(p))
 {
  log_assert(FALSE);
  p->mode_count = 0;
  fvtell_player(SYSTEM_T(p),
                " ***** Error you've just quit from your base input mode.\n");
  user_logoff(p, NULL);
  return (FALSE);
 }

 cmds_run_func(&MODE_CURRENT(p).cleanup_func, p, NULL, 0);
 
 --p->mode_count;
 
 if (MODE_CURRENT(p).prompt)
   prompt_update(p, MODE_CURRENT(p).prompt);
 
 if (MODE_CURRENT(p).flags & MODE_FLAGS_SYSTEM_INFO_ONLY)
   p->system_info_only = TRUE;
 else
   p->system_info_only = FALSE;
 
 cmds_run_func(&MODE_CURRENT(p).rejoin_func, p, NULL, 0);

 return (TRUE);
}

static int mode_ew_base_command(player *p, const char *str, size_t length)
{
 if (*str && (*str != '#'))
   return (cmds_match(p, str, length, CMDS_MATCH_DEFAULT_LOOP));

 return (FALSE);
}

void mode_init_base(player *p)
{
 cmds_function tmp_cmd1;
 cmds_function tmp_cmd2;
 cmds_function tmp_cmd3;

 log_assert(!p->mode_count);
 
 CMDS_FUNC_TYPE_RET_CHARS_SIZE_T(&tmp_cmd1, mode_ew_base_command);
 CMDS_FUNC_TYPE_NOTHING(&tmp_cmd2, NULL);
 CMDS_FUNC_TYPE_NOTHING(&tmp_cmd3, NULL);
 
 mode_change(p, NULL, MODE_ID_BASE_EW, 0, &tmp_cmd1, &tmp_cmd2, &tmp_cmd3);
 /* NOTE: don't want to do a prompt_update ... as we're not in a room */
 MODE_CURRENT(p).prompt = p->prompt;
}

static void user_su_mode_list(player *p, const char *str)
{
 player *p2 = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL);
 unsigned int count = 0;
 
 if (!p2)
   return;
 
 if (p2->mode_count >= MODE_MODES_SZ)
 {
  assert(FALSE);
  fvtell_player(NORMAL_T(p), " ** ERROR **.\n");
  return;
 }
 
 fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- is in %d modes.",
               p2->saved->name, p2->mode_count + 1);
 
 while (count <= p2->mode_count)
 {
  fvtell_player(NORMAL_T(p), "\n Mode [%02d] = %s",
                count + 1, mode_map_names[p2->modes[count].id]);
  ++count;
 }
 
 fvtell_player(NORMAL_T(p), "%s", "  <=- This is the current mode.\n");
}

static void user_su_mode_remove(player *p, const char *str)
{
 player *p2 = player_find_on(p, str, PLAYER_FIND_SC_SU_ALL);

 if (p2)
 {
  if (MODE_INVALID(p2))
  {
   fvtell_player(NORMAL_T(p), " The player -- ^S^B%s^s -- isn't in any modes "
                 "at the moment.\n", p2->saved->name);
   return;
  }

  fvtell_player(NORMAL_T(p),
                " Removed the player -- ^S^B%s^s -- from mode %s\n",
                p2->saved->name, mode_map_names[p2->modes[p2->mode_count].id]);

  mode_del(p2);
 }
}

void cmds_init_mode(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("modes", user_su_mode_list, CONST_CHARS, SU);
 CMDS_PRIV(pretend_su);
 CMDS_ADD("unmode", user_su_mode_remove, CONST_CHARS, SU);
 CMDS_PRIV(normal_su);
}
