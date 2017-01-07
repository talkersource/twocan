#define MOTD_C
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


void user_motd(player *p)
{ 
 p->motdlr = now;
 
 fvtell_player(NORMAL_T(p), "%s", msg_motd.text);
}

static void motd_edit_cleanup(player *p)
{ 
 buffer_file_destroy(p);
}

static void motd_edit_quit(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", " Leaving without changes.\n");

 motd_edit_cleanup(p);
}

static void motd_edit_end(player *p)
{
 assert(MODE_IN_MODE(p, EDIT));
 
 if (!msg_edit_sync_file(&msg_motd, EDIT_BASE(p)))
   fvtell_player(NORMAL_T(p), "%s",
                 " Could not write the motd file - error occured.\n");
else
  fvtell_player(NORMAL_T(p), "%s", " MOTD changed.\n");

 motd_edit_cleanup(p);
}

static void user_motd_edit(player *p)
{
 int created = 0;
 
 if ((created = buffer_file_create(p)) > 0)
   P_MEM_ERR(p);
 else if (created < 0)
   fvtell_player(NORMAL_T(p), "%s",
                 " You cannot edit the motd whilst already editing a file.\n");
 else if (edit_start(p, msg_motd.text))
 {
  assert(MODE_IN_MODE(p, EDIT));
  
  edit_limit_characters(p, 5000);
  edit_limit_lines(p, 18);
  
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, motd_edit_quit);
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, motd_edit_end);
 }
 else
   buffer_file_destroy(p);
}

void cmds_init_motd(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("edmotd", user_motd_edit, NO_CHARS, ADMIN);
 CMDS_PRIV(lower_admin);
 CMDS_ADD("motd", user_motd, NO_CHARS, INFORMATION);
}

