#define WOTW_C
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

static void user_wotw(player *p)
{
 if (msg_wotw.text)
 {
  fvtell_player(NORMAL_T(p), "%s", msg_wotw.text);
  pager(p, PAGER_DEFAULT);
 }
 else
 {
  fvtell_player(NORMAL_T(p), "%s", " Wotw is currently disabled.");
  wlog("error", " Couldn't find wotw file.");
 }
}

void cmds_init_wotw(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("wotw", user_wotw, NO_CHARS, MISC);
}
