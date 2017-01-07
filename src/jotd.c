#define JOTD_C
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

/* Joke of the day - a zany idea for Crazy */



static void user_jotd(player *p)
{
 if (msg_jotd.text)
 {
  fvtell_player(NORMAL_T(p), "%s", msg_jotd.text);
  pager(p, PAGER_DEFAULT);
 }
 else
 {
  fvtell_player(NORMAL_T(p), "%s", " Joke of the day is currently disabled.");
  wlog("jotd", " Couldn't find jotd file.");
 }
}

static void jotd_edit_cleanup(player *p)
{
 buffer_file_destroy(p);
}

static void jotd_edit_quit(player *p)
{
 fvtell_player(NORMAL_T(p), "%s",
               " Leaving jotd edit without saving changes.\n");

 jotd_edit_cleanup(p);
}

static void jotd_edit_end(player *p)
{
 if (!msg_edit_sync_file(&msg_jotd, EDIT_BASE(p)))
   fvtell_player(NORMAL_T(p), "%s",
                 " Could not write the joke file - error occured.\n");
 else
   fvtell_player(NORMAL_T(p), "%s", " Joke of the day changed.\n");

 jotd_edit_cleanup(p);
}

static void user_jotd_edit(player *p)
{
 int created = 0;
 
 if ((created = buffer_file_create(p)) > 0)
   P_MEM_ERR(p);
 else if (created < 0)
   fvtell_player(NORMAL_T(p), "%s",
                 " Cannot edit the joke of the day whilst using the current "
                 " command, sorry.\n");
 else if (edit_start(p, msg_jotd.text))
 {
  assert(MODE_IN_MODE(p, EDIT));
  
  edit_limit_characters(p, 10000);
  edit_limit_lines(p, 8);
  
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_quit, jotd_edit_quit);
  CMDS_FUNC_TYPE_NO_CHARS(&EDIT_BASE(p)->cmd_end, jotd_edit_end);
 }
}

void cmds_init_jotd(void)
{
 CMDS_BEGIN_DECLS();

 CMDS_ADD("edjotd", user_jotd_edit, NO_CHARS, ADMIN);
 CMDS_PRIV(command_jotd_edit);
 CMDS_ADD("jotd", user_jotd, NO_CHARS, MISC);
}
