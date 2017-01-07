#define SCREENLOCK_C
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

static void internal_screenunlock(player *p, const char *str)
{
 ICTRACE("unscreen_lock");

 assert(MODE_IN_MODE(p, SCREENLOCK));

 if (!passwd_check(p, str))
 {
  fvtell_player(NORMAL_T(p), "%s", "\n Hey, thats wrong, password failed!\n");
 }
 else
 {
  mode_del(p);
  fvtell_player(NORMAL_T(p), "%s", "\n\n Password Authenticated, Screenlock "
                "disabled.\n");
  telopt_ask_passwd_mode_off(p);
  return;
 }
}

void user_screenlock(player *p)
{
 if (p->passwd[0])
 {
  cmds_function tmp_cmd;
    
  CMDS_FUNC_TYPE_CONST_CHARS(&tmp_cmd, internal_screenunlock);
  
  fvtell_player(NORMAL_T(p), "%s", 
                "\n Screenlock activated, enter your password to resume.\n");
  while (!mode_add(p, " Please enter your current password: ",
                   MODE_ID_SCREENLOCK, 0, &tmp_cmd, NULL, NULL))
    mode_del(p);
  
  telopt_ask_passwd_mode_on(p);
 }
 else
 {
  fvtell_player(NORMAL_T(p), "%s", 
                "\n You have not set a password, why not set one first??.\n");
 }
}
