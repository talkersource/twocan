#define TOGGLE_C
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



static void user_toggle_see_echo(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_see_echo, TRUE,
                       " You will %ssee who echos what.\n",
                       " You will %snot see who echos what.\n", TRUE);
}

static void user_toggle_24_clock(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_use_24_clock, TRUE,
                       " You will %ssee time in 24 hour format.\n",
                       " You will %ssee time in 12 hour format.\n", TRUE);
}

static void user_toggle_long_clock(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_use_long_clock, TRUE,
                       " You will %ssee time in long format.\n",
                       " You will %ssee time in dynamic format.\n", TRUE);
}

static void user_toggle_hidden_logon(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->saved->flag_hide_logon_time, TRUE,
                       " Your total logon time is %shidden.\n",
                       " Your total logon time is %svisible by everyone.\n",
                       TRUE);
}

static void user_toggle_birthday_age(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_use_birthday_as_age, TRUE,
                       " Your age is %sworked out from your birthday.\n",
                       " Your age is not %sworked out from your birthday.\n",
                       TRUE);
}

/* players over 5 mins idle */
static void user_toggle_block_idles(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_idle_warns_block, TRUE, 
                       " You will %snot get idle warns.\n",
                       " You will %sget idle warns.\n", TRUE);
}

static void user_toggle_show_inorder(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_list_show_inorder, TRUE,
                       " You %shave lists alphabeticaly ordered.\n",
                       " You %shave lists reverse-cronologicaly ordered.\n",
                       TRUE);
}

static void user_toggle_raw_twinkle(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->see_raw_twinkles, TRUE,
                       " You will %sget twinkles in RAW fashion.\n",
                       " You will %snot get twinkles in RAW fashion.\n", TRUE);
}

static void user_toggle_gmt_offset_hide(player *p, const char *str)
{
 TOGGLE_COMMAND_ON_OFF(p, str, p->flag_gmt_offset_hide, TRUE,
                       " You %shave your jetlag hidden.\n",
                       " You %sdo not have your jetlag hidden.\n",
                       TRUE);
}

void cmds_init_toggle(void)
{
 CMDS_BEGIN_DECLS();

 /* name has to die */
 CMDS_ADD("seeecho", user_toggle_see_echo, CONST_CHARS, SETTINGS);

 CMDS_ADD("clock_24_format", user_toggle_24_clock, CONST_CHARS, SETTINGS);
 CMDS_ADD("clock_long_format", user_toggle_long_clock, CONST_CHARS, SETTINGS);

 CMDS_ADD("conceal_logon", user_toggle_hidden_logon, CONST_CHARS, SETTINGS);
 CMDS_PRIV(base);

 CMDS_ADD("age_auto", user_toggle_birthday_age, CONST_CHARS, PERSONAL_INFO);
 
 CMDS_ADD("blockidle_warns", user_toggle_block_idles, CONST_CHARS, SETTINGS);

 CMDS_ADD("inorder_lists", user_toggle_show_inorder, CONST_CHARS, SETTINGS);

 CMDS_ADD("raw_twinkles", user_toggle_raw_twinkle, CONST_CHARS, SETTINGS);
 CMDS_PRIV(spod_pretend_su);

 CMDS_ADD("kill_gmt_offset", user_toggle_gmt_offset_hide,
          CONST_CHARS, SETTINGS);
}
