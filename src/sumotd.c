#define SUMOTD_C
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


void user_sumotd(player *p)
{
 if (PRIV_STAFF(p->saved))
 {
  fvtell_player(NORMAL_T(p), "%s", msg_sumotd.text);
  p->su_motdlr = now;
 }
}

static void sumotd_edit_cleanup(player *p)
{ 
 buffer_file_destroy(p);
}

static void sumotd_edit_quit(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", " Leaving without changes.\n");

 sumotd_edit_cleanup(p);
}

static void sumotd_edit_end(player *p)
{
 if (msg_edit_sync_file(&msg_sumotd, EDIT_BASE(p)))
   fvtell_player(NORMAL_T(p), "%s", " SUMOTD changed.\n");
 else
   fvtell_player(NORMAL_T(p), "%s",
                 " SUMOTD changed ^S^Bonly temporarily^s.\n");

 sumotd_edit_cleanup(p);
}

static void user_sumotd_edit(player *p)
{
 int created = 0;
 
 if ((created = buffer_file_create(p)) > 0)
   P_MEM_ERR(p);
 else if (created < 0)
   fvtell_player(NORMAL_T(p), "%s",
                 " You cannot Edit the sumotd whilst already editing "
                 "a file.\n");
 else if (edit_start(p, msg_sumotd.text))
 {
  assert(MODE_IN_MODE(p, EDIT));
  
  edit_limit_characters(p, 5000);
  edit_limit_lines(p, 18);
  
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, sumotd_edit_quit);
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, sumotd_edit_end);
 }
 else
   buffer_file_destroy(p);
}

void cmds_init_sumotd(void)
{
 CMDS_BEGIN_DECLS();
  
 CMDS_ADD("edsumotd", user_sumotd_edit, NO_CHARS, ADMIN);
 CMDS_PRIV(lower_admin);
 CMDS_ADD("sumotd", user_sumotd, NO_CHARS, SU);
 CMDS_PRIV(pretend_su);
}
