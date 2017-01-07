#define CHLIM_C
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

static void chlim_rejoin_func(player *p)
{
 fvtell_player(NORMAL_T(p), "%s", 
               " Re-Entering change limit mode. Use ^Bhelp chlim^N for help\n"
               " To run ^Bnormal commands^N type /<command>\n"
               " Use '^Bend^N' to ^Bleave^N.\n");
}

static int chlim_command(player *p, const char *str, size_t length)
{
 ICTRACE("chlim_command");
 
 if (MODE_IN_MODE(p, CHLIM))
   MODE_HELPER_COMMAND();
 else
   if (!*str)
   {
    cmds_function tmp_cmd;
    cmds_function tmp_rejoin;
    
    CMDS_FUNC_TYPE_RET_CHARS_SIZE_T((&tmp_cmd), chlim_command);
    CMDS_FUNC_TYPE_NO_CHARS((&tmp_rejoin), chlim_rejoin_func);

    if (mode_add(p, "Change limit Mode-> ", MODE_ID_CHLIM, 0,
                 &tmp_cmd, &tmp_rejoin, NULL))
      fvtell_player(NORMAL_T(p), "%s", 
                    " Entering check mode. Use ^Bhelp chlim^N for help\n"
                    " To run ^Bnormal commands^N type /<command>\n"
                    " Use '^Bend^N' to ^Bleave^N.\n");
    else
      fvtell_player(SYSTEM_T(p), 
                    " You cannot enter chlim mode as you are in too many "
                    "other modes.\n");
    return (TRUE);
   }
 
 return (cmds_sub_match(p, str, length,
                        &cmds_sub[CMDS_SUB_OFFSET(CMDS_SECTION_CHLIM)]));
}

static void user_chlim_view_commands(player *p)
{
 user_cmds_show_section(p, "chlim");
}

static void chlim_exit_command(player *p)
{
 if (!MODE_IN_MODE(p, CHLIM))
 {
  fvtell_player(NORMAL_T(p), "%s",
                " You just tried to exit change limit mode"
                " while not being in it?\n");
  return;
 }

 fvtell_player(NORMAL_T(p), "%s", " Leaving chlim mode.\n");

 mode_del(p);
}

static void user_su_chlim_alias(player *p, parameter_holder *params)
{
 CHLIM_BUILD("Aliases", max_aliases, TRUE);
}

static void user_su_chlim_autos(player *p, parameter_holder *params)
{
 CHLIM_BUILD("Autos", max_autos, TRUE);
}

static void user_su_chlim_exits(player *p, parameter_holder *params)
{
 CHLIM_BUILD("Exits", max_exits, TRUE);
}

static void user_su_chlim_list_entries(player *p,
                                              parameter_holder *params)
{
 CHLIM_BUILD("List entries", max_list_entries, TRUE);
}

static void user_su_chlim_mails(player *p, parameter_holder *params)
{
 CHLIM_BUILD("Mails", max_mails, TRUE);
}

static void user_su_chlim_nicknames(player *p, parameter_holder *params)
{
 CHLIM_BUILD("Nicknames", max_nicknames, TRUE);
}

static void user_su_chlim_rooms(player *p, parameter_holder *params)
{
 CHLIM_BUILD("Rooms", max_rooms, TRUE);
}

void cmds_init_chlim(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("chlim", chlim_command, RET_CHARS_SIZE_T, SETTINGS);
 CMDS_XTRA_SECTION(SYSTEM);
 CMDS_PRIV(lower_admin);

#define CMDS_SECTION_SUB CMDS_SECTION_CHLIM

 CMDS_ADD_SUB("aliases", user_su_chlim_alias, PARSE_PARAMS);
 CMDS_ADD_SUB("autos", user_su_chlim_autos, PARSE_PARAMS);
 CMDS_ADD_SUB("exits", user_su_chlim_exits, PARSE_PARAMS);
 CMDS_ADD_SUB("list_entries", user_su_chlim_list_entries,
              PARSE_PARAMS);
 CMDS_ADD_SUB("mails", user_su_chlim_mails, PARSE_PARAMS);
 CMDS_ADD_SUB("nicknames", user_su_chlim_nicknames, PARSE_PARAMS);
 CMDS_ADD_SUB("rooms", user_su_chlim_rooms, PARSE_PARAMS);

 CMDS_ADD_SUB("commands", user_chlim_view_commands, NO_CHARS);
 CMDS_ADD_SUB("end", chlim_exit_command, NO_CHARS);
 CMDS_PRIV(mode_chlim);
}
